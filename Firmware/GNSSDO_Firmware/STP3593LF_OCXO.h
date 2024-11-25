#ifndef __SFE_GNSSDO_STP3593LF_TCXO__
#define __SFE_GNSSDO_STP3593LF_TCXO__

#include <SparkFun_Toolkit.h> // Click here to get the library: http://librarymanager/All#SparkFun_Toolkit
#include <SparkFun_STP3593LF.h> // Click here to get the library: http://librarymanager/All#SparkFun_STP3593LF

class GNSSDO_STP3593LF : GNSSDO_TCXO
{
    private:
        SfeSTP3593LFArdI2C * _STP3593LF = nullptr; // Don't instantiate until we know what oscillators are connected

    public:
        // Constructor
        GNSSDO_STP3593LF() : GNSSDO_TCXO() {}

        // Begin the oscillator
        bool begin(TwoWire &wirePort, const uint8_t &address);

        // Get the frequency control word
        int64_t getFrequencyControlWord(void);

        // Set the frequency by bias millis
        bool setFrequencyByBiasMillis(double bias, double Pk, double Ik);

        // Save the frequency control word - if supported
        bool saveFrequencyControlValue(void);

        // Get the default P and I terms - for the default settings
        double getDefaultFrequencyByBiasPTerm(void) { return (1.0 / 6.25); }
        double getDefaultFrequencyByBiasITerm(void) { return ((1.0 / 6.25) / 150.0); }
};

#endif // /__SFE_GNSSDO_STP3593LF_TCXO__