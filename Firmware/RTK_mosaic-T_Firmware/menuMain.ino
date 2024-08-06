// Check to see if we've received serial over USB
void updateSerial()
{
    if (systemAvailable())
    {
        systemRead();

        menuMain(); // Present user menu
    }
}

// Display the options
// If user doesn't respond within a few seconds, return to main loop
void menuMain()
{
    inMainMenu = true;
    displaySerialConfig(); // Display 'Serial Config' while user is configuring

    while (1)
    {
        systemPrintln();
        char versionString[21];
        getFirmwareVersion(versionString, sizeof(versionString), true);
        systemPrintf("SparkFun RTK %s %s\r\n", platformPrefix, versionString);

        systemPrint("ESP32 WiFi MAC Address: ");
        systemPrintf("%02X:%02X:%02X:%02X:%02X:%02X\r\n", wifiMACAddress[0], wifiMACAddress[1], wifiMACAddress[2],
                     wifiMACAddress[3], wifiMACAddress[4], wifiMACAddress[5]);

        // Display the uptime
        uint64_t uptimeMilliseconds = millis();
        uint32_t uptimeDays = 0;
        byte uptimeHours = 0;
        byte uptimeMinutes = 0;
        byte uptimeSeconds = 0;

        uptimeDays = uptimeMilliseconds / MILLISECONDS_IN_A_DAY;
        uptimeMilliseconds %= MILLISECONDS_IN_A_DAY;

        uptimeHours = uptimeMilliseconds / MILLISECONDS_IN_AN_HOUR;
        uptimeMilliseconds %= MILLISECONDS_IN_AN_HOUR;

        uptimeMinutes = uptimeMilliseconds / MILLISECONDS_IN_A_MINUTE;
        uptimeMilliseconds %= MILLISECONDS_IN_A_MINUTE;

        uptimeSeconds = uptimeMilliseconds / MILLISECONDS_IN_A_SECOND;
        uptimeMilliseconds %= MILLISECONDS_IN_A_SECOND;

        systemPrint("System Uptime: ");
        systemPrintf("%d %02d:%02d:%02d.%03lld\r\n", uptimeDays, uptimeHours, uptimeMinutes, uptimeSeconds,
                     uptimeMilliseconds);

        systemPrintf("Filtered by parser: %d NMEA / %d RTCM / %d UBX / %d SBF\r\n", failedParserMessages_NMEA,
                     failedParserMessages_RTCM, failedParserMessages_UBX, failedParserMessages_SBF);

        systemPrintln("Menu: Main");

        systemPrintln("d) Debug software");

        systemPrintln("o) Configure operation");

        systemPrintln("r) Reset all settings to default");

        systemPrintln("x) Exit");

        byte incoming = getCharacterNumber();

        if (incoming == 'd')
            menuDebugSoftware();
        else if (incoming == 'o')
            menuOperation();
        else if (incoming == 'r')
        {
            systemPrintln("\r\nResetting to factory defaults. Press 'y' to confirm:");
            byte bContinue = getCharacterNumber();
            if (bContinue == 'y')
            {
                factoryReset(false);
            }
            else
                systemPrintln("Reset aborted");
        }

        else if (incoming == 'x')
            break;
        else if (incoming == INPUT_RESPONSE_GETCHARACTERNUMBER_EMPTY)
            break;
        else if (incoming == INPUT_RESPONSE_GETCHARACTERNUMBER_TIMEOUT)
            break;
        else
            printUnknown(incoming);
    }

    recordSystemSettings(); // Once all menus have exited, record the new settings to LittleFS and config file

    clearBuffer();           // Empty buffer of any newline chars
    inMainMenu = false;
}

// Erase all settings. Upon restart, unit will use defaults
void factoryReset(bool alreadyHasSemaphore)
{
    displaySytemReset(); // Display friendly message on OLED

    tasksStopUART1();

    systemPrintln("Formatting internal file system...");
    LittleFS.format();

    systemPrintln("Settings erased successfully. Rebooting. Goodbye!");
    delay(2000);
    ESP.restart();
}

