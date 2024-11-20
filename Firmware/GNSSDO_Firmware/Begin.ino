/*------------------------------------------------------------------------------
Begin.ino

  This module implements the initial startup functions for GNSS, display, etc.
------------------------------------------------------------------------------*/

#include <esp_mac.h> // required - exposes esp_mac_type_t values

//----------------------------------------
// Constants
//----------------------------------------

#define MAX_ADC_VOLTAGE 3300 // Millivolts

// Testing shows the combined ADC+resistors is under a 1% window
#define TOLERANCE 5.20 // Percent:  94.8% - 105.2%

//----------------------------------------
// Hardware initialization functions
//----------------------------------------
// Determine if the measured value matches the product ID value
// idWithAdc applies resistor tolerance using worst-case tolerances:
// Upper threshold: R1 down by TOLERANCE, R2 up by TOLERANCE
// Lower threshold: R1 up by TOLERANCE, R2 down by TOLERANCE
bool idWithAdc(uint16_t mvMeasured, float r1, float r2)
{
    float lowerThreshold;
    float upperThreshold;

    //                                ADC input
    //                       r1 KOhms     |     r2 KOhms
    //  MAX_ADC_VOLTAGE -----/\/\/\/\-----+-----/\/\/\/\----- Ground

    // Return true if the mvMeasured value is within the tolerance range
    // of the mvProduct value
    upperThreshold = ceil(MAX_ADC_VOLTAGE * (r2 * (1.0 + (TOLERANCE / 100.0))) /
                          ((r1 * (1.0 - (TOLERANCE / 100.0))) + (r2 * (1.0 + (TOLERANCE / 100.0)))));
    lowerThreshold = floor(MAX_ADC_VOLTAGE * (r2 * (1.0 - (TOLERANCE / 100.0))) /
                           ((r1 * (1.0 + (TOLERANCE / 100.0))) + (r2 * (1.0 - (TOLERANCE / 100.0)))));

    // systemPrintf("r1: %0.2f r2: %0.2f lowerThreshold: %0.0f mvMeasured: %d upperThreshold: %0.0f\r\n", r1, r2,
    // lowerThreshold, mvMeasured, upperThreshold);

    return (upperThreshold > mvMeasured) && (mvMeasured > lowerThreshold);
}

// Use a pair of resistors on pin 35 to ID the board type
// If the ID resistors are not available then use a variety of other methods
// (I2C, GPIO test, etc) to ID the board.
// Assume no hardware interfaces have been started so we need to start/stop any hardware
// used in tests accordingly.
void identifyBoard()
{
    // Use ADC to check the resistor divider
    int pin_deviceID = 35;
    uint16_t idValue = analogReadMilliVolts(pin_deviceID);
    systemPrintf("Board ADC ID (mV): %d\r\n", idValue);

    // Order the following ID checks, by millivolt values high to low

    // GNSSDO: 1/1  -->  1571mV < 1650mV < 1729mV
    if (idWithAdc(idValue, 1, 1))
        productVariant = RTK_MOSAIC_T;

    else
    {
        systemPrintln("Out of band or nonexistent resistor IDs");
        productVariant = RTK_UNKNOWN;
    }
}

