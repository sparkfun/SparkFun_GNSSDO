// If debug option is on, print available heap
void reportHeapNow(bool alwaysPrint)
{
    if (alwaysPrint || (settings.enableHeapReport == true))
    {
        lastHeapReport = millis();
        systemPrintf("FreeHeap: %d / HeapLowestPoint: %d / LargestBlock: %d\r\n", ESP.getFreeHeap(),
                     xPortGetMinimumEverFreeHeapSize(), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    }
}

// If debug option is on, print available heap
void reportHeap()
{
    if (settings.enableHeapReport == true)
    {
        if (millis() - lastHeapReport > 1000)
        {
            reportHeapNow(false);
        }
    }
}

// Reset settings struct to default initializers
void settingsToDefaults()
{
    static const Settings defaultSettings;
    memcpy(&settings, &defaultSettings, sizeof(defaultSettings));
}

// Periodically print conditions if enabled
void printConditions()
{
    // Periodically print the position etc.
    if (settings.enablePrintConditions && ((millis() - lastPrintConditions) > settings.periodicPrintInterval_ms))
    {
        printCurrentConditions(settings.enablePrintConditions == 2);
        lastPrintConditions = millis();
    }
}

// Print the error message every 15 seconds
void reportFatalError(const char *errorMsg)
{
    displayError(errorMsg);

    while (1)
    {
        systemPrint("HALTED: ");
        systemPrint(errorMsg);
        systemPrintln();
        sleep(15);
    }
}
