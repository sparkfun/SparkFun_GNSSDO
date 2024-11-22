/*
  July 30th, 2024
  SparkFun Electronics
  Paul Clark

  This is the firmware for the SparkFun GNSSDO.
  It runs on an ESP32 and communicates with the mosaic-T and SiT5358.

  Compiled with Arduino v1.8.19 with ESP32 core v3.0.1.

  Set the board to "ESP32 Wrover Module"

  Settings are pulled from ESP32's file system LittleFS.

  Version history:
  1.0: Initial release
  1.1: Add TCP support
       The console can be accessed via TCP (COM3 is daisy chained to IPS1)
       On the v1.0 PCB, link:
       mosaic COM3 TX (TX3) to ESP32 GPIO 34
       mosaic COM3 RX (RX3) to ESP32 GPIO 23
  1.2: Add support for SiT5811 and STP3593LF oscillators
*/

// This is passed in from compiler extra flags
#ifndef FIRMWARE_VERSION_MAJOR
#define FIRMWARE_VERSION_MAJOR 9
#define FIRMWARE_VERSION_MINOR 9
#endif

// Define the RTK board identifier:
//  This is an int which is unique to this variant of the RTK Surveyor hardware which allows us
//  to make sure that the settings stored in flash (LittleFS) are correct for this version of the RTK
//  (sizeOfSettings is not necessarily unique and we want to avoid problems when swapping from one variant to another)
//  It is the sum of:
//    the major firmware version * 0x10
//    the minor firmware version
#define RTK_IDENTIFIER (FIRMWARE_VERSION_MAJOR * 0x10 + FIRMWARE_VERSION_MINOR)

#include <Arduino.h>
#include <Wire.h>
#include "settings.h"

#define MAX_CPU_CORES 2
#define IDLE_COUNT_PER_SECOND 515400 //Found by empirical sketch
#define IDLE_TIME_DISPLAY_SECONDS 5
#define MAX_IDLE_TIME_COUNT (IDLE_TIME_DISPLAY_SECONDS * IDLE_COUNT_PER_SECOND)
#define MILLISECONDS_IN_A_SECOND 1000
#define MILLISECONDS_IN_A_MINUTE (60 * MILLISECONDS_IN_A_SECOND)
#define MILLISECONDS_IN_AN_HOUR (60 * MILLISECONDS_IN_A_MINUTE)
#define MILLISECONDS_IN_A_DAY (24 * MILLISECONDS_IN_AN_HOUR)

#define SECONDS_IN_A_MINUTE     60
#define SECONDS_IN_AN_HOUR      (60 * SECONDS_IN_A_MINUTE)
#define SECONDS_IN_A_DAY        (24 * SECONDS_IN_AN_HOUR)

// Hardware connections
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// These pins are set in beginBoard()
int pin_errorLED = -1;
int pin_lockLED = -1;

const int pin_serial0TX = 1;
const int pin_serial0RX = 3;

int pin_serial0TX_Alt = -1;
int pin_serial0RX_Alt = -1;

int pin_serial1TX = -1;
int pin_serial1RX = -1;
int pin_serial1CTS = -1;
int pin_serial1RTS = -1;

int pin_serial2TX = -1;
int pin_serial2RX = -1;

int pin_SDA1 = -1;
int pin_SCL1 = -1;

int pin_SDA2 = -1;
int pin_SCL2 = -1;

int pin_setupButton = -1;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// I2C for oscillator and OLED
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <Wire.h>
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// LittleFS for storing settings
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <LittleFS.h>

char settingsFileName[60];                 // Contains the %s_Settings_%d.txt with current profile number set

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Handy library for setting ESP32 system time to GNSS time
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <ESP32Time.h> //http://librarymanager/All#ESP32Time by FBiego v2.0.0
ESP32Time rtc;
unsigned long syncRTCInterval = 1000; // To begin, sync RTC every second. Interval can be increased once sync'd.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define platformPrefix      platformPrefixTable[productVariant] // Sets the prefix for broadcast names
#define platformFilePrefix  platformFilePrefixTable[productVariant] // Sets the prefix for logs and settings files

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// These globals are updated regularly via the SBF parser

