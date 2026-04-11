// Begin the oscillator
bool GNSSDO_STP3593LF::begin(TwoWire &wirePort, const uint8_t &address)
{
    _STP3593LF = new SfeSTP3593LFArdI2C();
    
    if (!_STP3593LF->begin(wirePort, address))
        return false;

    _STP3593LF->setMaxFrequencyChangePPB(3.0); // Set the maximum frequency change in PPB

    // Don't restore the frequency control word with the saved value from settings.
    // Assume saveFrequencyControlValue has done its job and that the control word
    // has already been restored by the OCXO.

    return true;
}

// Get the frequency control word
int64_t GNSSDO_STP3593LF::getFrequencyControlWord(void)
{
    return (int64_t)_STP3593LF->getFrequencyControlWord();
}

// Set the frequency control word
bool GNSSDO_STP3593LF::setFrequencyControlWord(int64_t controlWord)
{
    return _STP3593LF->setFrequencyControlWord((uint32_t)controlWord);
}

// Set the frequency by bias millis
bool GNSSDO_STP3593LF::setFrequencyByBiasMillis(double bias, double Pk, double Ik)
{
    //return _STP3593LF->setFrequencyByBiasMillis(bias, Pk, Ik);

    //systemPrintln("setFrequencyByBiasMillis:");

    //systemPrintf("bias %.3e Pk %.3e Ik %.3e\r\n", bias, Pk, Ik);

    const double kSfeSTP3593LFFreqControlResolution = 8e-13;

    static double I;
    static bool initialized = false;
    if (!initialized)
    {
        I = (double)getFrequencyControlWord(); // Initialize I with the current control word for a more reasonable startup
        initialized = true;
    }

    // Our setpoint is zero. Bias is the process value. Convert it to error
    double error = 0.0 - bias;

    // Convert error from millis to seconds
    error /= 1000.0;

    // Convert the error to control word LSBs
    double requiredChangeInLSBs = error / kSfeSTP3593LFFreqControlResolution;

    // Calculate the maximum change in control word LSBs
    double _maxFrequencyChangePPB = _STP3593LF->getMaxFrequencyChangePPB();
    double maxChangeInLSBs = _maxFrequencyChangePPB * 1.0e-9 / kSfeSTP3593LFFreqControlResolution;

    //systemPrintf("maxChangeInLSBs %.1f\r\n", maxChangeInLSBs);

    // Limit requiredChangeInLSBs to +/-maxChangeInLSBs
    if (requiredChangeInLSBs >= 0.0)
    {
        if (requiredChangeInLSBs > maxChangeInLSBs)
            requiredChangeInLSBs = maxChangeInLSBs;
    }
    else
    {
        if (requiredChangeInLSBs < (0.0 - maxChangeInLSBs))
            requiredChangeInLSBs = 0.0 - maxChangeInLSBs;
    }

    //systemPrintf("requiredChangeInLSBs %.1f\r\n", requiredChangeInLSBs);

    // Control      P       Ti      Td
    // P            0.5Kc   -       -
    // PI           0.45Kc  Pc/1.2  -
    // PI           0.60Kc  0.5Pc   Pc/8

    double P = requiredChangeInLSBs * Pk * 1.333;
    double dI = requiredChangeInLSBs * Ik * 0.6;
    static double previousChangeInLSBs = requiredChangeInLSBs;
    double Dk = Ik * 0.15;
    double D = Dk * (previousChangeInLSBs - requiredChangeInLSBs);
    I += dI; // Add the delta to the integral

    //systemPrintf("P %f I %f\r\n", P, I);
    //systemPrintf("round(P + I) %ld\r\n", (uint32_t)round(P + I));

    double newControlWordDbl = round(P + I + D); // Set the control word to proportional plus integral

    previousChangeInLSBs = requiredChangeInLSBs;
    
    // Ensure control word is within bounds. Bad things happen if it wraps below zero!
    uint32_t newControlWord;
    if (newControlWordDbl > (double)kSfeSTP3593LFFreqControlMaxValue)
        newControlWord = kSfeSTP3593LFFreqControlMaxValue;
    else if (newControlWordDbl < 0.0)
        newControlWord = 0;
    else
        newControlWord = (uint32_t)newControlWordDbl;

    return _STP3593LF->setFrequencyControlWord(newControlWord);
}

// Save the frequency control word - if supported
bool GNSSDO_STP3593LF::saveFrequencyControlValue(void)
{
    return _STP3593LF->saveFrequencyControlValue();
}

// Read the temperature sensor - if present
uint16_t GNSSDO_STP3593LF::getTcxoTemperature(void)
{
    // If this is useful, it should be included in the STP3593LF library

    // Read the temperature from register 0x3E
    // Represents an image of the external temperature seen by the NCO.
    // The value can vary from Ox0000 to Ox0FFF, negative slope.
    union
    {
        uint8_t bytes[2];
        uint16_t unsignedInt;
    } temperature;

    i2c_2->beginTransmission(0x70);
    i2c_2->write(0x3E);
    if (i2c_2->endTransmission() != 0)
        return 0;

    i2c_2->requestFrom(0x70, (uint8_t)2);
    if (i2c_2->available())
    {
        temperature.bytes[1] = i2c_2->read();
        temperature.bytes[0] = i2c_2->read();
    }

    return temperature.unsignedInt;
}
