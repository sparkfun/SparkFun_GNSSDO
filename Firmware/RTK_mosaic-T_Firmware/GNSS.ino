//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void flushRX(unsigned long timeout = 0); // Header
void flushRX(unsigned long timeout)
{
  if (timeout > 0)
  {
    unsigned long startTime = millis();
    while (millis() < (startTime + timeout))
      if (serialGNSSConfig.available())
        serialGNSSConfig.read();
  }
  else
  {
    while (serialGNSSConfig.available())
      serialGNSSConfig.read();
  }
}

bool sendWithResponse(const char *message, const char *reply, unsigned long timeout = 1000, unsigned long wait = 100); // Header
bool sendWithResponse(const char *message, const char *reply, unsigned long timeout, unsigned long wait)
{
  if (strlen(reply) == 0) // Reply can't be zero-length
    return false;

  if (strlen(message) > 0)
    serialGNSSConfig.write(message, strlen(message)); // Send the message

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

// Connect to GNSS
void beginGNSS()
{
    if ((pin_serial2RX >= 0) && (pin_serial2TX >= 0))
    {
        systemPrint("Begin GNSS - ");

        serialGNSSConfig.begin(settings.dataPortBaud, SERIAL_8N1, pin_serial2RX, pin_serial2TX);

        flushRX(250);

        int retries = 3;

        while (!sendWithResponse("setPPSParameters, off\n\r", "PPSParameters") && (retries > 0)) // Disable PPS initially
        {
            Serial.println("No response from mosaic. Retrying - with escape sequence...");
            sendWithResponse("SSSSSSSSSSSSSSSSSSSS\n\r", "COM4>"); // Send escape sequence
            retries--;
        }

        if (retries == 0)
        {
            systemPrintln("FAIL");
            return;
        }

        if (!sendWithResponse("setClockSyncThreshold, usec500, on\n\r", "ClockSyncThreshold"))
        {
            systemPrintln("FAIL");
            return;
        }

        if (!sendWithResponse("setSBFGroups, Group1, PVTGeodetic+ReceiverTime\n\r", "SBFGroups"))
        {
            systemPrintln("FAIL");
            return;
        }

        if (!sendWithResponse("setSBFOutput, Stream1, COM1, Group1, sec1\n\r", "SBFOutput"))
        {
            systemPrintln("FAIL");
            return;
        }

        if (!sendWithResponse("setSBFOutput, Stream2, COM1, IPStatus, OnChange\n\r", "SBFOutput"))
        {
            systemPrintln("FAIL");
            return;
        }

        if (!sendWithResponse("exeCopyConfigFile, Current, Boot\n\r", "CopyConfigFile"))
        {
            systemPrintln("FAIL");
            return;
        }

        if (!sendWithResponse("exeSBFOnce, COM1, IPStatus\n\r", "SBFOnce"))
        {
            systemPrintln("FAIL");
            return;
        }

        systemPrintln("Success");
        online.gnss = true;
    }
}

void configureGNSSPPS()
{
    if (online.gnss == true)
    {
        systemPrintln("Configuring mosaic PPS - ");

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
            Serial.println("No response from mosaic. Retrying - with escape sequence...");
            sendWithResponse("SSSSSSSSSSSSSSSSSSSS\n\r", "COM4>"); // Send escape sequence
            retries--;
        }

        if (retries == 0)
        {
            systemPrintln("FAIL");
            return;
        }

        systemPrintln("Success");
    }
}

