// Class to contain all possible oscillators

// Begin the oscillator
bool GNSSDO_TCXO::begin(TwoWire &wirePort, const uint8_t &address)
{
    return true;
}

// Get the frequency control word
int64_t GNSSDO_TCXO::getFrequencyControlWord(void)
{
    return settings.tcxoControl;
}

// Set the frequency by bias millis
bool GNSSDO_TCXO::setFrequencyByBiasMillis(double bias, double Pk, double Ik)
{
    return true;
}

// Save the frequency control word - if supported
bool GNSSDO_TCXO::saveFrequencyControlValue(void)
{
    return true;
}

// Get the default P and I terms - for the default settings
double GNSSDO_TCXO::getDefaultFrequencyByBiasPTerm(void)
{
    return settings.Pk;
}
double GNSSDO_TCXO::getDefaultFrequencyByBiasITerm(void)
{
    return settings.Ik;
}
