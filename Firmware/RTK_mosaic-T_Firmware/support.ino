// Helper functions to support printing to eiter the serial port or bluetooth connection

// If we are printing to all endpoints, BT gets priority
int systemAvailable()
{
    return (Serial.available());
}

// If we are printing to all endpoints, BT gets priority
int systemRead()
{
    return (Serial.read());
}

// Output a buffer of the specified length to the serial port
void systemWrite(const uint8_t *buffer, uint16_t length)
{
    Serial.write(buffer, length);
}

// Ensure all serial output has been transmitted, FIFOs are empty
void systemFlush()
{
    Serial.flush();
}

// Output a byte to the serial port
void systemWrite(uint8_t value)
{
    systemWrite(&value, 1);
}

// Point the string at the selected endpoint
void systemPrint(const char *string)
{
    systemWrite((const uint8_t *)string, strlen(string));
}

// Enable printfs to various endpoints
// https://stackoverflow.com/questions/42131753/wrapper-for-printf
void systemPrintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    va_list args2;
    va_copy(args2, args);
    char buf[vsnprintf(nullptr, 0, format, args) + 1];

    vsnprintf(buf, sizeof buf, format, args2);

    systemPrint(buf);

    va_end(args);
    va_end(args2);
}

// Print a string with a carriage return and linefeed
void systemPrintln(const char *value)
{
    systemPrint(value);
    systemPrintln();
}

// Print an integer value
void systemPrint(int value)
{
    char temp[20];
    snprintf(temp, sizeof(temp), "%d", value);
    systemPrint(temp);
}

// Print an integer value as HEX or decimal
void systemPrint(int value, uint8_t printType)
{
    char temp[20];

    if (printType == HEX)
        snprintf(temp, sizeof(temp), "%08X", value);
    else if (printType == DEC)
        snprintf(temp, sizeof(temp), "%d", value);

    systemPrint(temp);
}

// Pretty print IP addresses
void systemPrint(IPAddress ipaddress)
{
    systemPrint(ipaddress[0], DEC);
    systemPrint(".");
    systemPrint(ipaddress[1], DEC);
    systemPrint(".");
    systemPrint(ipaddress[2], DEC);
    systemPrint(".");
    systemPrint(ipaddress[3], DEC);
}
void systemPrintln(IPAddress ipaddress)
{
    systemPrint(ipaddress);
    systemPrintln();
}

// Print an integer value with a carriage return and line feed
void systemPrintln(int value)
{
    systemPrint(value);
    systemPrintln();
}

// Print an 8-bit value as HEX or decimal
void systemPrint(uint8_t value, uint8_t printType)
{
    char temp[20];

    if (printType == HEX)
        snprintf(temp, sizeof(temp), "%02X", value);
    else if (printType == DEC)
        snprintf(temp, sizeof(temp), "%d", value);

    systemPrint(temp);
}

// Print an 8-bit value as HEX or decimal with a carriage return and linefeed
void systemPrintln(uint8_t value, uint8_t printType)
{
    systemPrint(value, printType);
    systemPrintln();
}

// Print a 16-bit value as HEX or decimal
void systemPrint(uint16_t value, uint8_t printType)
{
    char temp[20];

    if (printType == HEX)
        snprintf(temp, sizeof(temp), "%04X", value);
    else if (printType == DEC)
        snprintf(temp, sizeof(temp), "%d", value);

    systemPrint(temp);
}

// Print a 16-bit value as HEX or decimal with a carriage return and linefeed
void systemPrintln(uint16_t value, uint8_t printType)
{
    systemPrint(value, printType);
    systemPrintln();
}

// Print a floating point value with a specified number of decimal places
void systemPrint(float value, uint8_t decimals)
{
    char temp[20];
    snprintf(temp, sizeof(temp), "%.*f", decimals, value);
    systemPrint(temp);
}

