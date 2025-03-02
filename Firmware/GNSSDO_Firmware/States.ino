static uint32_t lastStateTime = 0;

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
                    changeState(STATE_GNSS_FINETIME);
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

        // Discipline the TCXO
        // Monitor gnssError. If it becomes non-zero, go into STATE_GNSS_ERROR_AFTER_FINETIME
        case (STATE_GNSS_FINETIME): {
            if (gnssPVTUpdated) // Wait for PVT to be updated. It contains Error.
            {
                gnssPVTUpdated = false;

                updateTCXOClockBias(); // Update the tcxoClockBias_ms from the best source

                updateLockLED(); // Update Lock LED based on tcxoClockBias_ms
                updateErrorLED();

                static uint8_t clkBiasLimitCount = 0;

                // If the clock bias is < the lock limit, start PPS
                if (fabs(tcxoClockBias_ms) < settings.rxClkBiasLockLimit_ms)
                {
                    if (clkBiasLimitCount < settings.rxClkBiasLimitCount)
                        clkBiasLimitCount++;
                    if (!ppsStarted && !gnssError && (clkBiasLimitCount >= settings.rxClkBiasLimitCount) && configureGNSSPPS())
                    {
                        ppsStarted = true;
                    }
                }
                else
                {
                    clkBiasLimitCount = 0;
                }

                static uint8_t badBiasCount = 0;

                // If the clock bias is > rxClkBiasInitialLimit_ms, reinitialize
                if (fabs(tcxoClockBias_ms) > settings.rxClkBiasInitialLimit_ms)
                {
                    if (badBiasCount < settings.rxClkBiasLimitCount)
                        badBiasCount++;
                    if (badBiasCount >= settings.rxClkBiasLimitCount)
                    {
                        clkBiasLimitCount = 0; // Reset the counts before changing state
                        badBiasCount = 0;
                        displayBadBias(2000);
                        gnssSoftReset(); // Kickstart the timing system
                        changeState(STATE_GNSS_CONFIGURED);
                    }
                }
                else
                {
                    badBiasCount = 0;
                }

                // Update the TCXO if the bias is OK. The message rate limits this to 1Hz
                if (badBiasCount == 0)
                    updateTCXO();

                static int tcxoUpdates = 0; // Keep count of TCXO control word updates. Save the control word every hour

                if (ppsStarted && (badBiasCount == 0))
                {
                    tcxoUpdates++;

                    // Save the TCXO control word once per hour only - to protect the LittleFS flash memory
                    if (tcxoUpdates > 3600)
                    {
                        tcxoUpdates = 0;
                        settings.tcxoControl = getFrequencyControlWord();

                        settings.previousIP = gnssIP;

                        recordSystemSettings();

                        systemPrint("TCXO Control Word saved to LFS: ");
                        systemPrintln(settings.tcxoControl);

                        saveTCXO(); // Tell the TCXO to save its frequency control word - if supported
                    }
                }
                else
                {
                    tcxoUpdates = 0;
                }

                // Change state on error - stop updating the TCXO
                if (gnssError)
                {
                    clkBiasLimitCount = 0; // Reset the counts before changing state
                    badBiasCount = 0;
                    tcxoUpdates = 0;
                    changeState(STATE_GNSS_ERROR_AFTER_FINETIME);
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
    switch (state)
    {
    case (STATE_GNSS_NOT_CONFIGURED):
        return "STATE_GNSS_NOT_CONFIGURED";
    case (STATE_GNSS_CONFIGURED):
        return "STATE_GNSS_CONFIGURED";
    case (STATE_GNSS_ERROR_BEFORE_FINETIME):
        return "STATE_GNSS_ERROR_BEFORE_FINETIME";
    case (STATE_GNSS_FINETIME):
        return "STATE_GNSS_FINETIME";
    case (STATE_GNSS_ERROR_AFTER_FINETIME):
        return "STATE_GNSS_ERROR_AFTER_FINETIME";
    case (STATE_NOT_SET):
        return "STATE_NOT_SET";
    }

    // Handle the unknown case
    sprintf(buffer, "Unknown: %d", state);
    return buffer;
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
