// Class to contain all possible oscillators

#ifndef __SFE_GNSSDO_TCXO__
#define __SFE_GNSSDO_TCXO__

class GNSSDO_TCXO
{
    public:
        // Constructor
        GNSSDO_TCXO() {}

        // Begin the oscillator
        virtual bool begin(TwoWire &wirePort, const uint8_t &address);

        // Get the frequency control word
        // SiT5358 is int32_t (26-bit, signed)
        // SiT5811 is int64_t (39-bit, signed)
        // STP3593LF is uint32_t (20-bit, unsigned)
        // int64_t can contain all three
        virtual int64_t getFrequencyControlWord(void);

        // Set the frequency by bias millis
        virtual bool setFrequencyByBiasMillis(double bias, double Pk, double Ik);

        // Save the frequency control word - if supported
        virtual bool saveFrequencyControlValue(void);
};

#endif // /__SFE_GNSSDO_TCXO__