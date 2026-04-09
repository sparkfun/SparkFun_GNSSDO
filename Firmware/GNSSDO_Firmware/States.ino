static volatile uint32_t lastStateTime = 0;
static volatile double setpoint_s;
static volatile double slew_turnaround_s;
static volatile double rate_s;
static volatile double rate_held_s;
static volatile double error_s;
static volatile bool slewing; // false causes setpoint etc. to be initialized. true indicates slewing is in progress
static volatile double smoothedTcxoBiasChange_ms = -2.0e10; // Same as Do-Not-Use
static volatile double smoothedTemperatureChange = 0.0; // GNSSDO+ only
static volatile int repeat = 0; // Keep count of how many times FREQUENCY_LOCK has been repeated

// Given the current state, see if conditions have moved us to a new state
// A user pressing the setup button (change between rover/base) is handled by checkpin_setupButton()
void updateSystemState()
{
    if (millis() - lastSystemStateUpdate > 500 || forceSystemStateUpdate == true)
    {
        lastSystemStateUpdate = millis();
        forceSystemStateUpdate = false;

        // Check to see if any external sources need to change state
        if (newSystemStateRequested == true)
        {
            newSystemStateRequested = false;
            if (systemState != requestedSystemState)
            {
                changeState(requestedSystemState);
                lastStateTime = millis();
            }
        }

        if (settings.enablePrintStates && ((millis() - lastStateTime) > 15000))
        {
            changeState(systemState);
            lastStateTime = millis();
        }

        // Move between states as needed
        DMW_st(changeState, systemState);
        switch (systemState)
        {
        // Code is starting for the first time. The mosaic-T needs to be configured to output messages on COM1
        case (STATE_GNSS_NOT_CONFIGURED): {

            if (!initializeGNSS()) // Configure GNSS PPS, messages etc.
                displayGNSSFail(1000);
            else
            {
                settings.lastState = STATE_GNSS_CONFIGURED;
                recordSystemSettings(); // Record next state for next POR (avoid configuring mosaic twice)

                changeState(STATE_GNSS_CONFIGURED);
            }
        }
        break;

        // GNSS has begun and has been configured
        // Wait for gnssWNSet, gnssToWSet and gnssFineTime to go true, then go into STATE_GNSS_FINETIME
        // Monitor gnssError. If it becomes non-zero, go into STATE_GNSS_ERROR_BEFORE_FINETIME
        case (STATE_GNSS_CONFIGURED): {
            if (gnssPVTUpdated) // Wait for PVT to be updated. It contains Error.
            {
                gnssPVTUpdated = false;

                updateErrorLED();

                if (gnssError)
                    changeState(STATE_GNSS_ERROR_BEFORE_FINETIME);
            }
            else if (gnssTimeUpdated[1]) // Wait for the time to be updated
            {
                gnssTimeUpdated[1] = false;

                if (gnssWNSet && gnssToWSet && gnssFineTime)
                {
                    if (presentTcxoTemperature)
                        changeState(STATE_TCXO_WARMUP);
                    else
                        changeState(STATE_GNSS_FINETIME);
                }
            }
        }
        break;

        // Wait for error to clear before checking FineTime
        case (STATE_GNSS_ERROR_BEFORE_FINETIME): {
            if (gnssPVTUpdated) // Wait for PVT to be updated
            {
                gnssPVTUpdated = false;

                updateErrorLED();

                if (!gnssError)
                    changeState(STATE_GNSS_CONFIGURED);
            }
        }
        break;

        // Wait for TCXO temperature to stabilise - if sensor is present
        case (STATE_TCXO_WARMUP): {
            if (gnssPVTUpdated) // Wait for PVT to be updated. It contains Error.
            {
                gnssPVTUpdated = false;

                updateTCXOClockBias(); // Update tcxoClockDrift_ppm and tcxoClockBias_ms from the best source

                updateErrorLED();

                if (gnssError)
                    changeState(STATE_GNSS_ERROR_DURING_WARMUP);

                // The STP3593LF waits for a minute or two before enabling its internal temperature loop
                // We need to stay in this state for at least tcxoMinWarmup_s
                static unsigned long seconds = 0;
                
                static double previousTemperature = tcxoTemperature;

                double temperatureChange = previousTemperature - tcxoTemperature;
                previousTemperature = tcxoTemperature;

                // updateTcxoTemperature() is called by the loop

                const double alpha = 0.90; // Exponential smoothing
                smoothedTemperatureChange = (alpha * smoothedTemperatureChange) + ((1.0 - alpha) * temperatureChange);
                
                // Wait for temperature to be stable to better than tcxoTemperatureStability ADU
                // and tcxoMinWarmup_s to have passed
                if ((fabs(smoothedTemperatureChange) < settings.tcxoTemperatureStability)
                    && (seconds > settings.tcxoMinWarmup_s))
                {
                    changeState(STATE_GNSS_FINETIME);
                }

                seconds++;
            }
        }
        break;

        // Wait for error to clear before resuming temperature monitoring
        case (STATE_GNSS_ERROR_DURING_WARMUP): {
            if (gnssPVTUpdated) // Wait for PVT to be updated
            {
                gnssPVTUpdated = false;

                updateErrorLED();

                if (!gnssError)
                    changeState(STATE_TCXO_WARMUP);
            }
        }
        break;

        // Discipline the TCXO in frequency locked mode
        // Monitor gnssError. If it becomes non-zero, go into STATE_GNSS_ERROR_AFTER_FINETIME
        // When smoothedTcxoBiasChange_ms is less than rxStabilityForFrequencyLockError, go into STATE_GNSS_FREQUENCY_LOCK
        case (STATE_GNSS_FINETIME): {
            if (gnssPVTUpdated) // Wait for PVT to be updated. It contains Error.
            {
                gnssPVTUpdated = false;

                updateTCXOClockBias(); // Update tcxoClockDrift_ppm and tcxoClockBias_ms from the best source

                updateLockLED(); // Update Lock LED based on systemState
                updateErrorLED();

                // Change state on error - stop updating the TCXO
                if (gnssError)
                {
                    changeState(STATE_GNSS_ERROR_AFTER_FINETIME);
                    break;
                }

                static double previousTcxoClockBias_ms = -2.0e10; // Same as Do-Not-Use
                if (previousTcxoClockBias_ms < -1.0e10)
                {
                    previousTcxoClockBias_ms = tcxoClockBias_ms;
                    break;
                }

                tcxoClockBiasChange_ms = previousTcxoClockBias_ms - tcxoClockBias_ms;
                previousTcxoClockBias_ms = tcxoClockBias_ms;

                if (smoothedTcxoBiasChange_ms < -1.0e10)
                {
                    smoothedTcxoBiasChange_ms = tcxoClockBiasChange_ms;
                    break;
                }

                const double alpha = 0.95; // Exponential smoothing
                smoothedTcxoBiasChange_ms = (alpha * smoothedTcxoBiasChange_ms) + ((1.0 - alpha) * tcxoClockBiasChange_ms);
                
                // Update the TCXO based on the change in the clock bias
                // updateTCXO calls myTCXO->setFrequencyByBiasMillis
                // myTCXO->setFrequencyByBiasMillis will limit the frequency change to _maxFrequencyChangePPB
                // So we can feed in a huge bias. All that is important is the sign...
                // If the previous bias is more positive than the current bias, the change is positive
                // But feeding a positive value into setFrequencyByBiasMillis reduces the frequency
                // So we need to invert the sign
                updateTCXO(0.0 - tcxoClockBiasChange_ms, settings.PkSteer, settings.IkSteer);

                // If smoothedTcxoBiasChange_ms is better than settings.rxStabilityForFrequencyLockError
                // go into STATE_GNSS_FREQUENCY_LOCK / STATE_GNSS_PHASE_LOCK
                if ((fabs(smoothedTcxoBiasChange_ms) < (1000.0 * settings.rxStabilityForFrequencyLockError)))
                {
                    // Reinitialize myTCXO to reset the integrator
                    // delete(myTCXO);
                    // myTCXO = nullptr;
                    // beginTCXO(i2cTCXO, settings.printDebugMessages);

                    // If the bias is low enough, go straight into phase lock
                    if (fabs(tcxoClockBias_ms) < (1000.0 * settings.rxPhaseErrorLimit_s))
                    {
                        // Party time! Excellent!
                        changeState(STATE_GNSS_PHASE_LOCK);
                    }
                    else
                    {
                        // Get everything ready, then change state
                        slewing = false;

                        changeState(STATE_GNSS_FREQUENCY_LOCK);
                    }
                }
            }
        }
        break;

        // Wait for error to clear before resuming TCXO discipline
        case (STATE_GNSS_ERROR_AFTER_FINETIME): {
            if (gnssPVTUpdated) // Wait for PVT to be updated
            {
                gnssPVTUpdated = false;

                updateErrorLED();

                if (!gnssError)
                    changeState(STATE_GNSS_FINETIME);
            }
        }
        break;

        // TCXO frequency is locked. Minimise the bias by pulling the frequency in increasing
        // then decreasing steps
        case (STATE_GNSS_FREQUENCY_LOCK): {
            if (gnssPVTUpdated) // Wait for PVT to be updated. It contains Error.
            {
                gnssPVTUpdated = false;

                updateTCXOClockBias(); // Update the tcxoClockBias_ms from the best source

                updateLockLED(); // Update Lock LED based on systemState
                updateErrorLED();

                // Change state on error - stop updating the TCXO
                if (gnssError)
                {
                    changeState(STATE_GNSS_ERROR_AFTER_FREQUENCY_LOCK);
                    break;
                }

                // =============================================================
                // Based on original Python code by Ole Rønningen. Thank you Ole

                // It seems that the STP3593 can decrease its frequency faster than it can increase it
                // This makes sense if the double-oven can cool at a faster rate than it can heat
                // So, we need to match both the heating and cooling rates to what the STP3593 is capable of

                // With tcxoRampAsymmetry set to 0.5:
                // If the initial bias is positive:
                //   Ramp up at 1ns/s to a maximum of 250ns/s
                //   Ramp down at 0.5ns/s
                //     To do this, we need to start ramping down earlier
                //     Ramp down will take twice as long as ramp up
                // If the initial bias is negative:
                //   Everything is reversed
                //   Ramp up at 0.5ns/s
                //   Ramp down at 1ns/s
                //   Ramp up will take twice as long as ramp down

                double tcxoClockBias_s = tcxoClockBias_ms * 0.001; // Convert bias to seconds

                static bool initalBiasPositive;

                // Record the sign of the initial bias
                // Initialise the setpoint with the current bias so we start with an error of ~zero
                // Set the turn around at the correct point for the tcxoRampAsymmetry
                // Initialize the rate
                if (!slewing)
                {
                    initalBiasPositive = (tcxoClockBias_s > 0.0);
                    setpoint_s = tcxoClockBias_s;
                    double turnaroundPoint = 1.0 / (1.0 + (1.0 / settings.tcxoRampAsymmetry));
                    if (initalBiasPositive)
                        slew_turnaround_s = fabs(setpoint_s) * (1.0 - turnaroundPoint); // slew_turnaround is absolute
                    else
                        slew_turnaround_s = fabs(setpoint_s) * turnaroundPoint; // slew_turnaround is absolute
                    rate_s = 3.0 * settings.tcxoRampStepSize_s; // initialize the rate
                    rate_held_s = rate_s;
                    error_s = 0.0;
                    slewing = true;
                    repeat = repeat + 1;
                }

                // If we have not yet reached the turnaround, increase the rate by the step size
                if (fabs(tcxoClockBias_s) > slew_turnaround_s) // Are we ramping 'up' ?
                {
                    if (initalBiasPositive)
                        rate_s = rate_s + settings.tcxoRampStepSize_s; // rate_s is absolute
                    else
                        rate_s = rate_s + (settings.tcxoRampStepSize_s * settings.tcxoRampAsymmetry);
                }
                // If have passed the turnaround, decrease the rate by the step size
                else
                {
                    if (initalBiasPositive)
                        rate_s = rate_s - (settings.tcxoRampStepSize_s * settings.tcxoRampAsymmetry);
                    else
                        rate_s = rate_s - settings.tcxoRampStepSize_s; // rate_s is absolute
                }

                // Check if the rate has returned to ~zero
                // We start at 3 * the step size. Check at 2 *
                if (rate_s <= (2.0 * settings.tcxoRampStepSize_s))
                    slewing = false;

                // Limit the rate (s/s)
                // With the control word set its limits, the STP3593 can ramp at about 330ns/s
                // To keep things stable and include room for TCXO aging, use a limit of 250ns/s
                if (rate_s > settings.tcxoRampRateLimit_sps)
                    rate_held_s = settings.tcxoRampRateLimit_sps;
                else
                    rate_held_s = rate_s;

                // Walk setpoint back to 0
                if (slewing)
                {
                    if (setpoint_s > rate_held_s)
                        setpoint_s = setpoint_s - rate_held_s; // Shift the setpoint by the rate. rate_s is absolute
                    else if (setpoint_s < (0.0 - rate_held_s))
                        setpoint_s = setpoint_s + rate_held_s;
                    else
                    {
                        setpoint_s = 0.0;
                        slewing = false;
                    }
                }

                // When slewing is false, we have finished slewing
                // Keep going until minimumRampRepeats is reached
                // Keep going if the phase error is still too large
                if (!slewing)
                {
                    if (repeat < settings.minimumRampRepeats)
                    {
                        // Reinitialize myTCXO to reset the integrator
                        // delete(myTCXO);
                        // myTCXO = nullptr;
                        // beginTCXO(i2cTCXO, settings.printDebugMessages);

                        //changeState(STATE_GNSS_FINETIME); // Fall back to FINETIME

                        // Go around again...
                        // Set settings.enablePrintDuplicateStates to true to see the non state change
                        changeState(STATE_GNSS_FREQUENCY_LOCK);
                    }
                    else if (fabs(tcxoClockBias_s) < settings.rxPhaseErrorLimit_s)
                    {
                        // Reinitialize myTCXO to reset the integrator
                        // delete(myTCXO);
                        // myTCXO = nullptr;
                        // beginTCXO(i2cTCXO, settings.printDebugMessages);

                        // Party time! Excellent!
                        changeState(STATE_GNSS_PHASE_LOCK);
                    }
                    else
                    {
                        // minimumRampRepeats has been reached but phase error is still too large

                        // Reinitialize myTCXO to reset the integrator
                        // delete(myTCXO);
                        // myTCXO = nullptr;
                        // beginTCXO(i2cTCXO, settings.printDebugMessages);

                        //changeState(STATE_GNSS_FINETIME); // Fall back to FINETIME

                        // Go around again...
                        // Set settings.enablePrintDuplicateStates to true to see the non state change
                        changeState(STATE_GNSS_FREQUENCY_LOCK);
                    }
                }
                else
                {
                    // The error term is the difference between the setpoint and the current bias
                    error_s = setpoint_s - tcxoClockBias_s;

                    // myTCXO->setFrequencyByBiasMillis needs the error in millis
                    // If the bias is positive, setpoint will be smaller than the bias by
                    // the cumulative rate (until the bias overtakes after turnaround)
                    // error_s will be negative. We need to invert
                    // setFrequencyByBiasMillis needs a positive value to reduce the frequency
                    // Use the aggressive P and I terms on the first repeat only
                    updateTCXO(1000.0 * (0.0 - error_s), // settings.PkSteer, settings.IkSteer);
                               repeat <= 1 ? settings.PkRamp : settings.PkSteer,
                               repeat <= 1 ? settings.IkRamp : settings.IkSteer);
                }

                // =============================================================
            }
        }
        break;

        // Wait for error to clear before resuming TCXO discipline
        case (STATE_GNSS_ERROR_AFTER_FREQUENCY_LOCK): {
            if (gnssPVTUpdated) // Wait for PVT to be updated
            {
                gnssPVTUpdated = false;

                updateErrorLED();

                if (!gnssError)
                {
                    updateTCXOClockBias(); // Update the tcxoClockBias_ms from the best source

                    // Get everything ready, then change state
                    slewing = false;
                    changeState(STATE_GNSS_FREQUENCY_LOCK); // Return to previous state

                    //changeState(STATE_GNSS_FINETIME); // Return to finetime (frequency locking)
                }
            }
        }
        break;

        // TCXO is locked. Run the PLL as normal
        case (STATE_GNSS_PHASE_LOCK): {
            if (gnssPVTUpdated) // Wait for PVT to be updated. It contains Error.
            {
                gnssPVTUpdated = false;

                updateTCXOClockBias(); // Update the tcxoClockBias_ms from the best source

                updateLockLED(); // Update Lock LED based on systemState
                updateErrorLED();

                // Keep count of TCXO control word updates
                static int tcxoUpdates = 0;
                static int saveControlWordAfter_s = 3600;

                // Change state on error - stop updating the TCXO
                if (gnssError)
                {
                    tcxoUpdates = 0; // Reset the count before changing state
                    saveControlWordAfter_s = 3600;
                    changeState(STATE_GNSS_ERROR_AFTER_PHASE_LOCK);
                    break;
                }

                // Start the PPS
                if (!ppsStarted && !gnssError && configureGNSSPPS())
                {
                    ppsStarted = true;
                }

                // Update the TCXO
                updateTCXO();

                // Check if it has all gone sideways...
                double tcxoClockBias_s = tcxoClockBias_ms * 0.001; // Convert bias to seconds
                if (fabs(tcxoClockBias_s) > (2.0 * settings.rxPhaseErrorLimit_s))
                {
                    tcxoUpdates = 0; // Reset the count before changing state
                    saveControlWordAfter_s = 3600;

                    slewing = false;
                    changeState(STATE_GNSS_FREQUENCY_LOCK); // Return to frequency locked

                    //changeState(STATE_GNSS_FINETIME); // Return to finetime (frequency locking)

                    break;
                }

                // Save the TCXO control word after 1, 2, 4, 8, 8, 8... hours - to protect the LittleFS flash memory
                tcxoUpdates++;
                if (tcxoUpdates > saveControlWordAfter_s)
                {
                    tcxoUpdates = 0;
                    settings.tcxoControl = getFrequencyControlWord();

                    settings.previousIP = gnssIP;

                    recordSystemSettings();

                    String debugString = String("TCXO Control Word saved to LFS: ")
                                        + String(settings.tcxoControl) + String("\r\n");
                    printDebug(debugString);

                    saveTCXO(); // Tell the TCXO to save its frequency control word - if supported

                    // Save the control word after: 1, 2, 4, 8, 8, 8... hours
                    if (saveControlWordAfter_s < 28800)
                        saveControlWordAfter_s *= 2;
                }
            }
        }
        break;

        // Wait for error to clear before resuming TCXO discipline
        case (STATE_GNSS_ERROR_AFTER_PHASE_LOCK): {
            if (gnssPVTUpdated) // Wait for PVT to be updated
            {
                gnssPVTUpdated = false;

                updateErrorLED();

                if (!gnssError)
                {
                    updateTCXOClockBias(); // Update the tcxoClockBias_ms from the best source

                    //changeState(STATE_GNSS_PHASE_LOCK); // Return to phase locked

                    slewing = false;
                    changeState(STATE_GNSS_FREQUENCY_LOCK); // Return to frequency locked

                    //changeState(STATE_GNSS_FINETIME); // Return to finetime (frequency locking)
                }
            }
        }
        break;

        default: {
            systemPrintf("Unknown state: %d\r\n", systemState);
        }
        break;
        }
    }
}

