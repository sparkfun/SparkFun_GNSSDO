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

            digitalWrite(pin_errorLED, HIGH); // Turn the error LED on
            digitalWrite(pin_lockLED, LOW); // Turn the lock LED off

            if (!tasksStartUART1()) // Start monitoring the UART from GNSS
                displayGNSSFail(1000);
            else
            {
                settings.lastState = STATE_GNSS_CONFIGURED;
                recordSystemSettings(); // Record next state for next POR (avoid configuring mosaic twice)

                changeState(STATE_GNSS_CONFIGURED);
            }
        }
        break;

        case (STATE_GNSS_CONFIGURED): {
        }
        break;

        case (STATE_GNSS_ERROR_BEFORE_FINETIME): {
        }
        break;

        case (STATE_GNSS_FINETIME): {
        }
        break;

        case (STATE_GNSS_ERROR_AFTER_FINETIME): {
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
        return "STATE_GNSS_NOT_CONFIGURED";
    case (STATE_GNSS_ERROR_BEFORE_FINETIME):
        return "STATE_GNSS_ERROR_BEFORE_FINETIME";
    case (STATE_GNSS_FINETIME):
        return "STATE_GNSS_NOT_CONFIGURED";
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

    if (settings.enablePrintStates)
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