// Print a floating point value with a specified number of decimal places and a
// carriage return and linefeed
void systemPrintln(float value, uint8_t decimals)
{
    systemPrint(value, decimals);
    systemPrintln();
}

// Print a double precision floating point value with a specified number of decimal places
void systemPrint(double value, uint8_t decimals)
{
    char temp[30];
    snprintf(temp, sizeof(temp), "%.*f", decimals, value);
    systemPrint(temp);
}

// Print a double precision floating point value with a specified number of decimal
// places and a carriage return and linefeed
void systemPrintln(double value, uint8_t decimals)
{
    systemPrint(value, decimals);
    systemPrintln();
}

// Print a string
void systemPrint(String myString)
{
    systemPrint(myString.c_str());
}
void systemPrintln(String myString)
{
    systemPrint(myString);
    systemPrintln();
}

// Print a carriage return and linefeed
void systemPrintln()
{
    systemPrint("\r\n");
}

// Option not known
void printUnknown(uint8_t unknownChoice)
{
    systemPrint("Unknown choice: ");
    systemWrite(unknownChoice);
    systemPrintln();
}
void printUnknown(int unknownValue)
{
    systemPrint("Unknown value: ");
    systemPrintln((uint16_t)unknownValue, DEC);
}

// Clear the Serial/Bluetooth RX buffer before we begin scanning for characters
void clearBuffer()
{
    systemFlush();
    delay(20); // Wait for any incoming chars to hit buffer
    while (systemAvailable() > 0)
        systemRead(); // Clear buffer
}

// Gathers raw characters from user until \n or \r is received
// Handles backspace
// Used for raw mixed entry (SSID, pws, etc)
// Used by other menu input methods that use sscanf
// Returns INPUT_RESPONSE_TIMEOUT, INPUT_RESPONSE_OVERFLOW, INPUT_RESPONSE_EMPTY, or INPUT_RESPONSE_VALID
InputResponse getString(char *userString, uint8_t stringSize)
{
    clearBuffer();

    long startTime = millis();
    uint8_t spot = 0;

    while ((millis() - startTime) / 1000 <= menuTimeout)
    {
        delay(1); // Yield to processor

        //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

        // Get the next input character
        while (systemAvailable() > 0)
        {
            byte incoming = systemRead();

            if ((incoming == '\r') || (incoming == '\n'))
            {
                if (settings.echoUserInput)
                    systemPrintln();     // Echo if needed
                userString[spot] = '\0'; // Null terminate

                if (spot == 0)
                    return INPUT_RESPONSE_EMPTY;

                return INPUT_RESPONSE_VALID;
            }
            // Handle backspace
            else if (incoming == '\b')
            {
                if (settings.echoUserInput == true && spot > 0)
                {
                    systemWrite('\b'); // Move back one space
                    systemWrite(' ');  // Put a blank there to erase the letter from the terminal
                    systemWrite('\b'); // Move back again
                    spot--;
                }
            }
            else
            {
                if (settings.echoUserInput)
                    systemWrite(incoming); // Echo if needed

                userString[spot++] = incoming;
                if (spot == stringSize) // Leave room for termination
                    return INPUT_RESPONSE_OVERFLOW;
            }
        }
    }

    return INPUT_RESPONSE_TIMEOUT;
}

// Get a valid IP Address (nnn.nnn.nnn.nnn) using getString
// Returns INPUT_RESPONSE_TIMEOUT, INPUT_RESPONSE_OVERFLOW, INPUT_RESPONSE_EMPTY, INPUT_RESPONSE_INVALID or
// INPUT_RESPONSE_VALID
InputResponse getIPAddress(char *userString, uint8_t stringSize)
{
    InputResponse result = getString(userString, stringSize);
    if (result != INPUT_RESPONSE_VALID)
        return result;
    int dummy[4];
    if (sscanf(userString, "%d.%d.%d.%d", &dummy[0], &dummy[1], &dummy[2], &dummy[3]) !=
        4) // Check that the user has entered nnn.nnn.nnn.nnn
        return INPUT_RESPONSE_INVALID;
    for (int i = 0; i <= 3; i++)
    {
        if ((dummy[i] < 0) || (dummy[i] > 255)) // Check each value is 0-255
            return INPUT_RESPONSE_INVALID;
    }
    return INPUT_RESPONSE_VALID;
}