// System state changes may only occur within main state machine
// To allow state changes from external sources (ie, Button Tasks) requests can be made
// Requests are handled at the start of updateSystemState()
void requestChangeState(SystemState requestedState)
{
    newSystemStateRequested = true;
    requestedSystemState = requestedState;
    systemPrintf("Requested System State: %d\r\n", requestedSystemState);
}

// Print the current state
const char *getState(SystemState state, char *buffer)
{
    for (int i = 0; i < numSystemStatesNames; i++)
    {
        if (SystemStatesNames[i].systemState == state)
            return SystemStatesNames[i].stateName;
    }

    // Handle the unknown case
    sprintf(buffer, "Unknown: %d", state);
    return (const char *)buffer;
}

// Change states and print the new state
void changeState(SystemState newState)
{
    char string1[30];
    char string2[30];
    const char *arrow;
    const char *asterisk;
    const char *initialState;
    const char *endingState;

    // Log the heap size at the state change
    reportHeapNow(false);

    // Debug print of new state, add leading asterisk for repeated states
    if ((!settings.enablePrintDuplicateStates) && (newState == systemState))
        return;

    arrow = "";
    asterisk = "";
    initialState = "";
    if (newState == systemState)
        asterisk = "*";
    else
    {
        initialState = getState(systemState, string1);
        arrow = " --> ";
    }

    // Set the new state
    systemState = newState;

    if (settings.enablePrintStates && (!inMainMenu))
    {
        endingState = getState(newState, string2);

        if (!online.rtc)
            systemPrintf("%s%s%s%s\r\n", asterisk, initialState, arrow, endingState);
        else
        {
            // Timestamp the state change
            //          1         2
            // 12345678901234567890123456
            // YYYY-mm-dd HH:MM:SS.xxxrn0
            struct tm timeinfo = rtc.getTimeStruct();
            char s[30];
            strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &timeinfo);
            systemPrintf("%s%s%s%s, %s.%03ld\r\n", asterisk, initialState, arrow, endingState, s, rtc.getMillis());
        }
    }
}
