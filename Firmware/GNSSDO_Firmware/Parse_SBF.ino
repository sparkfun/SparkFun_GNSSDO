/*------------------------------------------------------------------------------
Parse_SBF.ino

  Septentrio SBF message parsing support routines
------------------------------------------------------------------------------*/

//
//    SBF Message
//
//    |<-- Preamble --->|
//    |                 |
//    +--------+--------+---------+---------+---------+---------+
//    |  SYNC  |  SYNC  |   CRC   |   ID    | Length  | Payload |
//    | 8 bits | 8 bits | 16 bits | 2 bytes | 2 bytes | n bytes |
//    |  0x24  |  0x40  |         |         |         |         |
//    +--------+--------+---------+---------+---------+---------+
//                                |                             |
//                                |<-------- Checksum --------->|
//
//  The generator polynomial for the CRC is the so-called CRC-CCITT
//  polynomial: x16 +x12 +x5 +x0. The CRC is computed in the forward
//  direction using a seed of 0, no reverse and no final XOR.

static const uint16_t ccitt_crc_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

uint16_t ccitt_crc_update(uint16_t crc, const uint8_t data)
{
    uint8_t tbl_idx = ((crc >> 8) ^ data) & 0xff;
    
    crc = (ccitt_crc_table[tbl_idx]) ^ (crc << 8);

    return crc;
}

// Check for the NMEA or SBF preamble
uint8_t nmeaSbfPreamble1(PARSE_STATE *parse, uint8_t data)
{
    if (data == '$')
    {
        parse->crc = 0; // Needed for NMEA
        parse->nmeaMessageNameLength = 0; // Needed for NMEA
        parse->state = nmeaSbfPreamble2;
        return SENTENCE_TYPE_NMEA_SBF;
    }
    return SENTENCE_TYPE_NONE;
}

// Check for the SBF preamble Byte 2
uint8_t nmeaSbfPreamble2(PARSE_STATE *parse, uint8_t data)
{
    if (data == '@') // SBF
    {
        parse->state = sbfCRC1;
        return SENTENCE_TYPE_SBF;
    }
    else if ((data >= 'A') && (data <= 'Z')) // NMEA
    {
        parse->crc ^= data;

        // Save the message name
        parse->nmeaMessageName[parse->nmeaMessageNameLength++] = data;

        parse->state = nmeaFindFirstComma;
        return SENTENCE_TYPE_NMEA;
    }

    // Not SBF or NMEA. Must be invalid
    parse->length = 0;
    parse->buffer[parse->length++] = data;
    return gpsMessageParserFirstByte(parse, data);
}

/*
// Check for the preamble
uint8_t sbfPreamble(PARSE_STATE *parse, uint8_t data)
{
    if (data == 0x24)
    {
        parse->state = sbfSync2;
        return SENTENCE_TYPE_SBF;
    }
    return SENTENCE_TYPE_NONE;
}

// Read the second sync byte
uint8_t sbfSync2(PARSE_STATE *parse, uint8_t data)
{
    // Verify the sync 2 byte
    if (data != 0x40)
    {
        // Display the invalid data
        if (settings.enablePrintBadMessages && (!inMainMenu))
            printSbfInvalidData(parse);

        // Invalid sync 2 byte, place this byte at the beginning of the buffer
        parse->length = 0;
        parse->buffer[parse->length++] = data;

        // Start searching for a preamble byte
        return gpsMessageParserFirstByte(parse, data);
    }

    parse->state = sbfCRC1;
    return SENTENCE_TYPE_SBF;
}
*/

// Read the first CRC byte (LSB)
uint8_t sbfCRC1(PARSE_STATE *parse, uint8_t data)
{
    // Store the expected checksum
    parse->sbfCrcExpected = data;

    parse->state = sbfCRC2;
    return SENTENCE_TYPE_SBF;
}

// Read the second CRC byte (MSB)
uint8_t sbfCRC2(PARSE_STATE *parse, uint8_t data)
{
    // Store the expected checksum
    parse->sbfCrcExpected |= ((uint16_t)data) << 8;

    // Zero the computed checksum
    parse->sbfCrcComputed = 0;

    parse->state = sbfID1;
    return SENTENCE_TYPE_SBF;
}

// Read the first ID byte (LSB)
uint8_t sbfID1(PARSE_STATE *parse, uint8_t data)
{
    // Calculate the checksum
    parse->sbfCrcComputed = ccitt_crc_update(parse->sbfCrcComputed, data);

    // Store the ID
    parse->message = data;

    parse->state = sbfID2;
    return SENTENCE_TYPE_SBF;
}

// Read the second ID byte (MSB)
uint8_t sbfID2(PARSE_STATE *parse, uint8_t data)
{
    // Calculate the checksum
    parse->sbfCrcComputed = ccitt_crc_update(parse->sbfCrcComputed, data);

    // Store the ID
    parse->message |= ((uint16_t)data) << 8;

    parse->state = sbfLength1;
    return SENTENCE_TYPE_SBF;
}

// Read the first length byte
uint8_t sbfLength1(PARSE_STATE *parse, uint8_t data)
{
    // Calculate the checksum
    parse->sbfCrcComputed = ccitt_crc_update(parse->sbfCrcComputed, data);

    // Save the first length byte (LSB)
    parse->bytesRemaining = data;

    parse->state = sbfLength2;
    return SENTENCE_TYPE_SBF;
}

// Read the second length byte
uint8_t sbfLength2(PARSE_STATE *parse, uint8_t data)
{
    // Calculate the checksum
    parse->sbfCrcComputed = ccitt_crc_update(parse->sbfCrcComputed, data);

    // Save the second length byte (MSB)
    parse->bytesRemaining |= ((uint16_t)data) << 8;

    parse->bytesRemaining -= 8; // Subtract the header bytes

    parse->state = sbfPayload;
    return SENTENCE_TYPE_SBF;
}

// Read the payload
uint8_t sbfPayload(PARSE_STATE *parse, uint8_t data)
{
    // Calculate the checksum
    parse->sbfCrcComputed = ccitt_crc_update(parse->sbfCrcComputed, data);
    
    // Decrement bytesRemaining
    parse->bytesRemaining--;
    
    // If we are expecting more data
    if (parse->bytesRemaining)
    {
        return SENTENCE_TYPE_SBF;
    }

    // Block is complete. Check the CRC
    bool badChecksum;

    // Validate the checksum
    badChecksum = (parse->sbfCrcComputed != parse->sbfCrcExpected);

    if (settings.enablePrintBadMessages && badChecksum && (!inMainMenu))
        printSbfChecksumError(parse);

    // Process this message if checksum is valid
    if (badChecksum == false)
        parse->eomCallback(parse, SENTENCE_TYPE_SBF);
    else
        failedParserMessages_SBF++;

    // Search for the next preamble byte
    parse->length = 0;
    parse->state = gpsMessageParserFirstByte;
    return SENTENCE_TYPE_NONE;
}