// Gets a single character or number (0-32) from the user. Negative numbers become the positive equivalent.
// Numbers larger than 32 are allowed but will be confused with characters: ie, 74 = 'J'.
// Returns 255 if timeout
// Returns 0 if no data, only carriage return or newline
byte getCharacterNumber()
{
    char userEntry[50]; // Allow user to enter more than one char. sscanf will remove extra.
    int userByte = 0;

    InputResponse response = getString(userEntry, sizeof(userEntry));
    if (response == INPUT_RESPONSE_VALID)
    {
        int filled = sscanf(userEntry, "%d", &userByte);
        if (filled == 0) // Not a number
            sscanf(userEntry, "%c", (byte *)&userByte);
        else
        {
            if (userByte == 255)
                userByte = 0; // Not allowed
            else if (userByte > 128)
                userByte *= -1; // Drop negative sign
        }
    }
    else if (response == INPUT_RESPONSE_TIMEOUT)
    {
        systemPrintln("\r\nNo user response - Do you have line endings turned on?");
        userByte = 255; // Timeout
    }
    else if (response == INPUT_RESPONSE_EMPTY)
    {
        userByte = 0; // Empty
    }

    return userByte;
}

// Get a long int from user, uses sscanf to obtain 64-bit int
// Returns INPUT_RESPONSE_GETNUMBER_EXIT if user presses 'x' or doesn't enter data
// Returns INPUT_RESPONSE_GETNUMBER_TIMEOUT if input times out
long getNumber()
{
    char userEntry[50]; // Allow user to enter more than one char. sscanf will remove extra.
    long userNumber = 0;

    InputResponse response = getString(userEntry, sizeof(userEntry));
    if (response == INPUT_RESPONSE_VALID)
    {
        if (strcmp(userEntry, "x") == 0 || strcmp(userEntry, "X") == 0)
            userNumber = INPUT_RESPONSE_GETNUMBER_EXIT;
        else
            sscanf(userEntry, "%ld", &userNumber);
    }
    else if (response == INPUT_RESPONSE_TIMEOUT)
    {
        systemPrintln("\r\nNo user response - Do you have line endings turned on?");
        userNumber = INPUT_RESPONSE_GETNUMBER_TIMEOUT; // Timeout
    }
    else if (response == INPUT_RESPONSE_EMPTY)
    {
        userNumber = INPUT_RESPONSE_GETNUMBER_EXIT; // Empty
    }

    return userNumber;
}

// Gets a double (float) from the user
// Returns 0 for timeout and empty response
double getDouble()
{
    char userEntry[50];
    double userFloat = 0.0;

    InputResponse response = getString(userEntry, sizeof(userEntry));
    if (response == INPUT_RESPONSE_VALID)
        sscanf(userEntry, "%lf", &userFloat);
    else if (response == INPUT_RESPONSE_TIMEOUT)
    {
        systemPrintln("No user response - Do you have line endings turned on?");
        userFloat = 0.0;
    }
    else if (response == INPUT_RESPONSE_EMPTY)
    {
        userFloat = 0.0;
    }

    return userFloat;
}

void printElapsedTime(const char *title)
{
    systemPrintf("%s: %ld\r\n", title, millis() - startTime);
}

void printDebug(String thingToPrint)
{
    if (settings.printDebugMessages == true)
    {
        systemPrint(thingToPrint);
    }
}

#define TIMESTAMP_INTERVAL 1000 // Milliseconds

