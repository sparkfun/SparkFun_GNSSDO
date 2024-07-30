#ifndef __SETTINGS_H__
#define __SETTINGS_H__

// System can enter a variety of states
typedef enum
{
    STATE_GNSS_NOT_CONFIGURED = 0,
    STATE_GNSS_CONFIGURED, // GNSS has been configured: PVTGeodetic+ReceiverTime+IPStatus enabled on COM1
    STATE_GNSS_FINETIME, // GNSS FINETIME bit is set. Oscillator control can begin
    STATE_GNSS_ERROR, // PVTGeodetic Error is non-zero. Oscillator updates are paused
    STATE_NOT_SET, // Must be last on list
} SystemState;
volatile SystemState systemState = STATE_GNSS_NOT_CONFIGURED;
SystemState lastSystemState = STATE_GNSS_NOT_CONFIGURED;
SystemState requestedSystemState = STATE_GNSS_NOT_CONFIGURED;
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
        return &unknown;
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
        return &unknown;
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

// Define the periodic display values
typedef uint32_t PeriodicDisplay_t;

enum PeriodDisplayValues
{
    PD_IP = 0, //  0
    PD_TIME,   //  1
    PD_LAT,    //  2
    PD_LONG,   //  3
    PD_SYS,    //  4
    PD_ERROR,  //  5
    PD_FINE,   //  6
    PD_BIAS,   //  7
    // Add new values before this line
};

#define PERIODIC_MASK(x) (1 << x)
#define PERIODIC_DISPLAY(x) (periodicDisplay & PERIODIC_MASK(x))
#define PERIODIC_CLEAR(x) periodicDisplay &= ~PERIODIC_MASK(x)
#define PERIODIC_SETTING(x) (settings.periodicDisplay & PERIODIC_MASK(x))
#define PERIODIC_TOGGLE(x) settings.periodicDisplay ^= PERIODIC_MASK(x)

// These are the allowable messages to broadcast and log (if enabled)

// This is all the settings that can be set on RTK Surveyor. It's recorded to NVM and the config file.
typedef struct
{
    int sizeOfSettings = 0; // sizeOfSettings **must** be the first entry and must be int
    int rtkIdentifier = RTK_IDENTIFIER; // rtkIdentifier **must** be the second entry
    bool printDebugMessages = false;
    bool enableDisplay = true;
    bool enableHeapReport = false; // Turn on to display free heap
    bool enableTaskReports = false; // Turn on to display task high water marks
    SystemState lastState = STATE_GNSS_NOT_CONFIGURED; // Start unit in default state
    bool enablePrintBufferOverrun = false;
    PeriodicDisplay_t periodicDisplay = (PeriodicDisplay_t)0; // Turn off all periodic debug displays by default.
    uint32_t periodicDisplayInterval = 15 * 1000;

    int gnssHandlerBufferSize =
        1024 * 4; // This buffer is filled from the UART receive buffer
    uint16_t serialGNSSRxFullThreshold = 50; // RX FIFO full interrupt. Max of ~128. See pinUART2Task().
    int uartReceiveBufferSize = 1024 * 2; // This buffer is filled automatically as the UART receives characters.
    uint8_t i2cInterruptsCore = 1; // Core where hardware is started and interrupts are assigned to, 0=core, 1=Arduino
    uint8_t handleGnssDataTaskCore = 1;     // Core where task should run, 0=core, 1=Arduino
    uint8_t handleGnssDataTaskPriority = 1; // Read from the cicular buffer and dole out to end points (SD, TCP, BT).
    uint8_t gnssReadTaskCore = 1;           // Core where task should run, 0=core, 1=Arduino
    uint8_t gnssReadTaskPriority =
        1; // Read from ZED-F9x and Write to circular buffer (SD, TCP, BT). 3 being the highest, and 0 being the lowest

    int ppsInterval = 8; // sec1
    int ppsPolarity = 0; // Low2High
    float ppsDelay_ns = 0.0;
    int ppsTimeScale = 0; // GPS
    int ppsMaxSyncAge_s = 60;
    float ppsPulseWidth_ms = 5.0;

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
} online;

#endif // __SETTINGS_H__
