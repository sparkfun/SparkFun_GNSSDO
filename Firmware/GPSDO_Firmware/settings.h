#ifndef __SETTINGS_H__
#define __SETTINGS_H__

// System can enter a variety of states
typedef enum
{
    STATE_GNSS_NOT_CONFIGURED = 0,
    STATE_GNSS_CONFIGURED, // GNSS has been configured: PVTGeodetic+ReceiverTime+IPStatus enabled on COM1
    STATE_GNSS_ERROR_BEFORE_FINETIME, // Error is non-zero before FineTime is true
    STATE_GNSS_FINETIME, // GNSS FINETIME bit is set. Oscillator control can begin
    STATE_GNSS_ERROR_AFTER_FINETIME, // PVTGeodetic Error is non-zero. Oscillator updates are paused
    STATE_NOT_SET, // Must be last on list
} SystemState;
volatile SystemState systemState = STATE_NOT_SET;
SystemState lastSystemState = STATE_NOT_SET;
SystemState requestedSystemState = STATE_NOT_SET;
bool newSystemStateRequested = false;

typedef enum
{
    RTK_MOSAIC_T = 0,
    // Add new values just above this line
    RTK_UNKNOWN,
} ProductVariant;
ProductVariant productVariant = RTK_UNKNOWN;

const char *const productDisplayNames[] = {
    "GPSDO",
    // Add new values just above this line
    "Unknown",
};
const int productDisplayNamesEntries = sizeof(productDisplayNames) / sizeof(productDisplayNames[0]);

const char *const platformFilePrefixTable[] = {
    "SFE_GPSDO",
    // Add new values just above this line
    "SFE_Unknown",
};
const int platformFilePrefixTableEntries = sizeof(platformFilePrefixTable) / sizeof(platformFilePrefixTable[0]);

const char *const platformPrefixTable[] = {
    "GPSDO",
    // Add new values just above this line
    "Unknown",
};
const int platformPrefixTableEntries = sizeof(platformPrefixTable) / sizeof(platformPrefixTable[0]);

typedef enum
{
    RTCM_TRANSPORT_STATE_WAIT_FOR_PREAMBLE_D3 = 0,
    RTCM_TRANSPORT_STATE_READ_LENGTH_1,
    RTCM_TRANSPORT_STATE_READ_LENGTH_2,
    RTCM_TRANSPORT_STATE_READ_MESSAGE_1,
    RTCM_TRANSPORT_STATE_READ_MESSAGE_2,
    RTCM_TRANSPORT_STATE_READ_DATA,
    RTCM_TRANSPORT_STATE_READ_CRC_1,
    RTCM_TRANSPORT_STATE_READ_CRC_2,
    RTCM_TRANSPORT_STATE_READ_CRC_3,
    RTCM_TRANSPORT_STATE_CHECK_CRC
} RtcmTransportState;

typedef enum
{
    MOSAIC_TIME_SYSTEM_GPS = 0,
    MOSAIC_TIME_SYSTEM_GALILEO = 1,
    MOSAIC_TIME_SYSTEM_GLONASS = 3,
    MOSAIC_TIME_SYSTEM_BEIDOU = 4,
    MOSAIC_TIME_SYSTEM_QZSS = 5,
    MOSAIC_TIME_SYSTEM_FUGRO = 100,
} mosaicTimeSystemIds;

// FugroTimeOffset 4255
typedef struct {
    const uint8_t TimeSystem;
    const uint8_t SysUsage;
    const char name[8];
    double RxClkBias_ms;
    bool updated;
} fugroTimeSystem;

fugroTimeSystem fugroTimeSystems[] = {
    { MOSAIC_TIME_SYSTEM_GPS, 0x01, "GPS", 0.0, false },
    { MOSAIC_TIME_SYSTEM_GALILEO, 0x02, "Galileo", 0.0, false },
    { MOSAIC_TIME_SYSTEM_GLONASS, 0x04, "GLONASS", 0.0, false },
    { MOSAIC_TIME_SYSTEM_BEIDOU, 0x08, "BeiDou", 0.0, false },
    { MOSAIC_TIME_SYSTEM_QZSS, 0, "QZSS", 0.0, false },
    { MOSAIC_TIME_SYSTEM_FUGRO, 0, "Fugro", 0.0, false },
};

#define NUM_FUGRO_CLK_BIASES (sizeof(fugroTimeSystems) / sizeof(fugroTimeSystem))