// Print the timestamp
void printTimeStamp()
{
    uint32_t currentMilliseconds;
    static uint32_t previousMilliseconds;

    // Timestamp the messages
    currentMilliseconds = millis();
    if ((currentMilliseconds - previousMilliseconds) >= TIMESTAMP_INTERVAL)
    {
        //         1         2         3
        // 123456789012345678901234567890
        // YYYY-mm-dd HH:MM:SS.xxxrn0
        struct tm timeinfo = rtc.getTimeStruct();
        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
        systemPrintf("%s.%03ld\r\n", timestamp, rtc.getMillis());

        // Select the next time to display the timestamp
        previousMilliseconds = currentMilliseconds;
    }
}

// Parse the RTCM transport data
// Called by processRTCM in Base.ino - defines whether the data is passed to the NTRIP server
bool checkRtcmMessage(uint8_t data)
{
    static uint16_t bytesRemaining;
    static uint16_t length;
    static uint16_t message;
    static bool sendMessage = false;

    //    RTCM Standard 10403.2 - Chapter 4, Transport Layer
    //
    //    |<------------- 3 bytes ------------>|<----- length ----->|<- 3 bytes ->|
    //    |                                    |                    |             |
    //    +----------+--------+----------------+---------+----------+-------------+
    //    | Preamble |  Fill  | Message Length | Message |   Fill   |   CRC-24Q   |
    //    |  8 bits  | 6 bits |    10 bits     |  n-bits | 0-7 bits |   24 bits   |
    //    |   0xd3   | 000000 |   (in bytes)   |         |   zeros  |             |
    //    +----------+--------+----------------+---------+----------+-------------+
    //    |                                                         |
    //    |<------------------------ CRC -------------------------->|
    //

    switch (rtcmParsingState)
    {
    // Read the upper two bits of the length
    case RTCM_TRANSPORT_STATE_READ_LENGTH_1:
        // Verify the length byte - check the 6 MS bits are all zero
        if (!(data & (~3)))
        {
            length = data << 8;
            rtcmParsingState = RTCM_TRANSPORT_STATE_READ_LENGTH_2;
            break;
        }

        // Wait for the preamble byte
        rtcmParsingState = RTCM_TRANSPORT_STATE_WAIT_FOR_PREAMBLE_D3;

    // Fall through
    //      |
    //      |
    //      V

    // Wait for the preamble byte (0xd3)
    case RTCM_TRANSPORT_STATE_WAIT_FOR_PREAMBLE_D3:
        sendMessage = false;
        if (data == 0xd3)
        {
            rtcmParsingState = RTCM_TRANSPORT_STATE_READ_LENGTH_1;
            sendMessage = true;
        }
        break;

    // Read the lower 8 bits of the length
    case RTCM_TRANSPORT_STATE_READ_LENGTH_2:
        length |= data;
        bytesRemaining = length;
        rtcmParsingState = RTCM_TRANSPORT_STATE_READ_MESSAGE_1;
        break;

    // Read the upper 8 bits of the message number
    case RTCM_TRANSPORT_STATE_READ_MESSAGE_1:
        message = data << 4;
        bytesRemaining -= 1;
        rtcmParsingState = RTCM_TRANSPORT_STATE_READ_MESSAGE_2;
        break;

    // Read the lower 4 bits of the message number
    case RTCM_TRANSPORT_STATE_READ_MESSAGE_2:
        message |= data >> 4;
        bytesRemaining -= 1;
        rtcmParsingState = RTCM_TRANSPORT_STATE_READ_DATA;
        break;

    // Read the rest of the message
    case RTCM_TRANSPORT_STATE_READ_DATA:
        bytesRemaining -= 1;
        if (bytesRemaining <= 0)
            rtcmParsingState = RTCM_TRANSPORT_STATE_READ_CRC_1;
        break;

    // Read the upper 8 bits of the CRC
    case RTCM_TRANSPORT_STATE_READ_CRC_1:
        rtcmParsingState = RTCM_TRANSPORT_STATE_READ_CRC_2;
        break;

    // Read the middle 8 bits of the CRC
    case RTCM_TRANSPORT_STATE_READ_CRC_2:
        rtcmParsingState = RTCM_TRANSPORT_STATE_READ_CRC_3;
        break;

    // Read the lower 8 bits of the CRC
    case RTCM_TRANSPORT_STATE_READ_CRC_3:
        rtcmParsingState = RTCM_TRANSPORT_STATE_CHECK_CRC;
        break;
    }

    // Check the CRC. Note: this doesn't actually check the CRC!
    if (rtcmParsingState == RTCM_TRANSPORT_STATE_CHECK_CRC)
    {
        rtcmParsingState = RTCM_TRANSPORT_STATE_WAIT_FOR_PREAMBLE_D3;

        // Display the RTCM message header
        if (settings.debugNtripServerRtcm && (!inMainMenu))
        {
            printTimeStamp();
            systemPrintf("    Tx RTCM %d, %2d bytes\r\n", message, 3 + length + 3);
        }
    }

    // Let the upper layer know if this message should be sent
    return sendMessage;
}