// ReceiverTime 5914
unsigned long gnssTimeArrivalMillis = 0;
bool gnssTimeUpdated[3] = { false, false, false }; // RTC, TCXO, printConditions
uint32_t gnssTOW_ms = 0;
uint8_t gnssDay = 0;
uint8_t gnssMonth = 0;
uint16_t gnssYear = 0;
uint8_t gnssHour = 0;
uint8_t gnssMinute = 0;
uint8_t gnssSecond = 0;
bool gnssWNSet = false;
bool gnssToWSet = false;
bool gnssFineTime = false;

// PVTGeodetic 4007
unsigned long gnssPVTArrivalMillis = 0;
bool gnssPVTUpdated = false;
double gnssLatitude_d = 0.0;
double gnssLongitude_d = 0.0;
float gnssAltitude_m = 0.0;
uint8_t gnssTimeSys = 255; // Unknown
uint8_t gnssError = 255; // Unknown
double gnssClockBias_ms = 0.0;

// IPStatus 4058
IPAddress gnssIP = IPAddress((uint32_t)0);

// FugroTimeOffset 4255
// fugroTimeSystem fugroTimeSystems[] is in settings.h

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// GPS parse table
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Define the parsers that get included
#define PARSE_NMEA_MESSAGES
#define PARSE_RTCM_MESSAGES
#define PARSE_SBF_MESSAGES

// Build the GPS_PARSE_TABLE macro
#include "GpsMessageParser.h" // Include the parser

// Create the GPS message parse table instance
GPS_PARSE_TABLE;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <driver/uart.h>      // Required for uart_set_rx_full_threshold() on cores <v2.0.5
HardwareSerial serialConsole(0); // Use UART0 for Console
HardwareSerial serialGNSS(1); // Use UART1 for GNSS
HardwareSerial serialGNSSConfig(2); // Use UART2 for GNSS configuration

#define SERIAL_SIZE_TX 512
uint8_t wBuffer[SERIAL_SIZE_TX]; // Buffer for writing from incoming SPP to F9P

// Array of start of sentence offsets into the ring buffer
#define AMOUNT_OF_RING_BUFFER_DATA_TO_DISCARD (settings.gnssHandlerBufferSize >> 2)
#define AVERAGE_SENTENCE_LENGTH_IN_BYTES    32
RING_BUFFER_OFFSET * rbOffsetArray;
uint16_t rbOffsetEntries;

uint8_t *ringBuffer; // Buffer for reading from F9P. At 230400bps, 23040 bytes/s. If SD blocks for 250ms, we need 23040
                     // * 0.25 = 5760 bytes worst case.
TaskHandle_t gnssReadTaskHandle =
    nullptr; // Store handles so that we can kill them if user goes into WiFi NTRIP Server mode
const int gnssReadTaskStackSize = 2500;

TaskHandle_t handleGnssDataTaskHandle = nullptr;
const int handleGnssDataTaskStackSize = 3000;

TaskHandle_t pinUART1TaskHandle = nullptr; // Dummy task to start hardware on an assigned core
volatile bool uart1pinned = false; // This variable is touched by core 0 but checked by core 1. Must be volatile.

TaskHandle_t pinI2C1TaskHandle = nullptr; // Dummy task to start hardware on an assigned core
volatile bool i2c1Pinned = false; // This variable is touched by core 0 but checked by core 1. Must be volatile.

TaskHandle_t pinI2C2TaskHandle = nullptr; // Dummy task to start hardware on an assigned core
volatile bool i2c2Pinned = false; // This variable is touched by core 0 but checked by core 1. Must be volatile.

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// External Display
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <SparkFun_Qwiic_OLED.h> //http://librarymanager/All#SparkFun_Qwiic_Graphic_OLED

TwoWire *i2c_1 = nullptr; // OLED (400kHz)
TwoWire *i2c_2 = nullptr; // TCXO (100kHz)
TwoWire *i2cDisplay = nullptr;
TwoWire *i2cTCXO = nullptr;

DisplayType displayType = DISPLAY_MAX_NONE;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// DCTCXO
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GNSSDO_TCXO *myTCXO;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Low frequency tasks
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <Ticker.h>

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Buttons - Interrupt driven and debounce
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <JC_Button.h>      //http://librarymanager/All#JC_Button v2.1.2
Button *setupBtn = nullptr; // We can't instantiate the buttons here because we don't yet know what pin numbers to use

