//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void flushRX(unsigned long timeout = 0); // Header
void flushRX(unsigned long timeout)
{
  if (timeout > 0)
  {
    unsigned long startTime = millis();
    while (millis() < (startTime + timeout))
      if (serialGNSSConfig.available())
      {
        uint8_t chr = serialGNSSConfig.read();
        if (settings.enablePrintGNSSMessages)
          systemWrite(chr);
      }
  }
  else
  {
    while (serialGNSSConfig.available())
    {
      uint8_t chr = serialGNSSConfig.read();
      if (settings.enablePrintGNSSMessages)
        systemWrite(chr);
    }
  }
}

bool sendWithResponse(const char *message, const char *reply, unsigned long timeout = 1000, unsigned long wait = 100); // Header
bool sendWithResponse(const char *message, const char *reply, unsigned long timeout, unsigned long wait)
{
  if (strlen(reply) == 0) // Reply can't be zero-length
    return false;

  if (strlen(message) > 0)
  {
    serialGNSSConfig.write(message, strlen(message)); // Send the message
    if (settings.enablePrintGNSSMessages)
      systemWrite((const uint8_t *)message, strlen(message));
  }

  unsigned long startTime = millis();
  size_t replySeen = 0;
  bool keepGoing = true;

  while ((keepGoing) && (replySeen < strlen(reply))) // While not timed out and reply not seen
  {
    if (serialGNSSConfig.available()) // If a char is available
    {
      uint8_t chr = serialGNSSConfig.read(); // Read it
      if (chr == *(reply + replySeen)) // Is it a char from reply?
        replySeen++;
      else
        replySeen = 0; // Reset replySeen on an unexpected char

      if (settings.enablePrintGNSSMessages)
        systemWrite(chr);
    }

    // If the reply has started to arrive at the timeout, allow extra time
    if (millis() > (startTime + timeout)) // Have we timed out?
      if (replySeen == 0)                 // If replySeen is zero, don't keepGoing
        keepGoing = false;

    if (millis() > (startTime + timeout + wait)) // Have we really timed out?
      keepGoing = false;                         // Don't keepGoing
  }

  if (replySeen == strlen(reply)) // If the reply was seen
  {
    flushRX(wait); // wait and flush
    return true;
  }

  return false;
}