const double WGS84_A = 6378137;           // https://geographiclib.sourceforge.io/html/Constants_8hpp_source.html
const double WGS84_E = 0.081819190842622; // http://docs.ros.org/en/hydro/api/gps_common/html/namespacegps__common.html
                                          // and https://gist.github.com/uhho/63750c4b54c7f90f37f958cc8af0c718

// From: https://stackoverflow.com/questions/19478200/convert-latitude-and-longitude-to-ecef-coordinates-system
void geodeticToEcef(double lat, double lon, double alt, double *x, double *y, double *z)
{
    double clat = cos(lat * DEG_TO_RAD);
    double slat = sin(lat * DEG_TO_RAD);
    double clon = cos(lon * DEG_TO_RAD);
    double slon = sin(lon * DEG_TO_RAD);

    double N = WGS84_A / sqrt(1.0 - WGS84_E * WGS84_E * slat * slat);

    *x = (N + alt) * clat * clon;
    *y = (N + alt) * clat * slon;
    *z = (N * (1.0 - WGS84_E * WGS84_E) + alt) * slat;
}

// From: https://danceswithcode.net/engineeringnotes/geodetic_to_ecef/geodetic_to_ecef.html
void ecefToGeodetic(double x, double y, double z, double *lat, double *lon, double *alt)
{
    double a = 6378137.0;              // WGS-84 semi-major axis
    double e2 = 6.6943799901377997e-3; // WGS-84 first eccentricity squared
    double a1 = 4.2697672707157535e+4; // a1 = a*e2
    double a2 = 1.8230912546075455e+9; // a2 = a1*a1
    double a3 = 1.4291722289812413e+2; // a3 = a1*e2/2
    double a4 = 4.5577281365188637e+9; // a4 = 2.5*a2
    double a5 = 4.2840589930055659e+4; // a5 = a1+a3
    double a6 = 9.9330562000986220e-1; // a6 = 1-e2

    double zp, w2, w, r2, r, s2, c2, s, c, ss;
    double g, rg, rf, u, v, m, f, p;

    zp = abs(z);
    w2 = x * x + y * y;
    w = sqrt(w2);
    r2 = w2 + z * z;
    r = sqrt(r2);
    *lon = atan2(y, x); // Lon (final)

    s2 = z * z / r2;
    c2 = w2 / r2;
    u = a2 / r;
    v = a3 - a4 / r;
    if (c2 > 0.3)
    {
        s = (zp / r) * (1.0 + c2 * (a1 + u + s2 * v) / r);
        *lat = asin(s); // Lat
        ss = s * s;
        c = sqrt(1.0 - ss);
    }
    else
    {
        c = (w / r) * (1.0 - s2 * (a5 - u - c2 * v) / r);
        *lat = acos(c); // Lat
        ss = 1.0 - c * c;
        s = sqrt(ss);
    }

    g = 1.0 - e2 * ss;
    rg = a / sqrt(g);
    rf = a6 * rg;
    u = w - rg * c;
    v = zp - rf * s;
    f = c * u + s * v;
    m = c * v - s * u;
    p = m / (rf / g + f);
    *lat = *lat + p;        // Lat
    *alt = f + m * p / 2.0; // Altitude
    if (z < 0.0)
    {
        *lat *= -1.0; // Lat
    }

    *lat *= RAD_TO_DEG; // Convert to degrees
    *lon *= RAD_TO_DEG;
}