TaskHandle_t ButtonCheckTaskHandle = nullptr;
const uint8_t ButtonCheckTaskPriority = 1; // 3 being the highest, and 0 being the lowest
const int buttonTaskStackSize = 2000;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
uint8_t wifiMACAddress[6];     // Display this address in the system menu
char deviceName[70];           // The serial string that is broadcast. Ex: 'Surveyor Base-BC61'
const uint16_t menuTimeout = 60 * 10; // Menus will exit/timeout after this number of seconds
int systemTime_minutes = 0;           // Used to test if logging is less than max minutes
bool inMainMenu = false;              // Set true when in the serial config menu system.

uint32_t lastDisplayUpdate = 0;
bool forceDisplayUpdate = false;
uint32_t lastSystemStateUpdate = 0;
bool forceSystemStateUpdate = false; // Set true to avoid update wait
uint32_t lastPrintConditions = 0;

long lastStackReport = 0;         // Controls the report rate of stack highwater mark within a task
uint32_t lastHeapReport = 0;      // Report heap every 1s if option enabled
uint32_t lastTaskHeapReport = 0;  // Report task heap every 1s if option enabled

uint32_t rtcmLastReceived = 0;

unsigned long splashStart = 0; // Controls how long the splash is displayed for. Currently min of 2s.

unsigned long startTime = 0;           // Used for checking longest running functions

TaskHandle_t idleTaskHandle[MAX_CPU_CORES];
uint32_t max_idle_count = MAX_IDLE_TIME_COUNT;

uint16_t failedParserMessages_RTCM = 0;
uint16_t failedParserMessages_NMEA = 0;
uint16_t failedParserMessages_SBF = 0;

bool rtcSyncd = false;      // Set to true when the RTC has been sync'd
bool ppsStarted = false;    // Set to true when PPS have started. Cleared by menu changes.

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define DEAD_MAN_WALKING_ENABLED    0

#if DEAD_MAN_WALKING_ENABLED

// Developer subsitutions enabled by changing DEAD_MAN_WALKING_ENABLED
// from 0 to 1
volatile bool deadManWalking;
#define DMW_if                  if (deadManWalking)
#define DMW_c(string)           DMW_if systemPrintf("%s called\r\n", string);
#define DMW_ds(routine, dataStructure) DMW_if routine(dataStructure, dataStructure->state);
#define DMW_m(string)           DMW_if systemPrintln(string);
#define DMW_r(string)           DMW_if systemPrintf("%s returning\r\n",string);
#define DMW_rs(string, status)  DMW_if systemPrintf("%s returning %d\r\n",string, (int32_t)status);
#define DMW_st(routine, state)  DMW_if routine(state);

#define START_DEAD_MAN_WALKING                          \
{                                                       \
    deadManWalking = true;                              \
                                                        \
    /* Output as much as possible to identify the location of the failure */    \
    settings.printDebugMessages = true;                 \
    settings.enableI2Cdebug = true;                     \
    settings.enableHeapReport = true;                   \
    settings.enableTaskReports = true;                  \
    settings.enablePrintState = true;                   \
    settings.enablePrintConditions = 2;                 \
    settings.enablePrintIdleTime = true;                \
    settings.enablePrintBadMessages = true;             \
    settings.enablePrintRingBufferOffsets = true;       \
    settings.enablePrintStates = true;                  \
    settings.enablePrintDuplicateStates = true;         \
    settings.enablePrintRtcSync = true;                 \
    settings.enablePrintBufferOverrun = true;           \
}

#else   // 0

// Production substitutions
#define deadManWalking              0
#define DMW_if                      if (0)
#define DMW_c(string)
#define DMW_ds(routine, dataStructure)
#define DMW_m(string)
#define DMW_r(string)
#define DMW_rs(string, status)
#define DMW_st(routine, state)

#endif  // 0

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Initialize any globals that can't easily be given default values

