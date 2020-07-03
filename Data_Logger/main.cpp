#include "mbed.h"
#include "PinDetect.h"


DigitalOut myled (LED2);

#define ENC_PPR 1


PinDetect switchA (PA_0);
PinDetect switchB (PA_1);

int flushCount = 0;
int useCount = 0;
float speed = 0;
#define distance 0.3

int pulses = 0;


Timer switchA_Timer, switchB_Timer, uses_Timer;
Timeout Uses_Timeout;


void _switchA(void);
void _switchB(void);

bool switchA_Pressed = false;


void setup(void)
{
    _switchA();
    _switchB();



}


int main()
{
    setup();

    while(true)
    {

    sleep();    
    if (switchA_Pressed) {

    switchA_Pressed = false;
    }


    }
    

 
}

void stop_flush(void) {


  switchA_Pressed = true;

 
  Uses_Timeout.attach(&stop_flush, 0.1);
    
  
}





void _switchAPressed(void)
{
    switchA_Timer.start();
    uses_Timer.start();
    flushCount = flushCount +1;
 

    
    if(uses_Timer.read_ms() > 10000)
    {
        useCount = useCount + 1;
        uses_Timer.reset();
    }

    Uses_Timeout.attach(&stop_flush, 0.2);
    
 
}

void _switchAReleased(void)
{
    speed = (distance / switchA_Timer.read());
    switchA_Timer.reset();
}

void _switchBPressed(void)
{
    
}

void _switchBReleased(void)
{
    
}




void _switchA(void)
{
    switchA.mode(PullUp);
    switchA.setAssertValue(0);
    switchA.attach_asserted(&_switchAPressed);
    switchA.attach_deasserted(&_switchAReleased);
    switchA.setSampleFrequency();

}

void _switchB(void)
{
    switchB.mode(PullUp);
    switchB.setAssertValue(0);
    switchB.attach_asserted(&_switchBPressed);
    switchB.attach_deasserted(&_switchBReleased);
    switchB.setSampleFrequency();
}