// Convert nibble to ASCII
uint8_t nibbleToAscii(int nibble)
{
    nibble &= 0xf;
    return (nibble > 9) ? nibble + 'a' - 10 : nibble + '0';
}

// Convert nibble to ASCII
int AsciiToNibble(int data)
{
    // Convert the value to lower case
    data |= 0x20;
    if ((data >= 'a') && (data <= 'f'))
        return data - 'a' + 10;
    if ((data >= '0') && (data <= '9'))
        return data - '0';
    return -1;
}

void dumpBuffer(uint8_t *buffer, uint16_t length)
{
    int bytes;
    uint8_t *end;
    int index;
    uint16_t offset;

    end = &buffer[length];
    offset = 0;
    while (buffer < end)
    {
        // Determine the number of bytes to display on the line
        bytes = end - buffer;
        if (bytes > (16 - (offset & 0xf)))
            bytes = 16 - (offset & 0xf);

        // Display the offset
        systemPrintf("0x%08lx: ", offset);

        // Skip leading bytes
        for (index = 0; index < (offset & 0xf); index++)
            systemPrintf("   ");

        // Display the data bytes
        for (index = 0; index < bytes; index++)
            systemPrintf("%02x ", buffer[index]);

        // Separate the data bytes from the ASCII
        for (; index < (16 - (offset & 0xf)); index++)
            systemPrintf("   ");
        systemPrintf(" ");

        // Skip leading bytes
        for (index = 0; index < (offset & 0xf); index++)
            systemPrintf(" ");

        // Display the ASCII values
        for (index = 0; index < bytes; index++)
            systemPrintf("%c", ((buffer[index] < ' ') || (buffer[index] >= 0x7f)) ? '.' : buffer[index]);
        systemPrintf("\r\n");

        // Set the next line of data
        buffer += bytes;
        offset += bytes;
    }
}

// Make size of files human readable
void stringHumanReadableSize(String &returnText, uint64_t bytes)
{
    char suffix[5] = {'\0'};
    char readableSize[50] = {'\0'};
    float cardSize = 0.0;

    if (bytes < 1024)
        strcpy(suffix, "B");
    else if (bytes < (1024 * 1024))
        strcpy(suffix, "KB");
    else if (bytes < (1024 * 1024 * 1024))
        strcpy(suffix, "MB");
    else
        strcpy(suffix, "GB");

    if (bytes < (1024))
        cardSize = bytes; // B
    else if (bytes < (1024 * 1024))
        cardSize = bytes / 1024.0; // KB
    else if (bytes < (1024 * 1024 * 1024))
        cardSize = bytes / 1024.0 / 1024.0; // MB
    else
        cardSize = bytes / 1024.0 / 1024.0 / 1024.0; // GB

    if (strcmp(suffix, "GB") == 0)
        snprintf(readableSize, sizeof(readableSize), "%0.1f %s", cardSize, suffix); // Print decimal portion
    else if (strcmp(suffix, "MB") == 0)
        snprintf(readableSize, sizeof(readableSize), "%0.1f %s", cardSize, suffix); // Print decimal portion
    else if (strcmp(suffix, "KB") == 0)
        snprintf(readableSize, sizeof(readableSize), "%0.1f %s", cardSize, suffix); // Print decimal portion
    else
        snprintf(readableSize, sizeof(readableSize), "%.0f %s", cardSize, suffix); // Don't print decimal portion

    returnText = String(readableSize);
}

