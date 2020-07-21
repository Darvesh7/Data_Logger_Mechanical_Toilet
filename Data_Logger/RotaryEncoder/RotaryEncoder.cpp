#include "mbed.h"
#include "RotaryEncoder.h"


RotaryEncoder::RotaryEncoder(PinName channelA, PinName channelB, uint32_t val): 



pin1(channelA), pin2(channelB), val(val) 
{
    pin1.mode(PullUp);
    pin2.mode(PullUp);
    ticker.attach_us(this, &RotaryEncoder::func_ticker, 10000);
}

void RotaryEncoder::reset(void) 
{
    val = 0;
}



void RotaryEncoder::func_ticker() 
{
    static uint8_t code;

    code = (code << 2) + (((pin1.read() << 1) | (pin2.read() << 0)) & 3);
    code &= 15; //rotary encoder has 15 Decimal code

    switch (code) 
    {
    case 0xd:
        if (val >= 0) 
        {
            val++;
        
        } 
        else 
        {
               
        }
        break;
    }
}
