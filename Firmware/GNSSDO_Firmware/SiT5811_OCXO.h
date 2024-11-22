#ifndef __SFE_GNSSDO_SIT5811_TCXO__
#define __SFE_GNSSDO_SIT5811_TCXO__

#include <SparkFun_Toolkit.h> // Click here to get the library: http://librarymanager/All#SparkFun_Toolkit
#include <SparkFun_SiT5811.h> // Click here to get the library: http://librarymanager/All#SparkFun_SiT5811

class GNSSDO_SIT5811 : GNSSDO_TCXO
{
    private:
        SfeSiT5811ArdI2C * _SiT5811 = nullptr; // Don't instantiate until we know what oscillators are connected

    public:
        // Constructor
        GNSSDO_SIT5811() : GNSSDO_TCXO() {}

        // Begin the oscillator
        bool begin(TwoWire &wirePort, const uint8_t &address);

        // Get the frequency control word
        int64_t getFrequencyControlWord(void);

        // Set the frequency by bias millis
        bool setFrequencyByBiasMillis(double bias, double Pk, double Ik);

        // Save the frequency control word - if supported
        bool saveFrequencyControlValue(void);
};

#endif // /__SFE_GNSSDO_SIT5811_TCXO__