// Print the NMEA checksum error
void printNmeaChecksumError(PARSE_STATE *parse)
{
    printTimeStamp();
    systemPrintf("    %s NMEA %s, %2d bytes, bad checksum, expecting 0x%c%c, computed: 0x%02x\r\n",
                 parse->parserName, parse->nmeaMessageName, parse->length, parse->buffer[parse->nmeaLength - 2],
                 parse->buffer[parse->nmeaLength - 1], parse->crc);
}

// Print the RTCM checksum error
void printRtcmChecksumError(PARSE_STATE *parse)
{
    printTimeStamp();
    systemPrintf("    %s RTCM %d, %2d bytes, bad CRC, expecting 0x%02x%02x%02x, computed: 0x%06x\r\n",
                 parse->parserName, parse->message, parse->length, parse->buffer[parse->length - 3],
                 parse->buffer[parse->length - 2], parse->buffer[parse->length - 1], parse->rtcmCrc);
}

// Print the RTCM maximum length
void printRtcmMaxLength(PARSE_STATE *parse)
{
    systemPrintf("RTCM parser error maxLength: %d bytes\r\n", parse->maxLength);
}

// Print the u-blox checksum error
void printUbloxChecksumError(PARSE_STATE *parse)
{
    printTimeStamp();
    systemPrintf("    %s u-blox %d.%d, %2d bytes, bad checksum, expecting 0x%02X%02X, computed: 0x%02X%02X\r\n",
                 parse->parserName, parse->message >> 8, parse->message & 0xff, parse->length,
                 parse->buffer[parse->nmeaLength - 2], parse->buffer[parse->nmeaLength - 1], parse->ck_a,
                 parse->ck_b);
}

// Print the u-blox invalid data error
void printUbloxInvalidData(PARSE_STATE *parse)
{
    dumpBuffer(parse->buffer, parse->length - 1);
    systemPrintf("    %s Invalid UBX data, %d bytes\r\n", parse->parserName, parse->length - 1);
}

void printPartitionTable(void)
{
    systemPrintln("ESP32 Partition table:\n");

    systemPrintln("| Type | Sub |  Offset  |   Size   |       Label      |");
    systemPrintln("| ---- | --- | -------- | -------- | ---------------- |");

    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (pi != NULL)
    {
        do
        {
            const esp_partition_t *p = esp_partition_get(pi);
            systemPrintf("|  %02x  | %02x  | 0x%06X | 0x%06X | %-16s |\r\n", p->type, p->subtype, p->address, p->size,
                         p->label);
        } while ((pi = (esp_partition_next(pi))));
    }
}

// Locate the partition for the little file system
bool findSpiffsPartition(void)
{
    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (pi != NULL)
    {
        do
        {
            const esp_partition_t *p = esp_partition_get(pi);
            if (strcmp(p->label, "spiffs") == 0)
                return true;
        } while ((pi = (esp_partition_next(pi))));
    }
    return false;
}

// Verify table sizes match enum definitions
void verifyTables()
{
    // Verify the product name table
    if (productDisplayNamesEntries != (RTK_UNKNOWN + 1))
        reportFatalError("Fix productDisplayNames to match ProductVariant");
    if (platformFilePrefixTableEntries != (RTK_UNKNOWN + 1))
        reportFatalError("Fix platformFilePrefixTable to match ProductVariant");
    if (platformPrefixTableEntries != (RTK_UNKNOWN + 1))
        reportFatalError("Fix platformPrefixTable to match ProductVariant");
    if (mosaicTimeSystemIndexTableEntries != mosaicTimeSystemTableEntries)
        reportFatalError("Fix mosaicTimeSystemIndexTable to match mosaicTimeSystemTable");

    tasksValidateTables();
}