void beginBoard()
{
    if (productVariant == RTK_UNKNOWN)
    {
        reportFatalError("RTK Variant Unknown");
    }

    // Setup hardware pins
    if (productVariant == RTK_MOSAIC_T)
    {
        //   ESP32-WROVER-IE Pin Allocations:
        //   D0  : Boot + Boot Button
        //   D1  : Serial TX (CH340 RX)
        //   D2  : N/C
        //   D3  : Serial RX (CH340 TX)
        //   D4  : Serial RX (mosaic-T COM4 TX)
        //   D5  : N/C
        //   D12 : N/C
        //   D13 : Serial RX (mosaic-T COM1 TX)
        //   D14 : Serial TX (mosaic-T COM1 RX)
        //   D15 : N/C
        //   D16 : N/C
        //   D17 : N/C
        //   D18 : I2C SDA2 (SiT5358)
        //   D19 : I2C SCL2 (SiT5358)
        //   D21 : I2C SDA (OLED)
        //   D22 : I2C SCL (OLED)
        //   D23 : N/C
        //   D25 : Serial TX (mosaic-T COM4 RX)
        //   D26 : Serial CTS (mosaic-T COM1 CTS)
        //   D27 : Serial RTS (mosaic-T COM1 RTS)
        //   D32 : Error LED
        //   D33 : Lock LED
        //   A34 : N/C
        //   A35 : Device Sense (resistor divider)
        //   A36 : MRDY (mosaic-T module ready)
        //   A39 : N/C

        pin_errorLED = 32;
        pin_lockLED = 33;

        pin_serial0TX_Alt = 23;
        pin_serial0RX_Alt = 34;
        pin_serial1TX = 14;
        pin_serial1RX = 13;
        pin_serial1CTS = 26;
        pin_serial1RTS = 27;

        pin_serial2TX = 25;
        pin_serial2RX = 4;

        pin_SDA1 = 21;
        pin_SCL1 = 22;

        pin_SDA2 = 18;
        pin_SCL2 = 19;

        pin_setupButton = 0;

        displayType = DISPLAY_128x64;
    }

    char versionString[21];
    getFirmwareVersion(versionString, sizeof(versionString));
    systemPrintf("SparkFun RTK %s %s\r\n", platformPrefix, versionString);

    // Get unit MAC address
    esp_read_mac(wifiMACAddress, ESP_MAC_WIFI_STA);

    systemPrint("Reset reason: ");
    switch (esp_reset_reason())
    {
    case ESP_RST_UNKNOWN:
        systemPrintln("ESP_RST_UNKNOWN");
        break;
    case ESP_RST_POWERON:
        systemPrintln("ESP_RST_POWERON");
        break;
    case ESP_RST_SW:
        systemPrintln("ESP_RST_SW");
        break;
    case ESP_RST_PANIC:
        systemPrintln("ESP_RST_PANIC");
        break;
    case ESP_RST_INT_WDT:
        systemPrintln("ESP_RST_INT_WDT");
        break;
    case ESP_RST_TASK_WDT:
        systemPrintln("ESP_RST_TASK_WDT");
        break;
    case ESP_RST_WDT:
        systemPrintln("ESP_RST_WDT");
        break;
    case ESP_RST_DEEPSLEEP:
        systemPrintln("ESP_RST_DEEPSLEEP");
        break;
    case ESP_RST_BROWNOUT:
        systemPrintln("ESP_RST_BROWNOUT");
        break;
    case ESP_RST_SDIO:
        systemPrintln("ESP_RST_SDIO");
        break;
    default:
        systemPrintln("Unknown");
    }
}

void beginConsole(uint32_t baud, bool allowAlt)
{
    // Don't try to print here. serialConsole may not have been initialized

    static bool begun = false;

    if (begun)
        systemFlush();

    if ((!allowAlt) || (!settings.enableTCPServer))
    {
        serialConsole.begin(baud, SERIAL_8N1, pin_serial0RX, pin_serial0TX);
        begun = true;
    }
    else
    {
        if ((pin_serial0RX_Alt < 0) || (pin_serial0TX_Alt < 0))
        {
            reportFatalError("No Serial0 Alt pins");
        }
        else
        {
            serialConsole.begin(baud, SERIAL_8N1, pin_serial0RX_Alt, pin_serial0TX_Alt);
            begun = true;
        }
    }

    clearBuffer(); // Does a flush too
}