// Convert id (0,1,3,4,5,100) to index (0-5). Return NUM_FUGRO_CLK_BIASES (6) if id is invalid
uint8_t mosaicTimeSystemIndexFromId(uint8_t id) {
    int i = 0;
    for (; i < NUM_FUGRO_CLK_BIASES; i++) {
        if (fugroTimeSystems[i].TimeSystem == id)
            break;
    }
    return (uint8_t)i;
}

// Return the time system name for the given id. Return "Unknown" if id is invalid
const char * mosaicTimeSystemNameFromId(uint8_t id) {
    uint8_t index = mosaicTimeSystemIndexFromId(id);

    static const char unknown[] = "Unknown";
    if (index >= NUM_FUGRO_CLK_BIASES)
        return unknown;

    return fugroTimeSystems[index].name;
}

// Return the time system index (0-5) for the given name
uint8_t mosaicTimeSystemIndexFromName(const char *name)
{
    for (uint8_t i = 0; i < NUM_FUGRO_CLK_BIASES; i++)
    {
        if (strcmp(name, fugroTimeSystems[i].name) == 0)
            return i;
    }

    return 0; // This should never happen
}
double tcxoClockBias_ms; // Updated by updateTCXOClockBias
char rxClkBiasSource[8];

const char *const mosaicPVTErrorTable[] = {
    "None",
    "Measurements",
    "Ephemerides",
    "DOP",
    "Residuals",
    "Convergence",
    "Outliers",
    "Export",
    "Differential",
    "Base",
    "Ambiguities",
};
const int mosaicPVTErrorTableEntries = sizeof(mosaicPVTErrorTable) / sizeof(mosaicPVTErrorTable[0]);

const char * mosaicPVTErrorNameFromId(uint8_t id) {
    static const char unknown[] = "Unknown";
    if (id >= mosaicPVTErrorTableEntries)
        return unknown;
    return mosaicPVTErrorTable[id];
}

const char *const mosaicPPSParametersInterval[] = {
    "off",
    "msec10",
    "msec20",
    "msec50",
    "msec100",
    "msec200",
    "msec250",
    "msec500",
    "sec1",
    "sec2",
    "sec4",
    "sec5",
    "sec10",
    "sec30",
    "sec60",
};
const int mosaicPPSParametersIntervalEntries = sizeof(mosaicPPSParametersInterval) / sizeof(mosaicPPSParametersInterval[0]);

const char *const mosaicPPSParametersPolarity[] = {
    "Low2High",
    "High2Low",
};
const int mosaicPPSParametersPolarityEntries = sizeof(mosaicPPSParametersPolarity) / sizeof(mosaicPPSParametersPolarity[0]);

const char *const mosaicPPSParametersTimeScale[] = {
    "GPS",
    "Galileo",
    "BeiDou",
    "GLONASS",
    "UTC",
    "RxClock",
};
const int mosaicPPSParametersTimeScaleEntries = sizeof(mosaicPPSParametersTimeScale) / sizeof(mosaicPPSParametersTimeScale[0]);

typedef uint16_t RING_BUFFER_OFFSET;

// Return values for getString()
typedef enum
{
    INPUT_RESPONSE_GETNUMBER_EXIT =
        -9999999, // Less than min ECEF. User may be prompted for number but wants to exit without entering data
    INPUT_RESPONSE_GETNUMBER_TIMEOUT = -9999998,
    INPUT_RESPONSE_GETCHARACTERNUMBER_TIMEOUT = 255,
    INPUT_RESPONSE_GETCHARACTERNUMBER_EMPTY = 254,
    INPUT_RESPONSE_INVALID = -4,
    INPUT_RESPONSE_TIMEOUT = -3,
    INPUT_RESPONSE_OVERFLOW = -2,
    INPUT_RESPONSE_EMPTY = -1,
    INPUT_RESPONSE_VALID = 1,
} InputResponse;

// Display

typedef enum
{
    DISPLAY_64x48,
    DISPLAY_128x64,
    // Add new displays above this line
    DISPLAY_MAX_NONE // This represents the maximum numbers of display and also "no display"
} DisplayType;

const uint8_t DisplayWidth[DISPLAY_MAX_NONE] = { 64, 128 }; // We could get these from the oled, but this is const
const uint8_t DisplayHeight[DISPLAY_MAX_NONE] = { 48, 64 };

// These are the allowable messages to broadcast and log (if enabled)