const uint32_t SFE_DAYS_FROM_1970_TO_2020 = 18262; // Jan 1st 2020 Epoch = 1577836800 seconds
const uint16_t SFE_DAYS_SINCE_2020[80] =
    {
        0, 366, 731, 1096, 1461, 1827, 2192, 2557, 2922, 3288,
        3653, 4018, 4383, 4749, 5114, 5479, 5844, 6210, 6575, 6940,
        7305, 7671, 8036, 8401, 8766, 9132, 9497, 9862, 10227, 10593,
        10958, 11323, 11688, 12054, 12419, 12784, 13149, 13515, 13880, 14245,
        14610, 14976, 15341, 15706, 16071, 16437, 16802, 17167, 17532, 17898,
        18263, 18628, 18993, 19359, 19724, 20089, 20454, 20820, 21185, 21550,
        21915, 22281, 22646, 23011, 23376, 23742, 24107, 24472, 24837, 25203,
        25568, 25933, 26298, 26664, 27029, 27394, 27759, 28125, 28490, 28855};
const uint16_t SFE_DAYS_SINCE_MONTH[2][12] =
    {
        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}, // Leap Year (Year % 4 == 0)
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}  // Normal Year
};

const uint32_t SFE_JAN_1ST_2020_WEEK = 2086; // GPS Week Number for Jan 1st 2020
const uint32_t SFE_EPOCH_WEEK_2086 = 1577836800 - 259200; // Epoch for the start of GPS week 2086
const uint32_t SFE_SECS_PER_WEEK = 60 * 60 * 24 * 7; // Seconds per week

// Helper method to convert GNSS time and date into Unix Epoch
void convertGnssTimeToEpoch(uint32_t *epochSecs, uint32_t *epochMillis)
{
    uint32_t t = SFE_DAYS_FROM_1970_TO_2020;             // Jan 1st 2020 as days from Jan 1st 1970
    t += (uint32_t)SFE_DAYS_SINCE_2020[gnssYear - 2020]; // Add on the number of days since 2020
    t += (uint32_t)
        SFE_DAYS_SINCE_MONTH[gnssYear % 4 == 0 ? 0 : 1][gnssMonth - 1]; // Add on the number of days since Jan 1st
    t += (uint32_t)gnssDay - 1; // Add on the number of days since the 1st of the month
    t *= 24;                    // Convert to hours
    t += (uint32_t)gnssHour;    // Add on the hour
    t *= 60;                    // Convert to minutes
    t += (uint32_t)gnssMinute;  // Add on the minute
    t *= 60;                    // Convert to seconds
    t += (uint32_t)gnssSecond;  // Add on the second

    *epochSecs = t;
    *epochMillis = gnssTOW_ms % 1000;
}

// Format the firmware version
void formatFirmwareVersion(uint8_t major, uint8_t minor, char *buffer, int bufferLength, bool includeDate)
{
  char prefix;

  // Construct the full or release candidate version number
  prefix = 'd';
  //    if (enableRCFirmware && (bufferLength >= 21))
  //        // 123456789012345678901
  //        // pxxx.yyy-dd-mmm-yyyy0
  //        snprintf(buffer, bufferLength, "%c%d.%d-%s", prefix, major, minor, __DATE__);

  // Construct a truncated version number
  if (bufferLength >= 9)
    // 123456789
    // pxxx.yyy0
    snprintf(buffer, bufferLength, "%c%d.%d", prefix, major, minor);

  // The buffer is too small for the version number
  else
  {
    Serial.printf("ERROR: Buffer too small for version number!\r\n");
    if (bufferLength > 0)
      *buffer = 0;
  }
}

// Get the current firmware version
void getFirmwareVersion(char *buffer, int bufferLength, bool includeDate)
{
  formatFirmwareVersion(FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR, buffer, bufferLength, includeDate);
}