// We want the UART1 interrupts to be pinned to core 0 to avoid competing with I2C interrupts
// We do not start the UART1 here because the interrupts would be pinned to core 1
// We instead start a task that runs on core 0, that then begins serial
// See issue: https://github.com/espressif/arduino-esp32/issues/3386
void beginUARTs()
{
    if ((pin_serial1RX < 0) || (pin_serial1TX < 0))
        reportFatalError("No Serial2 pins");

    // Determine the length of data to be retained in the ring buffer
    // after discarding the oldest data
    size_t length = settings.gnssHandlerBufferSize;
    rbOffsetEntries = (length >> 1) / AVERAGE_SENTENCE_LENGTH_IN_BYTES;
    length = settings.gnssHandlerBufferSize + (rbOffsetEntries * sizeof(RING_BUFFER_OFFSET));
    ringBuffer = nullptr;
    rbOffsetArray = (RING_BUFFER_OFFSET *)malloc(length);
    if (!rbOffsetArray)
    {
        rbOffsetEntries = 0;
        systemPrintln("ERROR: Failed to allocate the ring buffer!");
    }
    else
    {
        ringBuffer = (uint8_t *)&rbOffsetArray[rbOffsetEntries];
        rbOffsetArray[0] = 0;
        if (pinUART1TaskHandle == nullptr)
            xTaskCreatePinnedToCore(
                pinUART1Task,
                "UART1Start", // Just for humans
                2000,        // Stack Size
                nullptr,     // Task input parameter
                0,           // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest
                &pinUART1TaskHandle,              // Task handle
                settings.gnssUartInterruptsCore); // Core where task should run, 0=core, 1=Arduino

        while (uart1pinned == false) // Wait for task to run once
            delay(1);
    }

    if ((pin_serial2RX < 0) || (pin_serial2TX < 0))
        reportFatalError("No Serial2 pins");

    serialGNSSConfig.begin(settings.dataPortBaud, SERIAL_8N1, pin_serial2RX, pin_serial2TX);
}

// Assign UART1 interrupts to the core that started the task. See:
// https://github.com/espressif/arduino-esp32/issues/3386
void pinUART1Task(void *pvParameters)
{
    serialGNSS.setRxBufferSize(settings.uartReceiveBufferSize);
    serialGNSS.setTimeout(settings.serialTimeoutGNSS); // Requires serial traffic on the UART pins for detection
    serialGNSS.begin(settings.dataPortBaud, SERIAL_8N1, pin_serial1RX, pin_serial1TX);
    serialGNSS.setRxFIFOFull(settings.serialGNSSRxFullThreshold);

    uart1pinned = true;

    vTaskDelete(nullptr); // Delete task once it has run once
}

void beginFS()
{
    if (online.fs == false)
    {
        if (LittleFS.begin(true) == false) // Format LittleFS if begin fails
        {
            systemPrintln("Error: LittleFS not online");
        }
        else
        {
            systemPrintln("LittleFS Started");
            online.fs = true;

            // Set the settingsFileName used many places
            setSettingsFileName();
        }
    }
}

// Set LEDs for output and configure PWM
void beginLEDs()
{
    if (productVariant == RTK_MOSAIC_T)
    {
        pinMode(pin_errorLED, OUTPUT);
        pinMode(pin_lockLED, OUTPUT);
        pinMode(pin_setupButton, INPUT_PULLUP);

        digitalWrite(pin_errorLED, HIGH);
        digitalWrite(pin_lockLED, LOW);
    }
}

void updateErrorLED()
{
    digitalWrite(pin_errorLED, (gnssError > 0) ? HIGH : LOW);
}

void updateLockLED()
{
    digitalWrite(pin_lockLED, (fabs(tcxoClockBias_ms) < settings.rxClkBiasLockLimit_ms) ? HIGH : LOW);
}

// Depending on platform and previous power down state, set system state
void beginSystemState()
{
    if (systemState > STATE_NOT_SET)
    {
        systemPrintln("Unknown state - factory reset");
        factoryReset(false); // We do not have the SD semaphore
    }

    if (productVariant == RTK_MOSAIC_T)
    {
        if (settings.lastState == STATE_NOT_SET) // Default after factory reset
            settings.lastState = STATE_GNSS_NOT_CONFIGURED;

        systemState = settings.lastState;

        setupBtn = new Button(pin_setupButton); // Create the button in memory
        // Allocation failure handled in ButtonCheckTask
    }

    // Starts task for monitoring button presses
    if (ButtonCheckTaskHandle == nullptr)
        xTaskCreate(ButtonCheckTask,
                    "BtnCheck",          // Just for humans
                    buttonTaskStackSize, // Stack Size
                    nullptr,             // Task input parameter
                    ButtonCheckTaskPriority,
                    &ButtonCheckTaskHandle); // Task handle
}

