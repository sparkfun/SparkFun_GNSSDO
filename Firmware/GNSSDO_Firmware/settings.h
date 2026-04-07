#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "GNSSDO_TCXO.h"
#include "SiT5358_TCXO.h"
#include "SiT5811_OCXO.h"
#include "STP3593LF_OCXO.h"

// System can enter a variety of states
typedef enum
{
    STATE_GNSS_NOT_CONFIGURED = 0,
    STATE_GNSS_CONFIGURED, // GNSS has been configured: PVTGeodetic+ReceiverTime+IPStatus enabled on COM1
    STATE_GNSS_ERROR_BEFORE_FINETIME, // Error is non-zero before FineTime is true
    STATE_TCXO_WARMUP, // Wait for the TCXO temperature to become stable - if sensor is present
    STATE_GNSS_ERROR_DURING_WARMUP,
    STATE_GNSS_FINETIME, // GNSS FINETIME bit is set. Oscillator control can begin in frequency locked mode
    STATE_GNSS_ERROR_AFTER_FINETIME, // PVTGeodetic Error is non-zero. Oscillator updates are paused
    STATE_GNSS_FREQUENCY_LOCK, // GNSS is frequency locked. Minimise the bias without blowing up the integrator
    STATE_GNSS_ERROR_AFTER_FREQUENCY_LOCK,
    STATE_GNSS_PHASE_LOCK, // GNSS has achieved phase lock. Maintain the bias using the PLL
    STATE_GNSS_ERROR_AFTER_PHASE_LOCK,
    STATE_NOT_SET, // Must be last on list
} SystemState;
volatile SystemState systemState = STATE_NOT_SET;
SystemState lastSystemState = STATE_NOT_SET;
SystemState requestedSystemState = STATE_NOT_SET;
bool newSystemStateRequested = false;

typedef struct
{
    const SystemState systemState;
    const char *stateName;
} SystemStatesName;

const SystemStatesName SystemStatesNames[] = {
    { STATE_GNSS_NOT_CONFIGURED, "NOT_CONFIGURED" },
    { STATE_GNSS_CONFIGURED, "CONFIGURED" },
    { STATE_GNSS_ERROR_BEFORE_FINETIME, "ERROR_BEFORE_FINETIME" },
    { STATE_TCXO_WARMUP, "WARMUP" },
    { STATE_GNSS_ERROR_DURING_WARMUP, "ERROR_DURING_WARMUP" },
    { STATE_GNSS_FINETIME, "FINETIME" },
    { STATE_GNSS_ERROR_AFTER_FINETIME, "ERROR_AFTER_FINETIME" },
    { STATE_GNSS_FREQUENCY_LOCK, "FREQUENCY_LOCK" },
    { STATE_GNSS_ERROR_AFTER_FREQUENCY_LOCK, "ERROR_AFTER_FREQUENCY_LOCK" },
    { STATE_GNSS_PHASE_LOCK, "PHASE_LOCK" },
    { STATE_GNSS_ERROR_AFTER_PHASE_LOCK, "ERROR_AFTER_PHASE_LOCK" },
    { STATE_NOT_SET, "NOT_SET" },
};

const int numSystemStatesNames = sizeof(SystemStatesNames) / sizeof(SystemStatesNames[0]);

typedef enum
{
    GNSSDO = 0,
    GNSSDO_PLUS,
    // Add new values just above this line
    GNSSDO_UNKNOWN,
} ProductVariant;
ProductVariant productVariant = GNSSDO_UNKNOWN;

const char *const productDisplayNames[] = {
    "GNSSDO",
    "GNSSDO+",
    // Add new values just above this line
    "Unknown",
};
const int productDisplayNamesEntries = sizeof(productDisplayNames) / sizeof(productDisplayNames[0]);

const char *const platformFilePrefixTable[] = {
    "SFE_GNSSDO",
    "SFE_GNSSDO_PLUS",
    // Add new values just above this line
    "SFE_Unknown",
};
const int platformFilePrefixTableEntries = sizeof(platformFilePrefixTable) / sizeof(platformFilePrefixTable[0]);

const char *const platformPrefixTable[] = {
    "GNSSDO",
    "GNSSDO_PLUS",
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
    float RxClkDrift_ppm;
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

const int NUM_FUGRO_CLK_BIASES = sizeof(fugroTimeSystems) / sizeof(fugroTimeSystems[0]);

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

volatile double tcxoClockBias_ms; // Updated by updateTCXOClockBias
volatile float tcxoClockDrift_ppm;
char rxClkBiasSource[8];
char sysSource[16]; // "Fugro (Galileo)"
volatile double tcxoClockBiasChange_ms; // Change in clock bias during initial frequency locking
volatile double tcxoTemperature; // Holds the TCXO temperature - if sensor is present

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

// TCXO / OCXO

typedef enum
{
    SFE_GNSSDO_OSC_SIT5811 = 0x50,
    SFE_GNSSDO_OSC_SIT5358 = 0x60,
    SFE_GNSSDO_OSC_STP3593LF = 0x70,
} OscillatorsByAddress;
bool presentSIT5811 = false;
bool presentSIT5358 = false;
bool presentSTP3593LF = false;

// PHT (MS8607)

bool presentMS8607 = false;

// TCXO internal temperature sensor - if present
bool presentTcxoTemperature = false;

// TCXO can save its frequency control word - if present
bool presentTcxoSaveControl = false;

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

// This is all the firmware settings. It's recorded to NVM and the config file.
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
    bool enablePrintConsumers = false;
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

    int64_t tcxoControl = 0; // Store the TCXO control word - to aid locking after power off
    double rxFrequencyLockErrorLimit_s = 1.0e-10; // Stay in STATE_GNSS_FINETIME until the change in the bias is <= this many s/s
    double rxPhaseErrorLimit_s = 1.0e-7; // Stay in STATE_GNSS_FREQUENCY_LOCK until the bias is <= this many s
    double PkSteer = 0.5; // PI P term for initial frequency steering (updated if needed by beginTCXO)
    double IkSteer = 0.5; // PI I term for initial frequency steering (updated if needed by beginTCXO)
    double Pk = 0.63; // PI P term (default for the SiT5358 - updated if needed by beginTCXO)
    double Ik = 0.151; // PI I term (default for the SiT5358 - updated if needed by beginTCXO)
    uint8_t lastSeenTCXO = 0; // Use this to identify a change of oscillator (by its I2C address)
    bool preferNonCompositeGPSBias = false; // Prefer non-composite GPS bias - if available. Mutex with preferNonCompositeGalileoBias
    bool preferNonCompositeGalileoBias = false; // Prefer non-composite Galileo bias - if available. Mutex with preferNonCompositeGPSBias
    bool enableTCPServer = false; // Enable and configure mosaic-T IPS1 for TCP2way for the ESP32 console
    uint16_t tcpServerPort = 28785;
    uint32_t previousIP = 0; // Store the previous IP address
    double tcxoTemperatureStability = 0.01; // Required temperature stability for TCXO warm up (ADU)
    unsigned long tcxoMinWarmup_s = 120; // Required minimum warmup
    double tcxoRampRateLimit_sps = 250.0e-9; // This limits the ramp rate during steering (s/s) (updated if needed by beginTCXO)
    double tcxoRampStepSize_s = 1.0e-9; // Ole uses 1ns/s
    double tcxoFreqLockIkMultiplier = 10.0; // STP3593 needs much more I. SiT5358 less so.

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
    bool pht = false;
} online;

#endif // __SETTINGS_H__