bool sendWithResponse(String message, const char *reply, unsigned long timeout = 1000, unsigned long wait = 100); // Header
bool sendWithResponse(String message, const char *reply, unsigned long timeout, unsigned long wait)
{
  return sendWithResponse(message.c_str(), reply, timeout, wait);
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Begin GNSS
// Ensure GNSS is communicating on COM4. Request IPStatus
void beginGNSS()
{
    systemPrintln("Begin GNSS - requesting IPStatus");

    int retries = 20; // The mosaic takes a few seconds to wake up after power on

    while (!sendWithResponse("esoc, COM1, IPStatus\n\r", "SBFOnce") && (retries > 0))
    {
        systemPrintln("No response from mosaic. Retrying - with escape sequence...");
        sendWithResponse("SSSSSSSSSSSSSSSSSSSS\n\r", "COM4>"); // Send escape sequence
        retries--;
    }

    if (retries == 0)
    {
        systemPrintln("GNSS FAIL (SBFOnce)");
        return;
    }

    systemPrintln("GNSS online. IPStatus requested");
    online.gnss = true;
}

// Kickstart the GNSS - e.g. to resync on bad bias
bool gnssSoftReset()
{
    if (!online.gnss)
        return false;

    int retries = 3; // GNSS is already begun. We shouldn't need to retry.

    while (!sendWithResponse("erst,Soft,PVTData\n\r", "ResetReceiver") && (retries > 0)) // Soft reset - erase PVT data
    {
        systemPrintln("No response from mosaic. Retrying - with escape sequence...");
        sendWithResponse("SSSSSSSSSSSSSSSSSSSS\n\r", "COM4>"); // Send escape sequence
        retries--;
    }

    return (retries > 0);
}

// Revert to boot configuration
bool gnssHardReset()
{
    if (!online.gnss)
        return false;

    int retries = 3; // GNSS is already begun. We shouldn't need to retry.

    while (!sendWithResponse("erst,Hard\n\r", "ResetReceiver") && (retries > 0)) // Hard reset - revert to boot configuration
    {
        systemPrintln("No response from mosaic. Retrying - with escape sequence...");
        sendWithResponse("SSSSSSSSSSSSSSSSSSSS\n\r", "COM4>"); // Send escape sequence
        retries--;
    }

    return (retries > 0);
}

// Initialize GNSS
// Disable PPS. Set clock sync threshold. Set output messages. Copy config file.
// This only needs to be done once.
bool initializeGNSS()
{
    if (!online.gnss)
        return false;

    systemPrintln("Initialize GNSS");

    flushRX(250);

    int retries = 3; // GNSS is already begun. We shouldn't need to retry.

    while (!sendWithResponse("eccf, RxDefault, Current\n\r", "CopyConfigFile") && (retries > 0)) // Restore defaults
    {
        systemPrintln("No response from mosaic. Retrying - with escape sequence...");
        sendWithResponse("SSSSSSSSSSSSSSSSSSSS\n\r", "COM4>"); // Send escape sequence
        retries--;
    }

    if (retries == 0)
    {
        systemPrintln("GNSS FAIL (CopyConfigFile RxDefault)");
        return false;
    }

    if (!sendWithResponse("spps, off\n\r", "PPSParameters"))
    {
        systemPrintln("GNSS FAIL (PPSParameters)");
        return false;
    }

    if (!sendWithResponse("scst, usec500, on\n\r", "ClockSyncThreshold"))
    {
        systemPrintln("GNSS FAIL (ClockSyncThreshold)");
        return false;
    }

    if (!sendWithResponse("ssgp, Group1, PVTGeodetic+ReceiverTime\n\r", "SBFGroups"))
    {
        systemPrintln("GNSS FAIL (SBFGroups)");
        return false;
    }

    if (!sendWithResponse("sso, Stream1, COM1, Group1, sec1\n\r", "SBFOutput"))
    {
        systemPrintln("GNSS FAIL (SBFOutput Stream1)");
        return false;
    }

    if (!sendWithResponse("sso, Stream2, COM1, IPStatus+FugroTimeOffset, OnChange\n\r", "SBFOutput"))
    {
        systemPrintln("GNSS FAIL (SBFOutput Stream2)");
        return false;
    }

    if (!sendWithResponse("eccf, Current, Boot\n\r", "CopyConfigFile"))
    {
        systemPrintln("GNSS FAIL (CopyConfigFile)");
        return false;
    }

    systemPrintln("GNSS initialized");
    return true;
}

bool configureGNSSPPS()
{
    if (!online.gnss)
        return false;

    systemPrintln("Configuring GNSS PPS");

    String ppsParams = String("setPPSParameters, ");
    ppsParams += String(mosaicPPSParametersInterval[settings.ppsInterval]) + String(", ");
    ppsParams += String(mosaicPPSParametersPolarity[settings.ppsPolarity]) + String(", ");
    ppsParams += String(settings.ppsDelay_ns) + String(", ");
    ppsParams += String(mosaicPPSParametersTimeScale[settings.ppsTimeScale]) + String(", ");
    ppsParams += String(settings.ppsMaxSyncAge_s) + String(", ");
    ppsParams += String(settings.ppsPulseWidth_ms);
    ppsParams += String("\n\r");

    int retries = 3;

    while (!sendWithResponse(ppsParams, "PPSParameters") && (retries > 0)) // Disable PPS initially
    {
        systemPrintln("No response from mosaic. Retrying - with escape sequence...");
        sendWithResponse("SSSSSSSSSSSSSSSSSSSS\n\r", "COM4>"); // Send escape sequence
        retries--;
    }

    if (retries == 0)
    {
        systemPrintln("GNSS FAIL (PPSParameters)");
        return false;
    }

    systemPrintln("GNSS PPS configured");
    return true;
}

