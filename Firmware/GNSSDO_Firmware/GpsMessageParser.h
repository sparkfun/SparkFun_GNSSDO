/*------------------------------------------------------------------------------
GpsMessageParser.h

  Constant and routine declarations for the GPS message parser.
------------------------------------------------------------------------------*/

#ifndef __GPS_MESSAGE_PARSER_H__
#define __GPS_MESSAGE_PARSER_H__

#include <stdint.h>

#include "crc24q.h" // 24-bit CRC-24Q cyclic redundancy checksum for RTCM parsing

//----------------------------------------
// Constants
//----------------------------------------

#define PARSE_BUFFER_LENGTH 3000 // Some USB RAWX messages can be > 2k

enum
{
    SENTENCE_TYPE_NONE = 0,
    SENTENCE_TYPE_NMEA_SBF,
    SENTENCE_TYPE_NMEA,
    SENTENCE_TYPE_RTCM,
    SENTENCE_TYPE_SBF,
    // Add new sentence types above this line
};

//----------------------------------------
// Types
//----------------------------------------

typedef struct _PARSE_STATE *P_PARSE_STATE;

// Parse routine
typedef uint8_t (*PARSE_ROUTINE)(P_PARSE_STATE parse, // Parser state
                                 uint8_t data);       // Incoming data byte

// End of message callback routine
typedef void (*PARSE_EOM_CALLBACK)(P_PARSE_STATE parse, // Parser state
                             uint8_t type);       // Message type

typedef struct _PARSE_STATE
{
    PARSE_ROUTINE state;                 // Parser state routine
    PARSE_EOM_CALLBACK eomCallback;      // End of message callback routine
    const char *parserName;              // Name of parser
    uint32_t crc;                        // RTCM computed CRC
    uint32_t rtcmCrc;                    // Computed CRC value for the RTCM message
    uint32_t invalidRtcmCrcs;            // Number of bad RTCM CRCs detected
    uint16_t bytesRemaining;             // Bytes remaining in RTCM CRC calculation
    uint16_t length;                     // Message length including line termination
    uint16_t maxLength;                  // Maximum message length including line termination
    uint16_t message;                    // RTCM message number. SBF ID
    uint16_t nmeaLength;                 // Length of the NMEA message without line termination
    uint8_t buffer[PARSE_BUFFER_LENGTH]; // Buffer containing the message
    uint8_t nmeaMessageName[16];         // Message name
    uint8_t nmeaMessageNameLength;       // Length of the message name
    uint16_t sbfCrcExpected;             // Expected SBF CRC - from the block header
    uint16_t sbfCrcComputed;             // SBF CRC - calculated from the ID to the end of the block
} PARSE_STATE;

//----------------------------------------
// Macros
//----------------------------------------

#if defined(PARSE_NMEA_MESSAGES) || defined(PARSE_SBF_MESSAGES)
#define NMEA_SBF_PREAMBLE   nmeaSbfPreamble1,
#else
#define NMEA_SBF_PREAMBLE
#endif  // PARSE_NMEA_MESSAGES || PARSE_SBF_MESSAGES

#ifdef  PARSE_RTCM_MESSAGES
#define RTCM_PREAMBLE       rtcmPreamble,
#else
#define RTCM_PREAMBLE
#endif  // PARSE_RTCM_MESSAGES

#define GPS_PARSE_TABLE                 \
PARSE_ROUTINE const gpsParseTable[] =   \
{                                       \
    NMEA_SBF_PREAMBLE                   \
    RTCM_PREAMBLE                       \
};                                      \
                                        \
const int gpsParseTableEntries = sizeof(gpsParseTable) / sizeof(gpsParseTable[0]);

//----------------------------------------
// External values
//----------------------------------------

extern PARSE_ROUTINE const gpsParseTable[];
extern const int gpsParseTableEntries;

//----------------------------------------
// External routines
//----------------------------------------

// Main parser routine
uint8_t gpsMessageParserFirstByte(PARSE_STATE *parse, uint8_t data);

// Common NMEA and SBF parse routines
uint8_t nmeaSbfPreamble1(PARSE_STATE *parse, uint8_t data);
uint8_t nmeaSbfPreamble2(PARSE_STATE *parse, uint8_t data);

// NMEA parse routines
//uint8_t nmeaPreamble(PARSE_STATE *parse, uint8_t data);
uint8_t nmeaFindFirstComma(PARSE_STATE *parse, uint8_t data);
uint8_t nmeaFindAsterisk(PARSE_STATE *parse, uint8_t data);
uint8_t nmeaChecksumByte1(PARSE_STATE *parse, uint8_t data);
uint8_t nmeaChecksumByte2(PARSE_STATE *parse, uint8_t data);
uint8_t nmeaLineTermination(PARSE_STATE *parse, uint8_t data);

// SBF parse routines
//uint8_t sbfPreamble(PARSE_STATE *parse, uint8_t data);
//uint8_t sbfSync2(PARSE_STATE *parse, uint8_t data);
uint8_t sbfCRC1(PARSE_STATE *parse, uint8_t data);
uint8_t sbfCRC2(PARSE_STATE *parse, uint8_t data);
uint8_t sbfID1(PARSE_STATE *parse, uint8_t data);
uint8_t sbfID2(PARSE_STATE *parse, uint8_t data);
uint8_t sbfLength1(PARSE_STATE *parse, uint8_t data);
uint8_t sbfLength2(PARSE_STATE *parse, uint8_t data);
uint8_t sbfPayload(PARSE_STATE *parse, uint8_t data);

// RTCM parse routines
uint8_t rtcmPreamble(PARSE_STATE *parse, uint8_t data);
uint8_t rtcmReadLength1(PARSE_STATE *parse, uint8_t data);
uint8_t rtcmReadLength2(PARSE_STATE *parse, uint8_t data);
uint8_t rtcmReadMessage1(PARSE_STATE *parse, uint8_t data);
uint8_t rtcmReadMessage2(PARSE_STATE *parse, uint8_t data);
uint8_t rtcmReadData(PARSE_STATE *parse, uint8_t data);
uint8_t rtcmReadCrc(PARSE_STATE *parse, uint8_t data);

// External print routines
void printNmeaChecksumError(PARSE_STATE *parse);
void printRtcmChecksumError(PARSE_STATE *parse);
void printRtcmMaxLength(PARSE_STATE *parse);
void printSbfChecksumError(PARSE_STATE *parse);
void printSbfInvalidData(PARSE_STATE *parse);

#endif  // __GPS_MESSAGE_PARSER_H__
