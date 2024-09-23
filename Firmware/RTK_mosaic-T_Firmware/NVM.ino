void loadSettings()
{
    loadSystemSettingsFromFileLFS(settingsFileName, &settings);
}

// Set the settingsFileName and coordinate file names used many places
void setSettingsFileName()
{
    snprintf(settingsFileName, sizeof(settingsFileName), "/%s_Settings.txt", platformFilePrefix);
}

void recordSystemSettings()
{
    settings.sizeOfSettings = sizeof(settings); // Update to current setting size

    recordSystemSettingsToFileLFS(settingsFileName); // Record to LFS if available
}

void recordSystemSettingsToFileLFS(char *fileName)
{
    if (online.fs == true)
    {
        if (LittleFS.exists(fileName))
        {
            LittleFS.remove(fileName);
            systemPrintf("Removing LittleFS: %s\r\n", fileName);
        }

        File settingsFile = LittleFS.open(fileName, FILE_WRITE);
        if (!settingsFile)
        {
            systemPrintf("Failed to write to settings file %s\r\n", fileName);
        }
        else
        {
            recordSystemSettingsToFile(&settingsFile); // Record all the settings via strings to file
            settingsFile.close();
            systemPrintf("Settings recorded to LittleFS: %s\r\n", fileName);
        }
    }
}

