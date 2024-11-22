// Begin the oscillator
bool GNSSDO_SIT5811::begin(TwoWire &wirePort, const uint8_t &address)
{
    _SiT5811 = new SfeSiT5811ArdI2C();
    
    if (!_SiT5811->begin(wirePort, address))
        return false;

    _SiT5811->setBaseFrequencyHz(10000000.0); // Pass the oscillator base frequency into the driver
    _SiT5811->setMaxFrequencyChangePPB(3.0); // Set the maximum frequency change in PPB
    _SiT5811->setFrequencyControlWord(settings.tcxoControl); // Restore the saved control word

    return true;
}

// Get the frequency control word
int64_t GNSSDO_SIT5811::getFrequencyControlWord(void)
{
    return (int64_t)_SiT5811->getFrequencyControlWord();
}

// Set the frequency by bias millis
bool GNSSDO_SIT5811::setFrequencyByBiasMillis(double bias, double Pk, double Ik)
{
    return _SiT5811->setFrequencyByBiasMillis(bias, Pk, Ik);
}

// Save the frequency control word - if supported
bool GNSSDO_SIT5811::saveFrequencyControlValue(void)
{
    return false;
}
