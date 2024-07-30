// Toggle debug settings for software
void menuDebugSoftware()
{
    while (1)
    {
        systemPrintln();
        systemPrintln("Menu: Debug Software");

        // Heap
        systemPrint("1) Heap Reporting: ");
        systemPrintf("%s\r\n", settings.enableHeapReport ? "Enabled" : "Disabled");

        // Ring buffer - ZED Tx
        systemPrint("10) Print ring buffer offsets: ");
        systemPrintf("%s\r\n", settings.enablePrintRingBufferOffsets ? "Enabled" : "Disabled");

        systemPrint("11) Print ring buffer overruns: ");
        systemPrintf("%s\r\n", settings.enablePrintBufferOverrun ? "Enabled" : "Disabled");

        systemPrint("12) RTCM message checking: ");
        systemPrintf("%s\r\n", settings.enableRtcmMessageChecking ? "Enabled" : "Disabled");

        // RTK
        systemPrint("30) Print states: ");
        systemPrintf("%s\r\n", settings.enablePrintStates ? "Enabled" : "Disabled");

        systemPrint("31) Print duplicate states: ");
        systemPrintf("%s\r\n", settings.enablePrintDuplicateStates ? "Enabled" : "Disabled");

        systemPrintf("34) Print partition table\r\n");

        // Tasks
        systemPrint("50) Task Highwater Reporting: ");
        if (settings.enableTaskReports == true)
            systemPrintln("Enabled");
        else
            systemPrintln("Disabled");

        // Automatic Firmware Update
        systemPrintf("60) Print firmware update states: %s\r\n", settings.debugFirmwareUpdate ? "Enabled" : "Disabled");

        systemPrintln("e) Erase LittleFS");

        systemPrintln("r) Force system reset");

        systemPrintln("x) Exit");

        byte incoming = getCharacterNumber();

        if (incoming == 1)
            settings.enableHeapReport ^= 1;
        else if (incoming == 10)
            settings.enablePrintRingBufferOffsets ^= 1;
        else if (incoming == 11)
            settings.enablePrintBufferOverrun ^= 1;
        else if (incoming == 12)
            settings.enableRtcmMessageChecking ^= 1;
        else if (incoming == 30)
            settings.enablePrintStates ^= 1;
        else if (incoming == 31)
            settings.enablePrintDuplicateStates ^= 1;
        else if (incoming == 34)
            printPartitionTable();
        else if (incoming == 50)
            settings.enableTaskReports ^= 1;
        else if (incoming == 60)
            settings.debugFirmwareUpdate ^= 1;
        else if (incoming == 'e')
        {
            systemPrintln("Erasing LittleFS and resetting");
            LittleFS.format();
            ESP.restart();
        }

        // Menu exit control
        else if (incoming == 'r')
        {
            recordSystemSettings();

            ESP.restart();
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

    clearBuffer(); // Empty buffer of any newline chars
}

// Configure the RTK operation
void menuOperation()
{
    while (1)
    {
        systemPrintln();
        systemPrintln("Menu: Operation");

        systemPrint("3) GNSS Handler Buffer Size: ");
        systemPrintln(settings.gnssHandlerBufferSize);

        systemPrint("4) GNSS Serial RX Full Threshold: ");
        systemPrintln(settings.serialGNSSRxFullThreshold);

        // UART
        systemPrint("9) UART Receive Buffer Size: ");
        systemPrintln(settings.uartReceiveBufferSize);

        systemPrint("32) I2C Interrupts Core: ");
        systemPrintln(settings.i2cInterruptsCore);

        // Tasks
        systemPrintln("-------  Tasks  ------");

        systemPrint("52) GNSS Data Handler Core: ");
        systemPrintln(settings.handleGnssDataTaskCore);
        systemPrint("53) GNSS Data Handler Task Priority: ");
        systemPrintln(settings.handleGnssDataTaskPriority);

        systemPrint("54) GNSS Read Task Core: ");
        systemPrintln(settings.gnssReadTaskCore);
        systemPrint("55) GNSS Read Task Priority: ");
        systemPrintln(settings.gnssReadTaskPriority);

        systemPrintln("x) Exit");

        byte incoming = getCharacterNumber();

        if (incoming == 1)
        {
            settings.enableResetDisplay ^= 1;
            if (settings.enableResetDisplay == true)
            {
                settings.resetCount = 0;
                recordSystemSettings(); // Record to NVM
            }
        }
        else if (incoming == 2)
        {
            systemPrint("Enter GNSS Serial Timeout in milliseconds (0 to 1000): ");
            int serialTimeoutGNSS = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((serialTimeoutGNSS != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (serialTimeoutGNSS != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (serialTimeoutGNSS < 0 || serialTimeoutGNSS > 1000) // Arbitrary 1s limit
                    systemPrintln("Error: Timeout is out of range");
                else
                    settings.serialTimeoutGNSS = serialTimeoutGNSS; // Recorded to NVM and file at main menu exit
            }
        }
        else if (incoming == 3)
        {
            systemPrintln("Warning: changing the Handler Buffer Size will restart the RTK. Enter 0 to abort");
            systemPrint("Enter GNSS Handler Buffer Size in Bytes (32 to 65535): ");
            int queSize = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((queSize != INPUT_RESPONSE_GETNUMBER_EXIT) && (queSize != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (queSize < 32 || queSize > 65535) // Arbitrary 64k limit
                    systemPrintln("Error: Queue size out of range");
                else
                {
                    // Stop the UART2 tssks to prevent the system from crashing
                    tasksStopUART2();

                    // Update the buffer size
                    settings.gnssHandlerBufferSize = queSize; // Recorded to NVM and file
                    recordSystemSettings();

                    // Reboot the system
                    ESP.restart();
                }
            }
        }
        else if (incoming == 4)
        {
            systemPrint("Enter Serial GNSS RX Full Threshold (1 to 127): ");
            int serialGNSSRxFullThreshold = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((serialGNSSRxFullThreshold != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (serialGNSSRxFullThreshold != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (serialGNSSRxFullThreshold < 1 || serialGNSSRxFullThreshold > 127)
                    systemPrintln("Error: Core out of range");
                else
                {
                    settings.serialGNSSRxFullThreshold = serialGNSSRxFullThreshold; // Recorded to NVM and file
                }
            }
        }
        else if (incoming == 5)
        {
            systemPrint("Enter number of seconds in RTK float before hot-start (0-disable to 3600): ");
            int timeout = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((timeout != INPUT_RESPONSE_GETNUMBER_EXIT) && (timeout != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (timeout < 0 || timeout > 3600) // Arbitrary 60 minute limit
                    systemPrintln("Error: Timeout out of range");
                else
                    settings.lbandFixTimeout_seconds = timeout; // Recorded to NVM and file at main menu exit
            }
        }
        else if (incoming == 6)
        {
            systemPrint("Enter SPI frequency in MHz (1 to 16): ");
            int freq = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((freq != INPUT_RESPONSE_GETNUMBER_EXIT) && (freq != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (freq < 1 || freq > 16) // Arbitrary 16MHz limit
                    systemPrintln("Error: SPI frequency out of range");
                else
                    settings.spiFrequency = freq; // Recorded to NVM and file at main menu exit
            }
        }
        else if (incoming == 7)
        {
            systemPrint("Enter SPP RX Queue Size in Bytes (32 to 16384): ");
            int queSize = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((queSize != INPUT_RESPONSE_GETNUMBER_EXIT) && (queSize != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (queSize < 32 || queSize > 16384) // Arbitrary 16k limit
                    systemPrintln("Error: Queue size out of range");
                else
                    settings.sppRxQueueSize = queSize; // Recorded to NVM and file at main menu exit
            }
        }
        else if (incoming == 8)
        {
            systemPrint("Enter SPP TX Queue Size in Bytes (32 to 16384): ");
            int queSize = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((queSize != INPUT_RESPONSE_GETNUMBER_EXIT) && (queSize != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (queSize < 32 || queSize > 16384) // Arbitrary 16k limit
                    systemPrintln("Error: Queue size out of range");
                else
                    settings.sppTxQueueSize = queSize; // Recorded to NVM and file at main menu exit
            }
        }
        else if (incoming == 9)
        {
            systemPrintln("Warning: changing the Receive Buffer Size will restart the RTK. Enter 0 to abort");
            systemPrint("Enter UART Receive Buffer Size in Bytes (32 to 16384): ");
            int queSize = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((queSize != INPUT_RESPONSE_GETNUMBER_EXIT) && (queSize != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (queSize < 32 || queSize > 16384) // Arbitrary 16k limit
                    systemPrintln("Error: Queue size out of range");
                else
                {
                    settings.uartReceiveBufferSize = queSize; // Recorded to NVM and file
                    recordSystemSettings();
                    ESP.restart();
                }
            }
        }
        else if (incoming == 10)
        {
            bool response = setMessagesUSB(MAX_SET_MESSAGES_RETRIES);

            if (response == false)
                systemPrintln(F("Failed to enable USB messages"));
            else
                systemPrintln(F("USB messages successfully enabled"));
        }
        else if (incoming == 11)
        {
            settings.useI2cForLbandCorrectionsConfigured =
                true; // Record that the user has manually modified the settings.
            settings.useI2cForLbandCorrections ^= 1;
        }
        else if (incoming == 12)
        {
            systemPrint("Enter the number of seconds before L-Band is used once RTCM is absent (1 to 255): ");
            int rtcmTimeoutBeforeUsingLBand_s = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((rtcmTimeoutBeforeUsingLBand_s != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (rtcmTimeoutBeforeUsingLBand_s != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (rtcmTimeoutBeforeUsingLBand_s < 1 || rtcmTimeoutBeforeUsingLBand_s > 255)
                    systemPrintln("Error: RTCM timeout out of range");
                else
                    settings.rtcmTimeoutBeforeUsingLBand_s = rtcmTimeoutBeforeUsingLBand_s; // Recorded to NVM and file
            }
        }
        else if (incoming == 13)
        {
            settings.enableZedUsb ^= 1;

            bool response = true;

            response &= theGNSS.newCfgValset();

            if (settings.enableZedUsb == true)
            {
                // The USB port on the ZED may be used for RTCM to/from the computer (as an NTRIP caster or client)
                // So let's be sure all protocols are on for the USB port
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBOUTPROT_UBX, 1);
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBOUTPROT_NMEA, 1);
                if (commandSupported(UBLOX_CFG_USBOUTPROT_RTCM3X) == true)
                    response &= theGNSS.addCfgValset(UBLOX_CFG_USBOUTPROT_RTCM3X, 1);
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBINPROT_UBX, 1);
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBINPROT_NMEA, 1);
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBINPROT_RTCM3X, 1);
                if (commandSupported(UBLOX_CFG_USBINPROT_SPARTN) == true)
                {
                    // See issue: https://github.com/sparkfun/SparkFun_RTK_Firmware/issues/713
                    response &= theGNSS.addCfgValset(UBLOX_CFG_USBINPROT_SPARTN, 1);
                }
            }
            else
            {
                // Disable all protocols over USB
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBOUTPROT_UBX, 0);
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBOUTPROT_NMEA, 0);
                if (commandSupported(UBLOX_CFG_USBOUTPROT_RTCM3X) == true)
                    response &= theGNSS.addCfgValset(UBLOX_CFG_USBOUTPROT_RTCM3X, 0);
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBINPROT_UBX, 0);
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBINPROT_NMEA, 0);
                response &= theGNSS.addCfgValset(UBLOX_CFG_USBINPROT_RTCM3X, 0);
                if (commandSupported(UBLOX_CFG_USBINPROT_SPARTN) == true)
                {
                    // See issue: https://github.com/sparkfun/SparkFun_RTK_Firmware/issues/713
                    response &= theGNSS.addCfgValset(UBLOX_CFG_USBINPROT_SPARTN, 0);
                }
            }
            response &= theGNSS.sendCfgValset();

            if (response == false)
                systemPrintln("Failed to set UART2 settings");
        }

        else if (incoming == 30)
        {
            systemPrint("Not yet implemented! - Enter Core used for Bluetooth Interrupts (0 or 1): ");
            int bluetoothInterruptsCore = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((bluetoothInterruptsCore != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (bluetoothInterruptsCore != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (bluetoothInterruptsCore < 0 || bluetoothInterruptsCore > 1)
                    systemPrintln("Error: Core out of range");
                else
                {
                    settings.bluetoothInterruptsCore = bluetoothInterruptsCore; // Recorded to NVM and file
                }
            }
        }
        else if (incoming == 31)
        {
            systemPrint("Enter Core used for GNSS UART Interrupts (0 or 1): ");
            int gnssUartInterruptsCore = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((gnssUartInterruptsCore != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (gnssUartInterruptsCore != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (gnssUartInterruptsCore < 0 || gnssUartInterruptsCore > 1)
                    systemPrintln("Error: Core out of range");
                else
                {
                    settings.gnssUartInterruptsCore = gnssUartInterruptsCore; // Recorded to NVM and file
                }
            }
        }
        else if (incoming == 32)
        {
            systemPrint("Enter Core used for I2C Interrupts (0 or 1): ");
            int i2cInterruptsCore = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((i2cInterruptsCore != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (i2cInterruptsCore != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (i2cInterruptsCore < 0 || i2cInterruptsCore > 1)
                    systemPrintln("Error: Core out of range");
                else
                {
                    settings.i2cInterruptsCore = i2cInterruptsCore; // Recorded to NVM and file
                }
            }
        }

        else if (incoming == 50)
        {
            systemPrint("Enter BT Read Task Core (0 or 1): ");
            int btReadTaskCore = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((btReadTaskCore != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (btReadTaskCore != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (btReadTaskCore < 0 || btReadTaskCore > 1)
                    systemPrintln("Error: Core out of range");
                else
                {
                    settings.btReadTaskCore = btReadTaskCore; // Recorded to NVM and file
                }
            }
        }
        else if (incoming == 51)
        {
            systemPrint("Enter BT Read Task Priority (0 to 3): ");
            int btReadTaskPriority = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((btReadTaskPriority != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (btReadTaskPriority != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (btReadTaskPriority < 0 || btReadTaskPriority > 3)
                    systemPrintln("Error: Task priority out of range");
                else
                {
                    settings.btReadTaskPriority = btReadTaskPriority; // Recorded to NVM and file
                }
            }
        }
        else if (incoming == 52)
        {
            systemPrint("Enter GNSS Data Handler Task Core (0 or 1): ");
            int handleGnssDataTaskCore = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((handleGnssDataTaskCore != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (handleGnssDataTaskCore != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (handleGnssDataTaskCore < 0 || handleGnssDataTaskCore > 1)
                    systemPrintln("Error: Core out of range");
                else
                {
                    settings.handleGnssDataTaskCore = handleGnssDataTaskCore; // Recorded to NVM and file
                }
            }
        }
        else if (incoming == 53)
        {
            systemPrint("Enter GNSS Data Handle Task Priority (0 to 3): ");
            int handleGnssDataTaskPriority = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((handleGnssDataTaskPriority != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (handleGnssDataTaskPriority != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (handleGnssDataTaskPriority < 0 || handleGnssDataTaskPriority > 3)
                    systemPrintln("Error: Task priority out of range");
                else
                {
                    settings.handleGnssDataTaskPriority = handleGnssDataTaskPriority; // Recorded to NVM and file
                }
            }
        }
        else if (incoming == 54)
        {
            systemPrint("Enter GNSS Read Task Core (0 or 1): ");
            int gnssReadTaskCore = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((gnssReadTaskCore != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (gnssReadTaskCore != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (gnssReadTaskCore < 0 || gnssReadTaskCore > 1)
                    systemPrintln("Error: Core out of range");
                else
                {
                    settings.gnssReadTaskCore = gnssReadTaskCore; // Recorded to NVM and file
                }
            }
        }
        else if (incoming == 55)
        {
            systemPrint("Enter GNSS Read Task Priority (0 to 3): ");
            int gnssReadTaskPriority = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((gnssReadTaskPriority != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (gnssReadTaskPriority != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (gnssReadTaskPriority < 0 || gnssReadTaskPriority > 3)
                    systemPrintln("Error: Task priority out of range");
                else
                {
                    settings.gnssReadTaskPriority = gnssReadTaskPriority; // Recorded to NVM and file
                }
            }
        }

        // Menu exit control
        else if (incoming == 'x')
            break;
        else if (incoming == INPUT_RESPONSE_GETCHARACTERNUMBER_EMPTY)
            break;
        else if (incoming == INPUT_RESPONSE_GETCHARACTERNUMBER_TIMEOUT)
            break;
        else
            printUnknown(incoming);
    }

    clearBuffer(); // Empty buffer of any newline chars
}

// Toggle periodic print message enables
void menuPeriodicPrint()
{
    while (1)
    {
        systemPrintln();
        systemPrintln("Menu: Periodic Print Messages");

        systemPrintln("-----  Hardware  -----");
        systemPrint("1) Bluetooth RX: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_BLUETOOTH_DATA_RX) ? "Enabled" : "Disabled");

        systemPrint("2) Bluetooth TX: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_BLUETOOTH_DATA_TX) ? "Enabled" : "Disabled");

        systemPrint("3) Ethernet IP address: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_ETHERNET_IP_ADDRESS) ? "Enabled" : "Disabled");

        systemPrint("4) Ethernet state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_ETHERNET_STATE) ? "Enabled" : "Disabled");

        systemPrint("5) SD log write data: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_SD_LOG_WRITE) ? "Enabled" : "Disabled");

        systemPrint("6) WiFi IP Address: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_WIFI_IP_ADDRESS) ? "Enabled" : "Disabled");

        systemPrint("7) WiFi state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_WIFI_STATE) ? "Enabled" : "Disabled");

        systemPrint("8) ZED RX data: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_ZED_DATA_RX) ? "Enabled" : "Disabled");

        systemPrint("9) ZED TX data: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_ZED_DATA_TX) ? "Enabled" : "Disabled");

        systemPrintln("-----  Software  -----");

        systemPrintf("20) Periodic print: %d (0x%08x)\r\n", settings.periodicDisplay, settings.periodicDisplay);

        systemPrintf("21) Interval (seconds): %d\r\n", settings.periodicDisplayInterval / 1000);

        systemPrint("22) CPU idle time: ");
        systemPrintf("%s\r\n", settings.enablePrintIdleTime ? "Enabled" : "Disabled");

        systemPrint("23) Network state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_NETWORK_STATE) ? "Enabled" : "Disabled");

        systemPrint("24) Ring buffer consumer times: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_RING_BUFFER_MILLIS) ? "Enabled" : "Disabled");

        systemPrint("25) RTK position: ");
        systemPrintf("%s\r\n", settings.enablePrintPosition ? "Enabled" : "Disabled");

        systemPrint("26) RTK state: ");
        systemPrintf("%s\r\n", settings.enablePrintState ? "Enabled" : "Disabled");

        systemPrintln("------  Clients  -----");
        systemPrint("40) NTP server data: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_NTP_SERVER_DATA) ? "Enabled" : "Disabled");

        systemPrint("41) NTP server state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_NTP_SERVER_STATE) ? "Enabled" : "Disabled");

        systemPrint("42) NTRIP client data: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_NTRIP_CLIENT_DATA) ? "Enabled" : "Disabled");

        systemPrint("43) NTRIP client GGA writes: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_NTRIP_CLIENT_GGA) ? "Enabled" : "Disabled");

        systemPrint("44) NTRIP client state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_NTRIP_CLIENT_STATE) ? "Enabled" : "Disabled");

        systemPrint("45) NTRIP server data: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_NTRIP_SERVER_DATA) ? "Enabled" : "Disabled");

        systemPrint("46) NTRIP server state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_NTRIP_SERVER_STATE) ? "Enabled" : "Disabled");

        systemPrint("47) PVT client data: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_PVT_CLIENT_DATA) ? "Enabled" : "Disabled");

        systemPrint("48) PVT client state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_PVT_CLIENT_STATE) ? "Enabled" : "Disabled");

        systemPrint("49) PVT server client data: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_PVT_SERVER_CLIENT_DATA) ? "Enabled" : "Disabled");

        systemPrint("50) PVT server data: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_PVT_SERVER_DATA) ? "Enabled" : "Disabled");

        systemPrint("51) PVT server state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_PVT_SERVER_STATE) ? "Enabled" : "Disabled");

        systemPrint("52) OTA client state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_OTA_CLIENT_STATE) ? "Enabled" : "Disabled");

        systemPrintln("-------  Tasks  ------");
        systemPrint("70) btReadTask state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_TASK_BLUETOOTH_READ) ? "Enabled" : "Disabled");

        systemPrint("71) ButtonCheckTask state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_TASK_BUTTON_CHECK) ? "Enabled" : "Disabled");

        systemPrint("72) gnssReadTask state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_TASK_GNSS_READ) ? "Enabled" : "Disabled");

        systemPrint("73) handleGnssDataTask state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_TASK_HANDLE_GNSS_DATA) ? "Enabled" : "Disabled");

        systemPrint("74) sdSizeCheckTask state: ");
        systemPrintf("%s\r\n", PERIODIC_SETTING(PD_TASK_SD_SIZE_CHECK) ? "Enabled" : "Disabled");

        systemPrintln("x) Exit");

        byte incoming = getCharacterNumber();

        if (incoming == 1)
            PERIODIC_TOGGLE(PD_BLUETOOTH_DATA_RX);
        else if (incoming == 2)
            PERIODIC_TOGGLE(PD_BLUETOOTH_DATA_TX);
        else if (incoming == 3)
            PERIODIC_TOGGLE(PD_ETHERNET_IP_ADDRESS);
        else if (incoming == 4)
            PERIODIC_TOGGLE(PD_ETHERNET_STATE);
        else if (incoming == 5)
            PERIODIC_TOGGLE(PD_SD_LOG_WRITE);
        else if (incoming == 6)
            PERIODIC_TOGGLE(PD_WIFI_IP_ADDRESS);
        else if (incoming == 7)
            PERIODIC_TOGGLE(PD_WIFI_STATE);
        else if (incoming == 8)
            PERIODIC_TOGGLE(PD_ZED_DATA_RX);
        else if (incoming == 9)
            PERIODIC_TOGGLE(PD_ZED_DATA_TX);

        else if (incoming == 20)
        {
            int value = getNumber();
            if ((value != INPUT_RESPONSE_GETNUMBER_EXIT) && (value != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
                settings.periodicDisplay = value;
        }
        else if (incoming == 21)
        {
            int seconds = getNumber();
            if ((seconds != INPUT_RESPONSE_GETNUMBER_EXIT) && (seconds != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
                settings.periodicDisplayInterval = seconds * 1000;
        }
        else if (incoming == 22)
            settings.enablePrintIdleTime ^= 1;
        else if (incoming == 23)
            PERIODIC_TOGGLE(PD_NETWORK_STATE);
        else if (incoming == 24)
            PERIODIC_TOGGLE(PD_RING_BUFFER_MILLIS);
        else if (incoming == 25)
            settings.enablePrintPosition ^= 1;
        else if (incoming == 26)
            settings.enablePrintState ^= 1;

        else if (incoming == 40)
            PERIODIC_TOGGLE(PD_NTP_SERVER_DATA);
        else if (incoming == 41)
            PERIODIC_TOGGLE(PD_NTP_SERVER_STATE);
        else if (incoming == 42)
            PERIODIC_TOGGLE(PD_NTRIP_CLIENT_DATA);
        else if (incoming == 43)
            PERIODIC_TOGGLE(PD_NTRIP_CLIENT_GGA);
        else if (incoming == 44)
            PERIODIC_TOGGLE(PD_NTRIP_CLIENT_STATE);
        else if (incoming == 45)
            PERIODIC_TOGGLE(PD_NTRIP_SERVER_DATA);
        else if (incoming == 46)
            PERIODIC_TOGGLE(PD_NTRIP_SERVER_STATE);
        else if (incoming == 47)
            PERIODIC_TOGGLE(PD_PVT_CLIENT_DATA);
        else if (incoming == 48)
            PERIODIC_TOGGLE(PD_PVT_CLIENT_STATE);
        else if (incoming == 49)
            PERIODIC_TOGGLE(PD_PVT_SERVER_CLIENT_DATA);
        else if (incoming == 50)
            PERIODIC_TOGGLE(PD_PVT_SERVER_DATA);
        else if (incoming == 51)
            PERIODIC_TOGGLE(PD_PVT_SERVER_STATE);
        else if (incoming == 52)
            PERIODIC_TOGGLE(PD_OTA_CLIENT_STATE);

        else if (incoming == 70)
            PERIODIC_TOGGLE(PD_TASK_BLUETOOTH_READ);
        else if (incoming == 71)
            PERIODIC_TOGGLE(PD_TASK_BUTTON_CHECK);
        else if (incoming == 72)
            PERIODIC_TOGGLE(PD_TASK_GNSS_READ);
        else if (incoming == 73)
            PERIODIC_TOGGLE(PD_TASK_HANDLE_GNSS_DATA);
        else if (incoming == 74)
            PERIODIC_TOGGLE(PD_TASK_SD_SIZE_CHECK);

        // Menu exit control
        else if (incoming == 'x')
            break;
        else if (incoming == INPUT_RESPONSE_GETCHARACTERNUMBER_EMPTY)
            break;
        else if (incoming == INPUT_RESPONSE_GETCHARACTERNUMBER_TIMEOUT)
            break;
        else
            printUnknown(incoming);
    }

    clearBuffer(); // Empty buffer of any newline chars
}

// Print the current long/lat/alt
void printCurrentConditions()
{
    if (online.gnss == true)
    {
        systemPrint("Lat: ");
        systemPrint(latitude, 7);
        systemPrint(", Lon: ");
        systemPrint(longitude, 7);

        systemPrint(", Altitude (m): ");
        systemPrint(altitude, 7);

        systemPrintln();
    }
}

