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
        {
            switch (settings.enablePrintConditions)
            {
                default:
                    systemPrintln("Unknown");
                    break;
                case 0:
                    systemPrintln("Disabled");
                    break;
                case 1:
                    systemPrintln("Enabled (Text)");
                    break;
                case 2:
                    systemPrintln("Enabled (CSV)");
                    break;
            }
        }

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

        systemPrint("15) Print addditional debug messages: ");
        systemPrintf("%s\r\n", settings.printDebugMessages ? "Enabled" : "Disabled");

        systemPrintf("16) Print partition table\r\n");

        systemPrintf("17) Print settings file\r\n");

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
        {
            settings.enablePrintConditions += 1;
            if (settings.enablePrintConditions > 2)
                settings.enablePrintConditions = 0;
        }
        else if (incoming == 9)
            settings.enablePrintConsumers ^= 1;
        else if (incoming == 10)
            settings.enablePrintIdleTime ^= 1;
        else if (incoming == 11)
        {
            settings.enablePrintGNSSMessages ^= 1;
        }
        else if (incoming == 12)
            settings.enablePrintRingBufferOffsets ^= 1;
        else if (incoming == 13)
            settings.enablePrintBufferOverrun ^= 1;
        else if (incoming == 14)
            settings.echoUserInput ^= 1;
        else if (incoming == 15)
            settings.printDebugMessages ^= 1;          
        else if (incoming == 16)
            printPartitionTable();
        else if (incoming == 17)
            printSettings();

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
            systemPrintln("Warning: changing the Handler Buffer Size will cause a restart. Enter 0 to abort");
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
            systemPrintln("Warning: changing the Receive Buffer Size will cause a restart. Enter 0 to abort");
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

            gnssHardReset();

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

        if (presentTcxoTemperature)
        {
        systemPrintf("1)  Minimum TCXO Warm Up (s):                          %ld\r\n", settings.tcxoMinWarmup_s);
        systemPrintf("2)  Required TCXO Temperature Stability (ADU):         %.2f\r\n", settings.tcxoTemperatureStability);
        }

        systemPrint("3)  Required bias stability for frequency lock (s):    ");
        systemPrintf("%.3e\r\n", settings.rxFrequencyLockErrorLimit_s);

        systemPrint("4)  PI P term for initial frequency steering:          ");
        systemPrintf("%.3e\r\n", settings.PkSteer);

        systemPrint("5)  PI I term for initial frequency steering:          ");
        systemPrintf("%.3e\r\n", settings.IkSteer);

        systemPrint("6)  TCXO steering ramp rate limit (s/s):               ");
        systemPrintf("%.3e\r\n", settings.tcxoRampRateLimit_sps);

        systemPrint("7)  TCXO steering ramp step size (s):                  ");
        systemPrintf("%.3e\r\n", settings.tcxoRampStepSize_s);

        systemPrint("8)  Required bias for phase lock (s):                  ");
        systemPrintf("%.3e\r\n", settings.rxPhaseErrorLimit_s);

        systemPrint("9)  PI P term for final TCXO disciplining:             ");
        systemPrintf("%.3e\r\n", settings.Pk);

        systemPrint("10) PI I term for final TCXO disciplining:             ");
        systemPrintf("%.3e\r\n", settings.Ik);

        systemPrint("11) Prefer non-composite GPS bias - if available:      ");
        systemPrintf("%s\r\n", settings.preferNonCompositeGPSBias ? "Enabled" : "Disabled");

        systemPrint("12) Prefer non-composite Galileo bias - if available:  ");
        systemPrintf("%s\r\n", settings.preferNonCompositeGalileoBias ? "Enabled" : "Disabled");

        systemPrint("13) Pulse-Per-Second Interval:                         ");
        systemPrintln(mosaicPPSParametersInterval[settings.ppsInterval]);

        systemPrint("14) Pulse-Per-Second Polarity:                         ");
        systemPrintln(mosaicPPSParametersPolarity[settings.ppsPolarity]);

        systemPrint("15) Pulse-Per-Second Delay (ns):                       ");
        systemPrintln(settings.ppsDelay_ns);

        systemPrint("16) Pulse-Per-Second Time Scale:                       ");
        systemPrintln(mosaicPPSParametersTimeScale[settings.ppsTimeScale]);

        systemPrint("17) Pulse-Per-Second Max Sync Age (s):                 ");
        systemPrintln(settings.ppsMaxSyncAge_s);

        systemPrint("18) Pulse-Per-Second Pulse Width (ms):                 ");
        systemPrintln(settings.ppsPulseWidth_ms);

        systemPrint("19) TCP Server (IPS1):                                 ");
        systemPrintf("%s\r\n", settings.enableTCPServer ? "Enabled" : "Disabled");

        systemPrint("20) TCP Server Port:                                   ");
        systemPrintln(settings.tcpServerPort);

        if (presentTcxoSaveControl)
            systemPrintln("21) Save TCXO control word to TCXO memory");

        systemPrintln("\r\nx) Exit");

        byte incoming = getCharacterNumber();

        if (presentTcxoTemperature && (incoming == 1))
        {
            systemPrint("Enter the minimum TCXO warm up in seconds: ");
            int warmup = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((warmup != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (warmup != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (warmup < 1 || warmup > 3600)
                    systemPrintln("Error: Warm up is out of range");
                else
                {
                    settings.tcxoMinWarmup_s = warmup;
                }
            }
        }
        else if (presentTcxoTemperature && (incoming == 2))
        {
            systemPrint("Enter the required temperature stability in ADU: ");
            double stability;
            if (getDouble(stability))
            {
                if (stability < 0.01 || stability > 100.0)
                    systemPrintln("Error: Temperature stability is out of range");
                else
                {
                    settings.tcxoTemperatureStability = stability;
                }
            }
        }
        else if (incoming == 3)
        {
            systemPrintln("The firmware will stay in STATE_GNSS_FINETIME");
            systemPrintf("until the change in the bias is less than %.3es.\r\n", settings.rxFrequencyLockErrorLimit_s);
            systemPrint("Enter the new bias stability in seconds: ");
            double limit;
            if (getDouble(limit))
            {
                if (limit <= 0.0 || limit >= 1.0e-3) // Arbitrary limits
                    systemPrintln("Error: bias stability is out of range");
                else
                {
                    settings.rxFrequencyLockErrorLimit_s = limit; // Recorded to NVM at main menu exit
                }
            }
        }
        else if (incoming == 4)
        {
            systemPrint("Enter the PI P term for initial frequency steering: ");
            double p;
            if (getDouble(p))
            {
                if (p < 0.0 || p > 10.0) // Arbitrary limits
                    systemPrintln("Error: term is out of range");
                else
                {
                    settings.PkSteer = p; // Recorded to NVM at main menu exit
                }
            }
        }
        else if (incoming == 5)
        {
            systemPrint("Enter the PI I term for initial frequency steering: ");
            double i;
            if (getDouble(i))
            {
                if (i < 0.0 || i > 10.0) // Arbitrary limits
                    systemPrintln("Error: term is out of range");
                else
                {
                    settings.IkSteer = i; // Recorded to NVM at main menu exit
                }
            }
        }
        else if (incoming == 6)
        {
            systemPrint("Enter the TCXO steering ramp rate limit in seconds per second: ");
            double dbl;
            if (getDouble(dbl))
            {
                if (dbl < 1.0e-9 || dbl > 1.0e-3)
                    systemPrintln("Error: steering rate limit is out of range");
                else
                {
                    settings.tcxoRampRateLimit_sps = dbl;
                }
            }
        }
        else if (incoming == 7)
        {
            systemPrint("Enter the TCXO steering ramp step size in seconds: ");
            double dbl;
            if (getDouble(dbl))
            {
                if (dbl < 1.0e-9 || dbl > 1.0e-6)
                    systemPrintln("Error: Steering Ramp Step Size is out of range");
                else
                {
                    settings.tcxoRampStepSize_s = dbl;
                }
            }
        }
        else if (incoming == 8)
        {
            systemPrintln("The firmware will stay in STATE_GNSS_FREQUENCY_LOCK");
            systemPrintf("until the bias is less than %.3es.\r\n", settings.rxPhaseErrorLimit_s);
            systemPrint("Enter the new bias limit in seconds: ");
            double limit;
            if (getDouble(limit))
            {
                if (limit <= 0.0 || limit >= 1.0e-3) // Arbitrary 1ms limit
                    systemPrintln("Error: bias limit is out of range");
                else
                {
                    settings.rxPhaseErrorLimit_s = limit; // Recorded to NVM at main menu exit
                }
            }
        }
        else if (incoming == 9)
        {
            systemPrint("Enter the PI P term: ");
            double p;
            if (getDouble(p))
            {
                if (p < 0.0 || p > 10.0) // Arbitrary limits
                    systemPrintln("Error: term is out of range");
                else
                {
                    settings.Pk = p; // Recorded to NVM at main menu exit
                }
            }
        }
        else if (incoming == 10)
        {
            systemPrint("Enter the PI I term: ");
            double i;
            if (getDouble(i))
            {
                if (i < 0.0 || i > 10.0) // Arbitrary limits
                    systemPrintln("Error: term is out of range");
                else
                {
                    settings.Ik = i; // Recorded to NVM at main menu exit
                }
            }
        }
        else if (incoming == 11)
        {
            settings.preferNonCompositeGPSBias ^= 1;
            if (settings.preferNonCompositeGPSBias)
                settings.preferNonCompositeGalileoBias = false;
        }
        else if (incoming == 12)
        {
            settings.preferNonCompositeGalileoBias ^= 1;
            if (settings.preferNonCompositeGalileoBias)
                settings.preferNonCompositeGPSBias = false;
        }
        else if (incoming == 13)
        {
            settings.ppsInterval++;
            if ((settings.ppsInterval >= mosaicPPSParametersIntervalEntries) || (settings.ppsInterval < 0))
                settings.ppsInterval = 0;
            ppsStarted = false; // Restart PPS afterwards
        }
        else if (incoming == 14)
        {
            settings.ppsPolarity++;
            if ((settings.ppsPolarity >= mosaicPPSParametersPolarityEntries) || (settings.ppsPolarity < 0))
                settings.ppsPolarity = 0;
            ppsStarted = false; // Restart PPS afterwards
        }
        else if (incoming == 15)
        {
            systemPrint("Enter the Pulse-Per-Second Delay in nanoseconds: ");
            double dly;
            if (getDouble(dly))
            {
                if (dly < -1000000.00 || dly > 1000000.00)
                    systemPrintln("Error: delay is out of range");
                else
                {
                    settings.ppsDelay_ns = dly;
                    ppsStarted = false; // Restart PPS afterwards
                }
            }
        }
        else if (incoming == 16)
        {
            settings.ppsTimeScale++;
            if ((settings.ppsTimeScale >= mosaicPPSParametersTimeScaleEntries) || (settings.ppsTimeScale < 0))
                settings.ppsTimeScale = 0;
            ppsStarted = false; // Restart PPS afterwards
        }
        else if (incoming == 17)
        {
            systemPrint("Enter the Max Sync Age in seconds (0 to 3600): ");
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
        else if (incoming == 18)
        {
            systemPrint("Enter the Pulse Width in milliseconds: ");
            double width;
            if (getDouble(width))
            {
                if (width <= 0.000001 || width > 1000.000000)
                    systemPrintln("Error: Pulse Width is out of range");
                else
                {
                    settings.ppsPulseWidth_ms = width;
                    ppsStarted = false; // Restart PPS afterwards
                }
            }
        }
        else if (incoming == 19)
        {
            settings.enableTCPServer ^= 1;
        }
        else if (incoming == 20)
        {
            systemPrint("Enter the TCP Server Port: ");
            int port = getNumber(); // Returns EXIT, TIMEOUT, or long
            if ((port != INPUT_RESPONSE_GETNUMBER_EXIT) &&
                (port != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
            {
                if (port < 1 || port > 65534)
                    systemPrintln("Error: Port is out of range");
                else
                {
                    settings.tcpServerPort = port;
                }
            }
        }
        else if (presentTcxoSaveControl && (incoming == 21))
        {
            systemPrintln("\r\nSaving the TCXO control word to TCXO memory. Press 'y' to confirm:");
            byte bContinue = getCharacterNumber();
            if (bContinue == 'y')
            {
                myTCXO->saveFrequencyControlValue();
            }
            else
                systemPrintln("Save aborted");
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
void printCurrentConditions(bool CSV)
{
    static bool firstTime = true;

    if (online.gnss == true)
    {
        if (CSV)
        {
            if (firstTime)
            {
                systemPrint("YYYY/MM/DD,HH:MM:SS,Epoch,Lat,Lon,Alt,TimeSys,Error,Fine,PPS,Bias,Drift,Source,TCXO,Pk,Ik");
                if (online.pht)
                    systemPrintln(",Press,Temp,Hum");
                else
                    systemPrintln();
                firstTime = false;
            }

            uint32_t epochSecs;
            uint32_t epochMillis;
            convertGnssTimeToEpoch(&epochSecs, &epochMillis);

            systemPrintf("%04d/%02d/%02d,%02d:%02d:%02d",
                gnssYear, gnssMonth, gnssDay, gnssHour, gnssMinute, gnssSecond);
            
            systemPrintf(",%lu.%03lu", epochSecs, epochMillis);
            
            systemPrint(",");
            systemPrint(gnssLatitude_d, 7);

            systemPrint(",");
            systemPrint(gnssLongitude_d, 7);

            systemPrint(",");
            systemPrint(gnssAltitude_m, 4);

            systemPrint(",");
            systemPrint(mosaicTimeSystemNameFromId(gnssTimeSys));

            systemPrint(",");
            systemPrint(gnssError);

            systemPrint(",");
            systemPrint(gnssFineTime);

            systemPrint(",");
            systemPrint(ppsStarted);

            systemPrintf(",%.3e", tcxoClockBias_ms / 1000.0); // Display clock bias in seconds

            systemPrintf(",%.3e", tcxoClockDrift_ppm); // Display clock drift in PPM

            systemPrint(",");
            systemPrint((const char *)rxClkBiasSource),

            systemPrintf(",%lld", getFrequencyControlWord());
            
            systemPrint(",");
            systemPrint(settings.Pk, 3);
            
            systemPrint(",");
            systemPrint(settings.Ik, 3);
            
            if (online.pht)
            {
                systemPrintf(",%.0f", pressure);
                systemPrintf(",%.1f", temperature);
                systemPrintf(",%.0f", humidity);
            }

            if (presentTcxoTemperature)
                systemPrintf(",%d", (uint16_t)round(tcxoTemperature));

            if (settings.printDebugMessages == true)
            {
                for (int i = 0; i < numSystemStatesNames; i++)
                {
                    if (SystemStatesNames[i].systemState == systemState)
                    {
                        systemPrintf(",%s", SystemStatesNames[i].stateName);
                        break;
                    }
                }

                systemPrintf(",%.3e", rate_s);
                systemPrintf(",%.3e", rate_held_s);
                systemPrintf(",%.3e", setpoint_s);
                systemPrintf(",%.3e", error_s);
            }

            systemPrintln();
        }
        else
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

            if ((tcxoClockBias_ms >= 1.0) || (tcxoClockBias_ms <= -1.0))
                systemPrintf(", Bias: %.3fms",
                    (float)tcxoClockBias_ms);
            else if ((tcxoClockBias_ms >= 0.001) || (tcxoClockBias_ms <= -0.001))
                systemPrintf(", Bias: %.3fus",
                    (float)(tcxoClockBias_ms * 1000.0));
            else
                systemPrintf(", Bias: %.3fns",
                    (float)(tcxoClockBias_ms * 1000000.0));

            systemPrintf(", Drift: %.3e", tcxoClockDrift_ppm);

            systemPrint(", Source: ");
            systemPrint((const char *)rxClkBiasSource),

            systemPrint(", TCXO Control: ");
            systemPrint(getFrequencyControlWord());
            
            if (online.pht)
            {
                systemPrintf(", Pressure: %.0fhPa", pressure);
                systemPrintf(", Temperature: %.1fC", temperature);
                systemPrintf(", Humidity: %.0f%%RH", humidity);
            }

            if (presentTcxoTemperature)
                systemPrintf(", TCXO Temp: %d", (uint16_t)(round(tcxoTemperature)));

            systemPrintln();
        }
    }
}