void initializeGlobals()
{
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void setup()
{
    initializeGlobals(); // Initialize any global variables that can't be given default values

    beginConsole(115200, false); // UART0 for programming and debugging. Don't allow Alt pins to be used yet
    systemPrintln();
    systemPrintln();

    DMW_c("verifyTables");
    verifyTables (); // Verify the consistency of the internal tables

    DMW_c("identifyBoard");
    identifyBoard(); // Determine what hardware platform we are running on

    DMW_c("beginBoard");
    beginBoard();

    DMW_c("beginLEDs");
    beginLEDs(); // LED setup

    DMW_c("beginI2C1");
    beginI2C1();

    DMW_c("beginI2C2");
    beginI2C2();

    DMW_c("beginDisplay");
    beginDisplay(i2cDisplay); // Start display to be able to display any errors

    DMW_c("findSpiffsPartition");
    if (!findSpiffsPartition())
    {
        printPartitionTable(); // Print the partition tables
        reportFatalError("spiffs partition not found!");
    }

    DMW_c("displaySplash");
    displaySplash(); // Display the RTK product name and firmware version

    DMW_c("beginFS");
    beginFS(); // Start LittleFS file system for settings

    DMW_c("loadSettings");
    loadSettings();

    DMW_c("beginIdleTasks");
    beginIdleTasks(); // Enable processor load calculations

    DMW_c("beginUARTs");
    beginUARTs(); // Start UART1 on core 0, used to receive serial from GNSS

    DMW_c("tasksStartUART1");
    tasksStartUART1();

    DMW_c("beginTCXO");
    beginTCXO(i2cTCXO); // Configure SiTime oscillator

    DMW_c("beginSystemState");
    beginSystemState(); // Determine initial system state. Configure GNSS messages.

    DMW_c("beginGNSS");
    beginGNSS(); // Connect to GNSS

    DMW_c("configureGNSSTCPServer");
    configureGNSSTCPServer(); // Configure TCP

    systemPrintf("Boot time: %d\r\n", millis());

    if (settings.enableTCPServer)
        systemPrintf("TCP Server is enabled. Please connect on port %d to view the console\r\n", settings.tcpServerPort);

    beginConsole(115200, true); // Swap to Alt pins if TCP is enabled
}

void loop()
{
    DMW_c("updateSystemState");
    updateSystemState();

    DMW_c("updateDisplay");
    updateDisplay();

    DMW_c("updateRTC");
    updateRTC(); // Set system time to GNSS once we have fix

    DMW_c("reportHeap");
    reportHeap(); // If debug enabled, report free heap

    DMW_c("updateSerial");
    updateSerial(); // Menu system via ESP32 USB connection

    DMW_c("printConditions");
    printConditions(); // Periodically print GNSS coordinates etc. if enabled

    // A small delay prevents panic if no other I2C or functions are called
    delay(10);
}

// Perform any essential background tasks while waiting for input from the user
// This is just regular code, not a true task. If we ran it in a task, we'd need semaphores etc.
// to avoid COM port collisions...
void performBackgroundTasks()
{
    DMW_c("updateSystemState");
    updateSystemState(); // Keep updating the system state. This keeps the TCXO updated.

    delay(1); // Yield to processor
}

// Once we have a fix, sync system clock to GNSS
void updateRTC()
{
    static uint16_t syncAge = 0;

    static bool firstTime = true;
    if (firstTime)
    {
        gnssTimeUpdated[0] = false; // This ensures gnssTimeUpdated[0] isn't stale
        firstTime = false;
    }

    if (online.gnss == true) // Only do this if the GNSS is online
    {
        if (gnssTimeUpdated[0]) // Only do this if we have fresh time
        {
            gnssTimeUpdated[0] = false;

            syncAge++; // Update syncAge every second
            if (syncAge == 3600) // Wrap every hour
                syncAge = 0;

            if (gnssWNSet && gnssToWSet && gnssFineTime) // Only do this if FineTime is set
            {
                // Only do this if the rtc has not been sync'd previously
                // or if it is an hour since the last sync
                if ((online.rtc == false) || (syncAge == 0))
                {
                    // To perform the time zone adjustment correctly, it's easiest if we convert the GNSS time and date
                    // into Unix epoch first and then correct for the arrival time
                    uint32_t epochSecs;
                    uint32_t epochMillis;
                    convertGnssTimeToEpoch(&epochSecs, &epochMillis);
                    
                    epochMillis += millis() - gnssTimeArrivalMillis; // Remove the lag

                    // Set the internal system time
                    rtc.setTime(epochSecs, epochMillis * 1000);

                    online.rtc = true;

                    if (settings.enablePrintRtcSync)
                    {
                        systemPrint("System time set to: ");
                        systemPrintln(rtc.getDateTime(true));
                    }
                }
            }
        }
    }
}

