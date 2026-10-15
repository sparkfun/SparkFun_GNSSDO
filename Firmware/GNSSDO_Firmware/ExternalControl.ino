// Based on a fork by @bobvan
// Thank you Bob
//
// External oscillator control
// ============================
//
// This adds a small, line-based command channel to the ESP32 console so an
// external host can discipline the oscillator directly, instead of the built-in
// GNSSDO loop.
//
// The oscillator status report can be read in any mode, but the control word can
// only be written when the firmware is in STATE_OPEN_LOOP.
// Ensure settings.tcxoClosedLoop is set to false for successful writes.
//
// The channel is the ordinary console, so it works over USB *and* over the
// mosaic-T TCP backdoor (Operation menu -> "TCP Server (IPS1)"): connect a TCP
// socket to the mosaic-T's IP on the configured port and you are talking to
// this parser.
//
// A command is a single line beginning with '$'. updateSerial() (menuMain.ino)
// routes '$' lines here; every other keystroke still opens the interactive menu.
//
// Commands are formatted using NMEA 0183 formatting, with two character checksums:
// $<letter>[,<params>]*<checksum>CRLF
//
//   $W,<word>       Write the oscillator frequency control word (absolute).
//   $R              Report status (see below)
//
// Replies are one line each and echo the command letter:
//   $W,OK,<word>                    control word written; <word> read back
//   $R,<word>,<bias_s>,<mode>       status report: read control word, GNSS bias, firmware mode
//   $<letter>,ERR[,<reason>]        command rejected
//
// With checksums, this becomes:
//
//   Command:  $R*52\r\n
//   Reply:    $R,123456,1.123456e-9,OPEN_LOOP*47\r\n
//
//   Command:  $W,123456*7C\r\n
//   Reply:    $W,OK,123456*54\r\n
//
// See (e.g.): https://www.meme.au/nmea-checksum.html 
//
// The control word is oscillator-specific:
//   STP3593LF (GNSSDO+ / SXT-D OCXO): 20-bit unsigned, LSB = 8e-13 (0.8 ppt)
//   SiT5811   (alternate OCXO):       39-bit signed
//   SiT5358   (GNSSDO / SXT DCTCXO):  26-bit signed
// getFrequencyControlWord() (see $R) returns the current word so the host can
// seed its own servo before taking over.

// Read the remainder of a command line (the leading '$' is already consumed)
// into buf. Terminates on LF, on buffer-full, or on a short idle timeout so
// a partial line can never wedge the console. Returns the number of characters
// captured.
static size_t readExternalControlLine(char *buf, size_t bufLen)
{
    size_t spot = 0;
    unsigned long lastChar_ms = millis();

    while ((millis() - lastChar_ms) < 250) // Ample for a line already in flight
    {
        while (systemAvailable() > 0)
        {
            int c = systemRead();
            lastChar_ms = millis();

            if (spot < (bufLen - 1))
                buf[spot++] = (char)c;
            // If the line overflows we keep draining until LF but stop storing

            if (c == '\n')
            {
                buf[spot] = '\0';
                return spot;
            }
        }
    }

    buf[spot] = '\0';
    return spot;
}

// Parse a signed integer from s, skipping leading separators (comma).
// Sets *ok false if no digits were found.
static long long parseExternalControlInt(const char *s, bool *ok)
{
    while (*s == ',') // Skip the comma
        s++;

    char *end = nullptr;
    long long value = strtoll(s, &end, 10);
    *ok = (end != s);
    return value;
}

// Handle one '$'-prefixed command line. Called from updateSerial() once the
// leading '$' has been read.
void parseExternalControlCommand()
{
    char line[48];
    size_t numChars = readExternalControlLine(line, sizeof(line));

    // First character is the command letter
    char *p = line;
    char cmd = *p;

    // A valid command must have at least 6 chars: R*52\r\n
    if (numChars < 6) 
    {
        systemPrintfNMEA("%c,ERR,TOO_SHORT,%d", cmd, numChars);
        return;
    }

    if (line[numChars - 5] != '*')
    {
        systemPrintfNMEA("%c,ERR,NO_ASTERIX", cmd);
        return;
    }

    if (line[numChars - 2] != '\r')
    {
        systemPrintfNMEA("%c,ERR,NO_CR", cmd);
        return;
    }

    if (line[numChars - 1] != '\n')
    {
        systemPrintfNMEA("%c,ERR,NO_LF", cmd);
        return;
    }

    // Validate checksum
    uint8_t csum = 0;
    // Ex-or all characters into csum until we hit the asterix
    while (*p != '*')
        csum ^= *p++;

    char expectedCsum[3];
    sprintf(expectedCsum, "%02X", csum);

    if ((line[numChars - 4] != expectedCsum[0])
        || (line[numChars - 3] != expectedCsum[1]))
    {
        systemPrintfNMEA("%c,ERR,CSUM_FAIL,%c%c,%c%c", cmd,
            line[numChars - 4], line[numChars - 3],
            expectedCsum[0], expectedCsum[1]);
        return;
    }

    switch (cmd)
    {
    // ---- Write control word (the discipline command) ----------------------
    case 'W': {
        if (!online.tcxo)
        {
            systemPrintfNMEA("W,ERR,NO_OSC");
            break;
        }

        if (systemState != STATE_OPEN_LOOP)
        {
            systemPrintfNMEA("W,ERR,NOT_OPEN_LOOP");
            break;
        }

        bool ok = false;
        p = line; // Re-point at the cmd
        p++; // Skip over the cmd
        // parseExternalControlInt will skip the comma
        long long word = parseExternalControlInt(p, &ok);
        if (!ok)
        {
            systemPrintfNMEA("W,ERR,NO_VALUE");
            break;
        }

        myTCXO->setFrequencyControlWord((int64_t)word);

        systemPrintfNMEA("W,OK,%lld", (long long)getFrequencyControlWord());
        break;
    }

    // ---- Report status ----------------------------------------------------
    case 'R': {
        char stateName[30];
        systemPrintfNMEA("R,%lld,%.6e,%s",
                     (long long)getFrequencyControlWord(),
                     tcxoClockBias_ms / 1000.0, // clock bias in seconds
                     getState(systemState, stateName));
        break;
    }

    default:
        systemPrintfNMEA("%c,ERR,UNKNOWN", cmd);
        break;
    }
}