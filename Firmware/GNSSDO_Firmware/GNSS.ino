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
    if (!inMainMenu)
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
        systemPrintln("GNSS FAIL (SBFOnce). Attempting soft reset...");

        // Module could be stuck in "Ready for SUF Download ...". Send a soft reset to unstick it
        sendWithResponse("erst,soft,none\n\r", "ResetReceiver");

        retries = 20;

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
    }

    if (!inMainMenu)
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
// Disable PPS. Set clock sync threshold. Set output messages. Enable Ethernet. Copy config file.
// This only needs to be done once.
bool initializeGNSS()
{
    if (!online.gnss)
        return false;

    if (!inMainMenu)
        systemPrintln("Initialize GNSS");

    flushRX(250);

    int retries = 3; // GNSS is already begun. We shouldn't need to retry.

    // Restore default configuration
    // Note: the IP settings set by the setIPSettings and setIPPortSettings commands keep their value
    while (!sendWithResponse("eccf, RxDefault, Current\n\r", "CopyConfigFile") && (retries > 0))
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

    // Configure COM3 for daisy chain to IPS1
    if (!sendWithResponse("sdio, COM3, DC1, DC2\n\r", "DataInOut"))
    {
        systemPrintln("GNSS FAIL (DataInOut COM3)");
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

    if (!sendWithResponse("seth, on\n\r", "EthernetMode"))
    {
        systemPrintln("GNSS FAIL (EthernetMode)");
        return false;
    }

    // Copy current configuration into boot
    if (!sendWithResponse("eccf, Current, Boot\n\r", "CopyConfigFile"))
    {
        systemPrintln("GNSS FAIL (CopyConfigFile)");
        return false;
    }

    if (!inMainMenu)
        systemPrintln("GNSS initialized");
    return true;
}

bool configureGNSSPPS()
{
    if (!online.gnss)
        return false;

    if (!inMainMenu)
        systemPrintln("Configuring GNSS PPS");

    String ppsParams = String("spps, ");
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

    if (!inMainMenu)
        systemPrintln("GNSS PPS configured");
    return true;
}

bool configureGNSSTCPServer()
{
    if (!online.gnss)
        return false;

    if (!inMainMenu)
        systemPrintln("Configuring GNSS TCP Server");

    String tcpParams = String("siss, IPS1, ");

    if (settings.enableTCPServer)
    {      
      tcpParams += String(settings.tcpServerPort) + String(", TCP2Way\n\r");
    }
    else
    {
      tcpParams += String("0\n\r");
    }

    int retries = 3;

    while (!sendWithResponse(tcpParams, "IPServerSettings") && (retries > 0))
    {
        systemPrintln("No response from mosaic. Retrying - with escape sequence...");
        sendWithResponse("SSSSSSSSSSSSSSSSSSSS\n\r", "COM4>"); // Send escape sequence
        retries--;
    }

    if (retries == 0)
    {
        systemPrintln("GNSS FAIL (IPServerSettings)");
        return false;
    }

    // Configure IPS1
    if (settings.enableTCPServer)
    {      
      if (!sendWithResponse("sdio, IPS1, DC2, DC1\n\r", "DataInOut"))
      {
          systemPrintln("GNSS FAIL (DataInOut IPS1)");
          return false;
      }
    }
    else
    {
      if (!sendWithResponse("sdio, IPS1, none, none\n\r", "DataInOut"))
      {
          systemPrintln("GNSS FAIL (DataInOut IPS1)");
          return false;
      }
    }

    if (!inMainMenu)
        systemPrintln("GNSS TCP Server configured");
    return true;
}