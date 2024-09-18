// Toggle debug settings for software
void menuDebugSoftware()
{
    while (1)
    {
        systemPrintln();
        systemPrintln("Menu: Debug Software");

        systemPrint("1) Periodic print interval (ms): ");
        systemPrintln(settings.periodicPrintInterval_ms);

        systemPrint("2) Heap Reporting: ");
        systemPrintf("%s\r\n", settings.enableHeapReport ? "Enabled" : "Disabled");

        systemPrint("3) Task Reporting: ");
        if (settings.enableTaskReports == true)
            systemPrintln("Enabled");
        else
            systemPrintln("Disabled");

        systemPrint("4) Print messages with bad checksums or CRCs: ");
        systemPrintf("%s\r\n", settings.enablePrintBadMessages ? "Enabled" : "Disabled");

        systemPrint("5) Print states: ");
        systemPrintf("%s\r\n", settings.enablePrintStates ? "Enabled" : "Disabled");

        systemPrint("6) Print duplicate states: ");
        systemPrintf("%s\r\n", settings.enablePrintDuplicateStates ? "Enabled" : "Disabled");

        systemPrint("7) Print RTC resyncs: ");
        systemPrintf("%s\r\n", settings.enablePrintRtcSync ? "Enabled" : "Disabled");

        systemPrint("8) Print conditions: ");
        systemPrintf("%s\r\n", settings.enablePrintConditions ? "Enabled" : "Disabled");

        systemPrint("9) Print consumers: ");
        systemPrintf("%s\r\n", settings.enablePrintConsumers ? "Enabled" : "Disabled");

        systemPrint("10) Print idle time: ");
        systemPrintf("%s\r\n", settings.enablePrintIdleTime ? "Enabled" : "Disabled");

        systemPrint("11) Print GNSS messages: ");
        systemPrintf("%s\r\n", settings.enablePrintGNSSMessages ? "Enabled" : "Disabled");

        systemPrint("12) Print ring buffer offsets: ");
        systemPrintf("%s\r\n", settings.enablePrintRingBufferOffsets ? "Enabled" : "Disabled");

        systemPrint("13) Print ring buffer overruns: ");
        systemPrintf("%s\r\n", settings.enablePrintBufferOverrun ? "Enabled" : "Disabled");

        systemPrint("14) Echo user input: ");
        systemPrintf("%s\r\n", settings.echoUserInput ? "Enabled" : "Disabled");

        systemPrintf("15) Print partition table\r\n");

        // UART
        systemPrintln("\r\n-------  UART  ------\r\n");

        systemPrint("20) GNSS Serial Timeout: ");
        systemPrintln(settings.serialTimeoutGNSS);

        systemPrint("21) GNSS Handler Buffer Size: ");
        systemPrintln(settings.gnssHandlerBufferSize);

        systemPrint("22) GNSS Serial RX Full Threshold: ");
        systemPrintln(settings.serialGNSSRxFullThreshold);

        systemPrint("23) UART Receive Buffer Size: ");
        systemPrintln(settings.uartReceiveBufferSize);

        systemPrint("24) I2C Interrupts Core: ");
        systemPrintln(settings.i2cInterruptsCore);

        systemPrint("25) GNSS UART Interrupts Core: ");
        systemPrintln(settings.gnssUartInterruptsCore);

        // Tasks
        systemPrintln("\r\n-------  Tasks  ------\r\n");

        systemPrint("30) GNSS Data Handler Core: ");
        systemPrintln(settings.handleGnssDataTaskCore);
        systemPrint("31) GNSS Data Handler Task Priority: ");
        systemPrintln(settings.handleGnssDataTaskPriority);

        systemPrint("32) GNSS Read Task Core: ");
        systemPrintln(settings.gnssReadTaskCore);
        systemPrint("33) GNSS Read Task Priority: ");
        systemPrintln(settings.gnssReadTaskPriority);

        systemPrintln("\r\ne) Erase LittleFS");

        systemPrintln("r) Force system reset");

        systemPrintln("x) Exit");

        byte incoming = getCharacterNumber();

        if (incoming == 1)
        {
            systemPrint("Enter periodic print interval in milliseconds: ");
            int printInterval = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((printInterval != INPUT_RESPONSE_GETNUMBER_EXIT) && (printInterval != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (printInterval < 1000 || printInterval > 60000)
                {
                    systemPrintln("Invalid interval");
                }
                else
                {
                    settings.periodicPrintInterval_ms = printInterval;
                }
            }
        }
        else if (incoming == 2)
            settings.enableHeapReport ^= 1;
        else if (incoming == 3)
            settings.enableTaskReports ^= 1;
        else if (incoming == 4)
            settings.enablePrintBadMessages ^= 1;
        else if (incoming == 5)
            settings.enablePrintStates ^= 1;
        else if (incoming == 6)
            settings.enablePrintDuplicateStates ^= 1;
        else if (incoming == 7)
            settings.enablePrintRtcSync ^= 1;
        else if (incoming == 8)
            settings.enablePrintConditions ^= 1;
        else if (incoming == 9)
            settings.enablePrintConsumers ^= 1;
        else if (incoming == 10)
            settings.enablePrintIdleTime ^= 1;
        else if (incoming == 11)
            settings.enablePrintGNSSMessages ^= 1;
        else if (incoming == 12)
            settings.enablePrintRingBufferOffsets ^= 1;
        else if (incoming == 13)
            settings.enablePrintBufferOverrun ^= 1;
        else if (incoming == 14)
            settings.echoUserInput ^= 1;
        else if (incoming == 15)
            printPartitionTable();

        else if (incoming == 20)
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
        else if (incoming == 21)
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
                    // Stop the UART1 tssks to prevent the system from crashing
                    tasksStopUART1();

                    // Update the buffer size
                    settings.gnssHandlerBufferSize = queSize; // Recorded to NVM and file
                    recordSystemSettings();

                    // Reboot the system
                    ESP.restart();
                }
            }
        }
        else if (incoming == 22)
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
        else if (incoming == 23)
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
        else if (incoming == 24)
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
        else if (incoming == 25)
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

        else if (incoming == 30)
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
        else if (incoming == 31)
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
        else if (incoming == 32)
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
        else if (incoming == 33)
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
        systemPrintln("Menu: Operation\r\n");

        systemPrint("1) RX Clock Bias Lock Limit (ms): ");
        systemPrintf("%.3e\r\n", settings.rxClkBiasLockLimit_ms);

        systemPrint("2) Pulse-Per-Second Interval: ");
        systemPrintln(mosaicPPSParametersInterval[settings.ppsInterval]);

        systemPrint("3) Pulse-Per-Second Polarity: ");
        systemPrintln(mosaicPPSParametersPolarity[settings.ppsPolarity]);

        systemPrint("4) Pulse-Per-Second Delay (ns): ");
        systemPrintln(settings.ppsDelay_ns);

        systemPrint("5) Pulse-Per-Second Time Scale: ");
        systemPrintln(mosaicPPSParametersTimeScale[settings.ppsTimeScale]);

        systemPrint("6) Pulse-Per-Second Max Sync Age (s): ");
        systemPrintln(settings.ppsMaxSyncAge_s);

        systemPrint("7) Pulse-Per-Second Pulse Width (ms): ");
        systemPrintln(settings.ppsPulseWidth_ms);

        systemPrint("8) Pk (PI P term): ");
        systemPrintf("%.3e\r\n", settings.Pk);

        systemPrint("9) Ik (PI I term): ");
        systemPrintf("%.3e\r\n", settings.Ik);

        systemPrintln("\r\nx) Exit");

        byte incoming = getCharacterNumber();

        if (incoming == 1)
        {
            systemPrint("Enter RX Clock Bias Lock Limit in milliseconds: ");
            double lockLimit = getDouble();
            if (lockLimit <= 0.0 || lockLimit >= 1000.0) // Arbitrary 1s limit
                systemPrintln("Error: Lock Limit is out of range");
            else
            {
                settings.rxClkBiasLockLimit_ms = lockLimit; // Recorded to NVM at main menu exit
                ppsStarted = false; // Restart PPS afterwards
            }
        }
        else if (incoming == 2)
        {
            settings.ppsInterval++;
            if ((settings.ppsInterval >= mosaicPPSParametersIntervalEntries) || (settings.ppsInterval < 0))
                settings.ppsInterval = 0;
            ppsStarted = false; // Restart PPS afterwards
        }
        else if (incoming == 3)
        {
            settings.ppsPolarity++;
            if ((settings.ppsPolarity >= mosaicPPSParametersPolarityEntries) || (settings.ppsPolarity < 0))
                settings.ppsPolarity = 0;
            ppsStarted = false; // Restart PPS afterwards
        }
        else if (incoming == 4)
        {
            systemPrint("Enter Pulse-Per-Second Delay in nanoseconds: ");
            double dly = getDouble();
            if (dly < -1000000.00 || dly > 1000000.00)
                systemPrintln("Error: Delay is out of range");
            else
            {
                settings.ppsDelay_ns = dly;
                ppsStarted = false; // Restart PPS afterwards
            }
        }
        else if (incoming == 5)
        {
            settings.ppsTimeScale++;
            if ((settings.ppsTimeScale >= mosaicPPSParametersTimeScaleEntries) || (settings.ppsTimeScale < 0))
                settings.ppsPolarity = 0;
            ppsStarted = false; // Restart PPS afterwards
        }
        else if (incoming == 6)
        {
            systemPrint("Enter Max Sync Age in seconds (0 to 3600): ");
            int syncAge = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((syncAge != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (syncAge != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (syncAge < 0 || syncAge > 3600)
                    systemPrintln("Error: Max Sync Age is out of range");
                else
                {
                    settings.ppsMaxSyncAge_s = syncAge;
                    ppsStarted = false; // Restart PPS afterwards
                }
            }
        }
        else if (incoming == 7)
        {
            systemPrint("Enter Pulse Width in milliseconds: ");
            double width = getDouble();
            if (width <= 0.000001 || width > 1000.000000)
                systemPrintln("Error: Pulse Width is out of range");
            else
            {
                settings.ppsPulseWidth_ms = width;
                ppsStarted = false; // Restart PPS afterwards
            }
        }
        else if (incoming == 8)
        {
            systemPrint("Enter the PI P term: ");
            double p = getDouble();
            if (p >= 0.0 || p <= -1000.0) // Arbitrary limits
                systemPrintln("Error: term is out of range");
            else
            {
                settings.Pk = p; // Recorded to NVM at main menu exit
                ppsStarted = false; // Restart PPS afterwards
            }
        }
        else if (incoming == 9)
        {
            systemPrint("Enter the PI I term: ");
            double i = getDouble();
            if (i >= 0.0 || i <= -1000.0) // Arbitrary limits
                systemPrintln("Error: term is out of range");
            else
            {
                settings.Ik = i; // Recorded to NVM at main menu exit
                ppsStarted = false; // Restart PPS afterwards
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

        systemPrint(", PPS: ");
        systemPrint(ppsStarted ? "On" : "Off");

        if ((gnssClockBias_ms >= 1.0) || (gnssClockBias_ms <= -1.0))
            systemPrintf(", Bias: %.3fms",
                (float)gnssClockBias_ms);
        else if ((gnssClockBias_ms >= 0.001) || (gnssClockBias_ms <= -0.001))
            systemPrintf(", Bias: %.3fus",
                (float)(gnssClockBias_ms * 1000.0));
        else
            systemPrintf(", Bias: %.3fns",
                (float)(gnssClockBias_ms * 1000000.0));

        systemPrint(", TCXO Control: ");
        systemPrint(myTCXO.getFrequencyControlWord());
        
        systemPrintln();
    }
}

