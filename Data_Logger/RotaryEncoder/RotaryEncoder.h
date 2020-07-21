#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include "mbed.h"


class RotaryEncoder 
{
    public:

  
    RotaryEncoder(PinName channelA, PinName channelB, uint32_t val = 0);

    void reset(void);

    int getPulses() const {
        return val;
    }

    private:
    InterruptIn pin1;
    InterruptIn pin2;
    int val;
    LowPowerTicker ticker;

    /**
     * Internal tick function.
     */
    void func_ticker();
};

#endif