// Write the settings struct to a clear text file
void recordSystemSettingsToFile(File *settingsFile)
{
    settingsFile->printf("%s=%d\r\n", "sizeOfSettings", settings.sizeOfSettings);
    settingsFile->printf("%s=%d\r\n", "rtkIdentifier", settings.rtkIdentifier);

    char firmwareVersion[30]; // v1.3 December 31 2021
    getFirmwareVersion(firmwareVersion, sizeof(firmwareVersion), true);
    settingsFile->printf("%s=%s\r\n", "rtkFirmwareVersion", firmwareVersion);

    settingsFile->printf("%s=%d\r\n", "printDebugMessages", settings.printDebugMessages);
    settingsFile->printf("%s=%d\r\n", "enableHeapReport", settings.enableHeapReport);
    settingsFile->printf("%s=%d\r\n", "enableTaskReports", settings.enableTaskReports);
    settingsFile->printf("%s=%d\r\n", "lastState", settings.lastState);

    settingsFile->printf("%s=%d\r\n", "uartReceiveBufferSize", settings.uartReceiveBufferSize);
    settingsFile->printf("%s=%d\r\n", "gnssHandlerBufferSize", settings.gnssHandlerBufferSize);
    settingsFile->printf("%s=%d\r\n", "enablePrintBufferOverrun", settings.enablePrintBufferOverrun);

    settingsFile->printf("%s=%d\r\n", "serialGNSSRxFullThreshold", settings.serialGNSSRxFullThreshold);
    settingsFile->printf("%s=%d\r\n", "gnssReadTaskPriority", settings.gnssReadTaskPriority);
    settingsFile->printf("%s=%d\r\n", "handleGnssDataTaskPriority", settings.handleGnssDataTaskPriority);
    settingsFile->printf("%s=%d\r\n", "gnssReadTaskCore", settings.gnssReadTaskCore);
    settingsFile->printf("%s=%d\r\n", "handleGnssDataTaskCore", settings.handleGnssDataTaskCore);
    settingsFile->printf("%s=%d\r\n", "i2cInterruptsCore", settings.i2cInterruptsCore);
    settingsFile->printf("%s=%d\r\n", "gnssUartInterruptsCore", settings.gnssUartInterruptsCore);

    settingsFile->printf("%s=%d\r\n", "serialTimeoutGNSS", settings.serialTimeoutGNSS);
    settingsFile->printf("%s=%lu\r\n", "dataPortBaud", settings.dataPortBaud);

    settingsFile->printf("%s=%d\r\n", "enablePrintBadMessages", settings.enablePrintBadMessages);
    settingsFile->printf("%s=%d\r\n", "enablePrintStates", settings.enablePrintStates);
    settingsFile->printf("%s=%d\r\n", "enablePrintDuplicateStates", settings.enablePrintDuplicateStates);
    settingsFile->printf("%s=%d\r\n", "enablePrintRtcSync", settings.enablePrintRtcSync);
    settingsFile->printf("%s=%d\r\n", "enablePrintIdleTime", settings.enablePrintIdleTime);
    settingsFile->printf("%s=%d\r\n", "enablePrintConditions", settings.enablePrintConditions);
    settingsFile->printf("%s=%d\r\n", "enablePrintConsumers", settings.enablePrintConsumers);
    settingsFile->printf("%s=%lu\r\n", "periodicPrintInterval_ms", settings.periodicPrintInterval_ms);
    settingsFile->printf("%s=%d\r\n", "enablePrintGNSSMessages", settings.enablePrintGNSSMessages);
    settingsFile->printf("%s=%d\r\n", "enablePrintRingBufferOffsets", settings.enablePrintRingBufferOffsets);
    settingsFile->printf("%s=%d\r\n", "disableSetupButton", settings.disableSetupButton);
    settingsFile->printf("%s=%d\r\n", "echoUserInput", settings.echoUserInput);

    settingsFile->printf("%s=%d\r\n", "ppsInterval", settings.ppsInterval);
    settingsFile->printf("%s=%d\r\n", "ppsPolarity", settings.ppsPolarity);
    settingsFile->printf("%s=%0.3f\r\n", "ppsDelay_ns", settings.ppsDelay_ns);
    settingsFile->printf("%s=%d\r\n", "ppsTimeScale", settings.ppsTimeScale);
    settingsFile->printf("%s=%d\r\n", "ppsMaxSyncAge_s", settings.ppsMaxSyncAge_s);
    settingsFile->printf("%s=%0.6f\r\n", "ppsPulseWidth_ms", settings.ppsPulseWidth_ms);

    settingsFile->printf("%s=%ld\r\n", "tcxoControl", settings.tcxoControl);
    settingsFile->printf("%s=%0.3e\r\n", "rxClkBiasInitialLimit_ms", settings.rxClkBiasInitialLimit_ms);
    settingsFile->printf("%s=%0.3e\r\n", "rxClkBiasLockLimit_ms", settings.rxClkBiasLockLimit_ms);
    settingsFile->printf("%s=%d\r\n", "rxClkBiasLimitCount", settings.rxClkBiasLimitCount);
    settingsFile->printf("%s=%0.3e\r\n", "Pk", settings.Pk);
    settingsFile->printf("%s=%0.3e\r\n", "Ik", settings.Ik);
    settingsFile->printf("%s=%d\r\n", "preferNonCompositeGPSBias", settings.preferNonCompositeGPSBias);
    settingsFile->printf("%s=%d\r\n", "preferNonCompositeGalileoBias", settings.preferNonCompositeGalileoBias);

    //settingsFile->printf("%s=%d\r\n", "", settings.);

    // Add new settings above <------------------------------------------------------------>
}

// Given a fileName, parse the file and load the given settings struct
// Returns true if some settings were loaded from a file
// Returns false if a file was not opened/loaded
bool loadSystemSettingsFromFileLFS(char *fileName, Settings *settings)
{
    if (online.fs == false)
        return false;
        
    systemPrintf("Reading setting fileName: %s\r\n", fileName);

    File settingsFile = LittleFS.open(fileName, FILE_READ);
    if (!settingsFile)
    {
        systemPrintf("settingsFile not found in LittleFS\r\n");
        return (false);
    }

    char line[100];
    int lineNumber = 0;

    while (settingsFile.available())
    {
        // Get the next line from the file
        int n;
        n = getLine(&settingsFile, line, sizeof(line));

        if (n <= 0)
        {
            systemPrintf("Failed to read line %d from settings file\r\n", lineNumber);
        }
        else if (line[n - 1] != '\n' && n == (sizeof(line) - 1))
        {
            systemPrintf("Settings line %d too long\r\n", lineNumber);
            if (lineNumber == 0)
            {
                // If we can't read the first line of the settings file, give up
                systemPrintln("Giving up on settings file");
                return (false);
            }
        }
        else if (parseLine(line, settings) == false)
        {
            systemPrintf("Failed to parse line %d: %s\r\n", lineNumber, line);
            if (lineNumber == 0)
            {
                // If we can't read the first line of the settings file, give up
                systemPrintln("Giving up on settings file");
                return (false);
            }
        }

        lineNumber++;
        if (lineNumber > 400) // Arbitrary limit. Catch corrupt files.
        {
            systemPrintf("Giving up reading file: %s\r\n", fileName);
            break;
        }
    }

    settingsFile.close();
    return (true);
}

