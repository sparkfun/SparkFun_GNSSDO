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
        else if (incoming == 30)
            settings.enablePrintStates ^= 1;
        else if (incoming == 31)
            settings.enablePrintDuplicateStates ^= 1;
        else if (incoming == 34)
            printPartitionTable();
        else if (incoming == 50)
            settings.enableTaskReports ^= 1;
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

        systemPrint("2) GNSS Serial Timeout: ");
        systemPrintln(settings.serialTimeoutGNSS);

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

        if (incoming == 2)
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

// Print the current long/lat/alt
void printCurrentConditions()
{
    if (online.gnss == true)
    {
        systemPrintf("%04d/%02d/%02d %02d:%02d:%02d",
            gnssYear, gnssMonth, gnssDay, gnssHour, gnssMinute, gnssSecond);
        
        systemPrint(", Lat: ");
        systemPrint(gnssLatitude_d, 7);

        systemPrint(", Lon: ");
        systemPrint(gnssLongitude_d, 7);

        systemPrint(", Alt: ");
        systemPrint(gnssAltitude_m, 4);

        systemPrint(", Sys: ");
        systemPrint(mosaicTimeSystemNameFromId(gnssTimeSys));

        systemPrint(", Error: ");
        systemPrint(mosaicPVTErrorNameFromId(gnssError));

        systemPrint(", Fine: ");
        systemPrint(gnssFineTime ? "True" : "False");

        if ((gnssClockBias_ms >= 1.0) || (gnssClockBias_ms <= -1.0))
            systemPrintf(", Bias: %.3fms",
                (float)gnssClockBias_ms);
        else if ((gnssClockBias_ms >= 0.001) || (gnssClockBias_ms <= -0.001))
            systemPrintf(", Bias: %.3f%cs",
                (float)(gnssClockBias_ms * 1000.0, char(181)));
        else
            systemPrintf(", Bias: %.3fns",
                (float)(gnssClockBias_ms * 1000000.0));

        systemPrintln();
    }
}

