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
        getFirmwareVersion(versionString, sizeof(versionString));
        systemPrintf("SparkPNT %s Firmware %s\r\n", platformPrefix, versionString);

        systemPrint("ESP32 WiFi MAC Address: ");
        systemPrintf("%02X:%02X:%02X:%02X:%02X:%02X\r\n", wifiMACAddress[0], wifiMACAddress[1], wifiMACAddress[2],
                     wifiMACAddress[3], wifiMACAddress[4], wifiMACAddress[5]);

        systemPrintf("Oscillator:             %s\r\n", oscillatorType);

        systemPrintf("GNSS:                   %s version %s S/N %s\r\n", ProductName, RxVersion, RxSerialNumber);

        systemPrint("Ethernet MAC Address:   ");
        systemPrintf("%02X:%02X:%02X:%02X:%02X:%02X\r\n", ethernetMACAddress[0], ethernetMACAddress[1], ethernetMACAddress[2],
                     ethernetMACAddress[3], ethernetMACAddress[4], ethernetMACAddress[5]);

        systemPrintf("Previous IP address:    %s\r\n", IPAddress(settings.previousIP).toString().c_str());

        systemPrintf("Current IP address:     %s\r\n", gnssIP.toString().c_str());

        if (settings.enableTCPServer)
            systemPrintf("TCP Server is enabled:  TCP Port %d\r\n", settings.tcpServerPort);

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

        systemPrint("System Uptime:          ");
        systemPrintf("%d %02d:%02d:%02d.%03lld\r\n", uptimeDays, uptimeHours, uptimeMinutes, uptimeSeconds,
                     uptimeMilliseconds);

        systemPrintf("Rejected by parser:     %d NMEA / %d RTCM / %d SBF\r\n", failedParserMessages_NMEA,
                     failedParserMessages_RTCM, failedParserMessages_SBF);

        systemPrintln();
        systemPrintln("Menu: Main");

        systemPrintln("c) Configure operation");

        systemPrintln("d) Debug software");

        systemPrintln("r) Reset all settings to default");

        systemPrintln("x) Exit");

        byte incoming = getCharacterNumber();

        if (incoming == 'c')
            menuOperation();
        else if (incoming == 'd')
            menuDebugSoftware();
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

    configureGNSSTCPServer(); // Configure TCP

    if (settings.enableTCPServer)
        systemPrintf("TCP Server is enabled. Please connect on %s:%d to view the console\r\n",
                    gnssIP.toString().c_str(), settings.tcpServerPort);
        
    beginConsole(115200, true); // Swap to Alt pins if TCP is enabled

    recordSystemSettings(); // Once all menus have exited, record the new settings to LittleFS and config file

    clearBuffer();           // Empty buffer of any newline chars
    inMainMenu = false;
}

// Erase all settings. Upon restart, unit will use defaults
void factoryReset(bool alreadyHasSemaphore)
{
    displaySytemReset(); // Display friendly message on OLED

    gnssHardReset(); // Probably redundant as initializeGNSS() will be called on restart

    tasksStopUART1();

    systemPrintln("Formatting internal file system...");
    LittleFS.format();

    systemPrintln("Settings erased successfully. Rebooting. Goodbye!");
    delay(2000);
    ESP.restart();
}

