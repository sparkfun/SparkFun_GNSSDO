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

// Set the frequency by bias millis
bool GNSSDO_STP3593LF::setFrequencyByBiasMillis(double bias, double Pk, double Ik)
{
    return _STP3593LF->setFrequencyByBiasMillis(bias, Pk, Ik);
}

// Save the frequency control word - if supported
bool GNSSDO_STP3593LF::saveFrequencyControlValue(void)
{
    return _STP3593LF->saveFrequencyControlValue();
}