// Convert a given line from file into a settingName and value
// Sets the setting if the name is known
bool parseLine(char *str, Settings *settings)
{
    char *ptr;

    // Set strtok start of line.
    str = strtok(str, "=");
    if (!str)
    {
        systemPrintln("parseLine Fail");
        return false;
    }

    // Store this setting name
    char settingName[100];
    snprintf(settingName, sizeof(settingName), "%s", str);

    double d = 0.0;
    char settingString[100] = "";

    // Move pointer to end of line
    str = strtok(nullptr, "\n");
    if (!str)
    {
        // This line does not contain a \n or the settingString is zero length
        // so there is nothing to parse
        // https://github.com/sparkfun/SparkFun_RTK_Firmware/issues/77
    }
    else
    {
        // if (strcmp(settingName, "ntripServer_CasterHost") == 0) //Debug
        // if (strcmp(settingName, "profileName") == 0) //Debug
        //   systemPrintf("Found problem spot raw: %s\r\n", str);

        // Assume the value is a string such as 8d8a48b. The leading number causes skipSpace to fail.
        // If settingString has a mix of letters and numbers, just convert to string
        snprintf(settingString, sizeof(settingString), "%s", str);

        // Check if string is mixed: 8a011EF, 192.168.1.1, -102.4, t6-h4$, etc.
        bool hasSymbol = false;
        int decimalCount = 0;
        int minusCount = 0;
        int eCount = 0;
        for (int x = 0; x < strlen(settingString); x++)
        {
            if (settingString[x] == '.')
                decimalCount++;
            else if (settingString[x] == '-')
                minusCount++; // Multiple -'s are good (scientific notation)
            else if (settingString[x] == 'e')
                eCount++; // e's are good (scientific notation)
            else if (isAlpha(settingString[x]))
                hasSymbol = true;
            else if (isDigit(settingString[x]) == false)
                hasSymbol = true;
        }

        if (hasSymbol || decimalCount > 1 || minusCount > 2 || eCount > 1)
        {
            // It's a mess. Skip strtod.
        }
        else
        {
            // Attempt to convert string to double
            d = strtod(str, &ptr);

            if (d == 0.0) // strtod failed, may be string or may be 0 but let it pass
            {
                snprintf(settingString, sizeof(settingString), "%s", str);
            }
            else
            {
                if (str == ptr || *skipSpace(ptr))
                    return false; // Check str pointer
            }
        }
    }

    //systemPrintf("settingName: %s - value: %s - d: %0.9f\r\n", settingName, settingString, d);

    // Get setting name
    if (strcmp(settingName, "sizeOfSettings") == 0)
    {
        // We may want to cause a factory reset from the settings file rather than the menu
        // If user sets sizeOfSettings to -1 in config file, RTK Surveyor will factory reset
        if (d == -1)
        {
            // Erase file system, erase settings file, reset GNSS module, display message on OLED
            factoryReset(true); // We already have the SD semaphore
        }

        // Check to see if this setting file is compatible with this version of RTK Surveyor
        if (d != sizeof(Settings))
            systemPrintf("Settings size is %d but current firmware expects %d. Attempting to use settings from file.\r\n", (int)d,
                  sizeof(Settings));
    }

    else if (strcmp(settingName, "rtkIdentifier") == 0)
    {
    } // Do nothing. Just read it to avoid 'Unknown setting' error
    else if (strcmp(settingName, "rtkFirmwareVersion") == 0)
    {
    } // Do nothing. Just read it to avoid 'Unknown setting' error

    else if (strcmp(settingName, "printDebugMessages") == 0)
        settings->printDebugMessages = d;
    else if (strcmp(settingName, "enableHeapReport") == 0)
        settings->enableHeapReport = d;
    else if (strcmp(settingName, "enableTaskReports") == 0)
        settings->enableTaskReports = d;
    else if (strcmp(settingName, "lastState") == 0)
    {
        if (settings->lastState != (SystemState)d)
        {
            settings->lastState = (SystemState)d;
        }
    }
    else if (strcmp(settingName, "uartReceiveBufferSize") == 0)
        settings->uartReceiveBufferSize = d;
    else if (strcmp(settingName, "gnssHandlerBufferSize") == 0)
        settings->gnssHandlerBufferSize = d;
    else if (strcmp(settingName, "enablePrintBufferOverrun") == 0)
        settings->enablePrintBufferOverrun = d;

    else if (strcmp(settingName, "serialGNSSRxFullThreshold") == 0)
        settings->serialGNSSRxFullThreshold = d;
    else if (strcmp(settingName, "gnssReadTaskPriority") == 0)
        settings->gnssReadTaskPriority = d;
    else if (strcmp(settingName, "handleGnssDataTaskPriority") == 0)
        settings->handleGnssDataTaskPriority = d;
    else if (strcmp(settingName, "gnssReadTaskCore") == 0)
        settings->gnssReadTaskCore = d;
    else if (strcmp(settingName, "handleGnssDataTaskCore") == 0)
        settings->handleGnssDataTaskCore = d;
    else if (strcmp(settingName, "i2cInterruptsCore") == 0)
        settings->i2cInterruptsCore = d;
    else if (strcmp(settingName, "gnssUartInterruptsCore") == 0)
        settings->gnssUartInterruptsCore = d;

    else if (strcmp(settingName, "serialTimeoutGNSS") == 0)
        settings->serialTimeoutGNSS = d;
    else if (strcmp(settingName, "dataPortBaud") == 0)
        settings->dataPortBaud = d;

    else if (strcmp(settingName, "enablePrintBadMessages") == 0)
        settings->enablePrintBadMessages = d;
    else if (strcmp(settingName, "enablePrintStates") == 0)
        settings->enablePrintStates = d;
    else if (strcmp(settingName, "enablePrintDuplicateStates") == 0)
        settings->enablePrintDuplicateStates = d;
    else if (strcmp(settingName, "enablePrintRtcSync") == 0)
        settings->enablePrintRtcSync = d;
    else if (strcmp(settingName, "enablePrintIdleTime") == 0)
        settings->enablePrintIdleTime = d;
    else if (strcmp(settingName, "enablePrintConditions") == 0)
        settings->enablePrintConditions = d;
    else if (strcmp(settingName, "enablePrintConsumers") == 0)
        settings->enablePrintConsumers = d;
    else if (strcmp(settingName, "periodicPrintInterval_ms") == 0)
        settings->periodicPrintInterval_ms = d;
    else if (strcmp(settingName, "enablePrintGNSSMessages") == 0)
        settings->enablePrintGNSSMessages = d;
    else if (strcmp(settingName, "enablePrintRingBufferOffsets") == 0)
        settings->enablePrintRingBufferOffsets = d;
    else if (strcmp(settingName, "disableSetupButton") == 0)
        settings->disableSetupButton = d;
    else if (strcmp(settingName, "echoUserInput") == 0)
        settings->echoUserInput = d;

    else if (strcmp(settingName, "ppsInterval") == 0)
        settings->ppsInterval = d;
    else if (strcmp(settingName, "ppsPolarity") == 0)
        settings->ppsPolarity = d;
    else if (strcmp(settingName, "ppsDelay_ns") == 0)
        settings->ppsDelay_ns = d;
    else if (strcmp(settingName, "ppsTimeScale") == 0)
        settings->ppsTimeScale = d;
    else if (strcmp(settingName, "ppsMaxSyncAge_s") == 0)
        settings->ppsMaxSyncAge_s = d;
    else if (strcmp(settingName, "ppsPulseWidth_ms") == 0)
        settings->ppsPulseWidth_ms = d;

    else if (strcmp(settingName, "tcxoControl") == 0)
        settings->tcxoControl = d;
    else if (strcmp(settingName, "rxClkBiasInitialLimit_ms") == 0)
        settings->rxClkBiasInitialLimit_ms = d;
    else if (strcmp(settingName, "rxClkBiasLockLimit_ms") == 0)
        settings->rxClkBiasLockLimit_ms = d;
    else if (strcmp(settingName, "rxClkBiasLimitCount") == 0)
        settings->rxClkBiasLimitCount = d;
    else if (strcmp(settingName, "Pk") == 0)
        settings->Pk = d;
    else if (strcmp(settingName, "Ik") == 0)
        settings->Ik = d;
    else if (strcmp(settingName, "preferNonCompositeGPSBias") == 0)
        settings->preferNonCompositeGPSBias = d;
    else if (strcmp(settingName, "preferNonCompositeGalileoBias") == 0)
        settings->preferNonCompositeGalileoBias = d;

    //else if (strcmp(settingName, "") == 0)
    //    settings-> = d;

    // Add new settings above
    //<------------------------------------------------------------>

    else
    {
        systemPrintf("Bad setting: %s - value: %s\r\n", settingName, settingString);
        return false;
    }

    return (true);
}