void beginIdleTasks()
{
    if (settings.enablePrintIdleTime == true)
    {
        char taskName[32];

        for (int index = 0; index < MAX_CPU_CORES; index++)
        {
            snprintf(taskName, sizeof(taskName), "IdleTask%d", index);
            if (idleTaskHandle[index] == nullptr)
                xTaskCreatePinnedToCore(
                    idleTask,
                    taskName, // Just for humans
                    2000,     // Stack Size
                    nullptr,  // Task input parameter
                    0,        // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest
                    &idleTaskHandle[index], // Task handle
                    index);                 // Core where task should run, 0=core, 1=Arduino
        }
    }
}

void beginI2C1()
{
    if ((pin_SDA1 >= 0) && (pin_SCL1 >= 0))
    {
        if (pinI2C1TaskHandle == nullptr)
            xTaskCreatePinnedToCore(
                pinI2C1Task,
                "I2C1Start",        // Just for humans
                2000,              // Stack Size
                nullptr,           // Task input parameter
                0,                 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest
                &pinI2C1TaskHandle, // Task handle
                settings.i2cInterruptsCore); // Core where task should run, 0=core, 1=Arduino

        while (i2c1Pinned == false) // Wait for task to run once
            delay(1);
    }
}

// Assign I2C interrupts to the core that started the task. See: https://github.com/espressif/arduino-esp32/issues/3386
void pinI2C1Task(void *pvParameters)
{
    bool i2cBusAvailable;
    uint32_t timer;

    if (i2c_1 == nullptr)
        i2c_1 = new TwoWire(0);

    i2c_1->begin(pin_SDA1, pin_SCL1); // SDA, SCL

    // Display the device addresses
    i2cBusAvailable = false;
    for (uint8_t addr = 0; addr < 127; addr++)
    {
        timer = millis();
        i2c_1->beginTransmission(addr);
        if (i2c_1->endTransmission() == 0)
        {
            i2cBusAvailable = true;
            switch (addr)
            {
                default: {
                    systemPrintf("0x%02x\r\n", addr);
                    break;
                }

                case 0x3d: {
                    systemPrintf("0x%02x - SSD1306 (64x48) OLED Driver\r\n", addr);
                    i2cDisplay = i2c_1;
                    break;
                }
            }
        }
        else if ((millis() - timer) > 3)
        {
            systemPrintln("Error: I2C Bus 1 Not Responding");
            i2cBusAvailable = false;
            break;
        }
    }

    // For code development only - TODO uncomment this
    //i2c_1->setClock(400000); // Set clock to 400kHz for OLED

    // Update the I2C status
    online.i2c1 = i2cBusAvailable;
    i2c1Pinned = true;
    vTaskDelete(nullptr); // Delete task once it has run once
}

void beginI2C2()
{
    if ((pin_SDA2 >= 0) && (pin_SCL2 >= 0))
    {
        if (pinI2C2TaskHandle == nullptr)
            xTaskCreatePinnedToCore(
                pinI2C2Task,
                "I2C2Start",        // Just for humans
                2000,              // Stack Size
                nullptr,           // Task input parameter
                0,                 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest
                &pinI2C2TaskHandle, // Task handle
                settings.i2cInterruptsCore); // Core where task should run, 0=core, 1=Arduino

        while (i2c2Pinned == false) // Wait for task to run once
            delay(1);
    }
}

