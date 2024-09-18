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
    "mosaic-T",
    // Add new values just above this line
    "Unknown",
};
const int productDisplayNamesEntries = sizeof(productDisplayNames) / sizeof(productDisplayNames[0]);

const char *const platformFilePrefixTable[] = {
    "SFE_mosaic-T",
    // Add new values just above this line
    "SFE_Unknown",
};
const int platformFilePrefixTableEntries = sizeof(platformFilePrefixTable) / sizeof(platformFilePrefixTable[0]);

const char *const platformPrefixTable[] = {
    "mosaic-T",
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

const uint8_t mosaicTimeSystemIndexTable[] = {
    MOSAIC_TIME_SYSTEM_GPS,
    MOSAIC_TIME_SYSTEM_GALILEO,
    MOSAIC_TIME_SYSTEM_GLONASS,
    MOSAIC_TIME_SYSTEM_BEIDOU,
    MOSAIC_TIME_SYSTEM_QZSS,
    MOSAIC_TIME_SYSTEM_FUGRO,
};
const int mosaicTimeSystemIndexTableEntries = sizeof(mosaicTimeSystemIndexTable) / sizeof(mosaicTimeSystemIndexTable[0]);

uint8_t mosaicTimeSystemIndexFromId(uint8_t id) {
    int i = 0;
    for (; i < mosaicTimeSystemIndexTableEntries; i++) {
        if (mosaicTimeSystemIndexTable[i] == id)
            break;
    }
    return (uint8_t)i;
}

const char *const mosaicTimeSystemTable[] = {
    "GPS",
    "Galileo",
    "GLONASS",
    "BeiDou",
    "QZSS",
    "Fugro",
};
const int mosaicTimeSystemTableEntries = sizeof(mosaicTimeSystemTable) / sizeof(mosaicTimeSystemTable[0]);

const char * mosaicTimeSystemNameFromId(uint8_t id) {
    uint8_t index = mosaicTimeSystemIndexFromId(id);
    static const char unknown[] = "Unknown";
    if (index >= mosaicTimeSystemIndexTableEntries)
        return unknown;
    return mosaicTimeSystemTable[index];
}

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
    bool enablePrintConditions = true;
    bool enablePrintConsumers = true;
    uint32_t periodicPrintInterval_ms = 5000;
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
    double rxClkBiasLockLimit_ms = 10.0e-6; // Consider the clock locked when the bias is <= this many ms. Default: 10.0ns (10.0e-6ms)
    double Pk = -0.25; // PI P term
    double Ik = -0.01; // PI I term

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