// The SD library doesn't have a fgets function like SD fat so recreate it here
// Read the current line in the file until we hit a EOL char \r or \n
int getLine(File *openFile, char *lineChars, int lineSize)
{
    int count = 0;
    while (openFile->available())
    {
        byte incoming = openFile->read();
        if (incoming == '\r' || incoming == '\n')
        {
            // Sometimes a line has multiple terminators
            while (openFile->peek() == '\r' || openFile->peek() == '\n')
                openFile->read(); // Dump it to prevent next line read corruption
            break;
        }

        lineChars[count++] = incoming;

        if (count == lineSize - 1)
            break; // Stop before overun of buffer
    }
    lineChars[count] = '\0'; // Terminate string
    return (count);
}

// Check for extra characters in field or find minus sign.
char *skipSpace(char *str)
{
    while (isspace(*str))
        str++;
    return str;
}

// Record large character blob to file
void recordFile(const char *fileID, char *fileContents, uint32_t fileSize)
{
    char fileName[80];
    snprintf(fileName, sizeof(fileName), "/%s_%s.txt", platformFilePrefix, fileID);

    if (LittleFS.exists(fileName))
    {
        LittleFS.remove(fileName);
        systemPrintf("Removing LittleFS: %s\r\n", fileName);
    }

    File fileToWrite = LittleFS.open(fileName, FILE_WRITE);
    if (!fileToWrite)
    {
        systemPrintf("Failed to write to file %s\r\n", fileName);
    }
    else
    {
        fileToWrite.write((uint8_t *)fileContents, fileSize); // Store cert into file
        fileToWrite.close();
        systemPrintf("File recorded to LittleFS: %s\r\n", fileName);
    }
}

// Read file into given char array
void loadFile(const char *fileID, char *fileContents)
{
    char fileName[80];
    snprintf(fileName, sizeof(fileName), "/%s_%s.txt", platformFilePrefix, fileID);

    File fileToRead = LittleFS.open(fileName, FILE_READ);
    if (fileToRead)
    {
        fileToRead.read((uint8_t *)fileContents, fileToRead.size()); // Read contents into pointer
        fileToRead.close();
        systemPrintf("File loaded from LittleFS: %s\r\n", fileName);
    }
    else
    {
        systemPrintf("Failed to read from LittleFS: %s\r\n", fileName);
    }
}