// Assign I2C interrupts to the core that started the task. See: https://github.com/espressif/arduino-esp32/issues/3386
void pinI2C2Task(void *pvParameters)
{
    bool i2cBusAvailable;
    uint32_t timer;

    if (i2c_2 == nullptr)
        i2c_2 = new TwoWire(1);

    i2c_2->begin(pin_SDA2, pin_SCL2); // SDA, SCL

    // Display the device addresses
    i2cBusAvailable = false;
    for (uint8_t addr = 0; addr < 127; addr++)
    {
        timer = millis();
        i2c_2->beginTransmission(addr);
        if (i2c_2->endTransmission() == 0)
        {
            i2cBusAvailable = true;
            switch (addr)
            {
                default: {
                    systemPrintf("0x%02x\r\n", addr);
                    break;
                }

                case SFE_GNSSDO_OSC_SIT5811: {
                    systemPrintf("0x%02x - SiT5811 TCXO\r\n", addr);
                    presentSIT5811 = true;
                    i2cTCXO = i2c_2;
                    break;

                case SFE_GNSSDO_OSC_SIT5358: {
                    systemPrintf("0x%02x - SiT5358 TCXO\r\n", addr);
                    presentSIT5358 = true;
                    i2cTCXO = i2c_2;
                    break;

                case SFE_GNSSDO_OSC_STP3593LF: {
                    systemPrintf("0x%02x - STP3593LF TCXO\r\n", addr);
                    presentSTP3593LF = true;
                    i2cTCXO = i2c_2;
                    break;
                }
            }
        }
        else if ((millis() - timer) > 3)
        {
            systemPrintln("Error: I2C Bus 2 Not Responding");
            i2cBusAvailable = false;
            break;
        }
    }

    // Update the I2C status
    online.i2c2 = i2cBusAvailable;
    i2c2Pinned = true;
    vTaskDelete(nullptr); // Delete task once it has run once
}

void beginTCXO(TwoWire *i2cBus)
{
    if (i2cBus == nullptr)
        reportFatalError("Illegal TCXO i2cBus");

    if (!myTCXO.begin(*i2cBus, SFE_GNSSDO_OSC_SIT5358)) // Initialize the SiT5358
        return;

    myTCXO.setBaseFrequencyHz(10000000.0); // Pass the oscillator base frequency into the driver

    systemPrint("TCXO base frequency set to ");
    systemPrint(myTCXO.getBaseFrequencyHz());
    systemPrintln(" Hz");

    myTCXO.setPullRangeControl(SiT5358_PULL_RANGE_6ppm25); // Set the pull range control to 6.25ppm

    systemPrint("TCXO pull range control set to ");
    systemPrintln(myTCXO.getPullRangeControlText(myTCXO.getPullRangeControl()));

    myTCXO.setMaxFrequencyChangePPB(3.0); // Set the maximum frequency change in PPB

    systemPrint("TCXO maximum frequency change set to ");
    systemPrint(myTCXO.getMaxFrequencyChangePPB());
    systemPrintln(" PPB");

    myTCXO.setFrequencyControlWord(settings.tcxoControl);

    systemPrint("TCXO frequency control word set to ");
    systemPrintln(myTCXO.getFrequencyControlWord());

    systemPrint("TCXO frequency is ");
    systemPrint(myTCXO.getFrequencyHz(), 6);
    systemPrintln(" Hz");

    online.tcxo = true;
}

// This function updates the TCXO to discipline the frequency
void updateTCXO()
{
    if (online.tcxo)
    {
        myTCXO.setFrequencyByBiasMillis(tcxoClockBias_ms, settings.Pk, settings.Ik);
    }
}

// This function updates the tcxoClockBias_ms used to discipline the TCXO frequency
// updateTCXOClockBias is only called by STATE_GNSS_FINETIME when gnssPVTUpdated was true
// So we know that gnssClockBias_ms is valid
// Use gnssClockBias_ms as the default
// If we have non-composite GPS from FugroTimeOffset, use that - if enabled
// If we have non-composite Galileo from FugroTimeOffset, use that - if enabled
void updateTCXOClockBias()
{
    tcxoClockBias_ms = gnssClockBias_ms; // Default to the PVTGeodetic RxClkBias
    snprintf(rxClkBiasSource, sizeof(rxClkBiasSource), "PVT");

    if (settings.preferNonCompositeGPSBias || settings.preferNonCompositeGalileoBias) // These are mutex
    {
        uint8_t index = mosaicTimeSystemIndexFromName(settings.preferNonCompositeGPSBias ? "GPS" : "Galileo");
        if (fugroTimeSystems[index].updated) // If we have the preferred non-composite bias, use that
        {
            tcxoClockBias_ms = fugroTimeSystems[index].RxClkBias_ms;
            fugroTimeSystems[index].updated = false;
            snprintf(rxClkBiasSource, sizeof(rxClkBiasSource), fugroTimeSystems[index].name);
        }
    }
}