// This is all the settings that can be set on RTK Surveyor. It's recorded to NVM and the config file.
typedef struct
{
    int sizeOfSettings = 0; // sizeOfSettings **must** be the first entry and must be int
    int rtkIdentifier = RTK_IDENTIFIER; // rtkIdentifier **must** be the second entry

    bool printDebugMessages = false;
    bool enableHeapReport = false; // Turn on to display free heap
    bool enableTaskReports = false; // Turn on to display task high water marks
    SystemState lastState = STATE_NOT_SET; // Start unit in default state

    int uartReceiveBufferSize = 1024 * 2; // This buffer is filled automatically as the UART receives characters.
    int gnssHandlerBufferSize =
        1024 * 4; // This buffer is filled from the UART receive buffer
    bool enablePrintBufferOverrun = false;

    uint16_t serialGNSSRxFullThreshold = 50; // RX FIFO full interrupt. Max of ~128. See pinUART1Task().
    uint8_t gnssReadTaskPriority =
        1; // Read from ZED-F9x and Write to circular buffer (SD, TCP, BT). 3 being the highest, and 0 being the lowest
    uint8_t handleGnssDataTaskPriority = 1; // Read from the cicular buffer and dole out to end points (SD, TCP, BT).
    uint8_t gnssReadTaskCore = 1;           // Core where task should run, 0=core, 1=Arduino
    uint8_t handleGnssDataTaskCore = 1;     // Core where task should run, 0=core, 1=Arduino
    uint8_t i2cInterruptsCore = 1; // Core where hardware is started and interrupts are assigned to, 0=core, 1=Arduino
    uint8_t gnssUartInterruptsCore =
        1; // Core where hardware is started and interrupts are assigned to, 0=core, 1=Arduino

    int16_t serialTimeoutGNSS = 1; // In ms - used during SerialGNSS.begin. Number of ms to pass of no data before
                                   // hardware serial reports data available.
    uint32_t dataPortBaud = 115200; // Default to 115200

    bool enablePrintBadMessages = false;
    bool enablePrintStates = true;
    bool enablePrintDuplicateStates = false;
    bool enablePrintRtcSync = false;
    bool enablePrintIdleTime = false;
    uint8_t enablePrintConditions = 2; // 0 : disabled; 1 : enabled (text); 2 : enabled (CSV)
    bool enablePrintConsumers = true;
    uint32_t periodicPrintInterval_ms = 1000;
    bool enablePrintGNSSMessages = false;
    bool enablePrintRingBufferOffsets = false;
    bool disableSetupButton = true;
    bool echoUserInput = true;

    int ppsInterval = 8; // sec1
    int ppsPolarity = 0; // Low2High
    float ppsDelay_ns = 0.0;
    int ppsTimeScale = 0; // GPS
    int ppsMaxSyncAge_s = 60;
    float ppsPulseWidth_ms = 5.0;

    int32_t tcxoControl = 0; // Store the TCXO control word - to aid locking after power off
    double rxClkBiasInitialLimit_ms = 1.0e-3; // Consider the clock bias 'bad' when > this many ms. Default: 1.0us (1.0e-3ms)
    double rxClkBiasLockLimit_ms = 10.0e-6; // Consider the clock locked when the bias is <= this many ms. Default: 10.0ns (10.0e-6ms)
    int rxClkBiasLimitCount = 3; // Consider the clock locked when the bias is <= rxClkBiasLockLimit_ms for this many successive readings. Default: 3
    // The default values for Pk and Ik come from very approximate Ziegler-Nichols tuning of the SiT5358:
    //   oscillation starts when Pk is ~1.4; with a period of ~5 seconds.
    double Pk = 0.63; // PI P term
    double Ik = 0.151; // PI I term
    bool preferNonCompositeGPSBias = false; // Prefer non-composite GPS bias - if available. Mutex with preferNonCompositeGalileoBias
    bool preferNonCompositeGalileoBias = false; // Prefer non-composite Galileo bias - if available. Mutex with preferNonCompositeGPSBias
    bool enableTCPServer = false; // Enable and configure mosaic-T IPS1 for TCP2way for the ESP32 console
    uint16_t tcpServerPort = 28785;

    // Add new settings above <------------------------------------------------------------>

} Settings;
Settings settings;

// Monitor which devices on the device are on or offline.
struct struct_online
{
    bool display = false;
    bool gnss = false;
    bool fs = false;
    bool rtc = false;
    bool i2c1 = false;
    bool i2c2 = false;
    bool tcxo = false;
} online;

#endif // __SETTINGS_H__
