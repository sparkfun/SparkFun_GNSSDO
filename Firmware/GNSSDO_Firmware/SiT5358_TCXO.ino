// Begin the oscillator
bool GNSSDO_SIT5358::begin(TwoWire &wirePort, const uint8_t &address)
{
    _SiT5358 = new SfeSiT5358ArdI2C();
    
    if (!_SiT5358->begin(wirePort, address))
        return false;

    _SiT5358->setBaseFrequencyHz(10000000.0); // Pass the oscillator base frequency into the driver
    _SiT5358->setPullRangeControl(sfe_SiT5358_pull_range_control_t::SiT5358_PULL_RANGE_6ppm25); // Set the pull range control to 6.25ppm
    _SiT5358->setMaxFrequencyChangePPB(3.0); // Set the maximum frequency change in PPB
    _SiT5358->setFrequencyControlWord((int32_t)settings.tcxoControl); // Restore the saved control word

    return true;
}

// Get the frequency control word
int64_t GNSSDO_SIT5358::getFrequencyControlWord(void)
{
    return (int64_t)_SiT5358->getFrequencyControlWord();
}

// Set the frequency by bias millis
bool GNSSDO_SIT5358::setFrequencyByBiasMillis(double bias, double Pk, double Ik)
{
    return _SiT5358->setFrequencyByBiasMillis(bias, Pk, Ik);
}

// Save the frequency control word - if supported
bool GNSSDO_SIT5358::saveFrequencyControlValue(void)
{
    return false;
}
