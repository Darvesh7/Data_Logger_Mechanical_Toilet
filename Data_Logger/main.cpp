#include "mbed.h"
#include "PinDetect.h"
#include "RotaryEncoder.h"
#include <string>

//////////PINS//////////////

PinDetect plunger_switch (PA_0);
PinDetect encoder_switch (PA_1);
RotaryEncoder encoder(PC_1, PC_0);


/////////VARIABLES//////////
#define distance 0.3

int flushCount = 0;
int pulses = 0;
float reverse_speed = 0.0;

//////////RTOS///////////////
Timer ReverseSpeedTimer,FlushSpeedTimer;

Semaphore SealingUnitSema(1);
Semaphore loggerSema(1);

void SystemStates_thread(void const *name);
void SealingUnit_thread(void const *name); 
void logger(void const *name);

Thread t1, loggerThread;



////////////////////////////



void plungerSW(void);
void encoderSW(void);

bool switchA_Pressed = false;


/////////TYPEDEF//////////


typedef enum 
{ 
    SU_Forward, 
    SU_Backward,
    SU_Stop,
} MA_t;

MA_t SealingUnitAction = SU_Stop;

////////////////////////////


Serial pc(USBTX, USBRX);

void setup(void)
{
    
    pc.baud(115200);


    plungerSW();
    encoderSW();

    t1.start(callback(SealingUnit_thread, (void *)"SealingUnitThread"));
    
    loggerThread.start(callback(logger, (void *)"DataLoggerThread"));



}


int main()
{
    setup();

    while(true)
    {
     if(switchA_Pressed)
     {
        pc.printf("%2.2f\n", reverse_speed);
        switchA_Pressed = false;

     }   


    }
    
 
}

void logger(void const *name)
{
    volatile int currenttime = 0;
    volatile int reversetime = 0;
    volatile int pulses = 0;
    volatile float rpm = 0.0;
    float RpmRatioConversion = (600/24);
    


    
    while(true)
    {
        
       
        if(loggerSema.try_acquire_until(20000))
        {


           pulses = encoder.getPulses(); 
           currenttime = FlushSpeedTimer.read_ms();
           rpm = (pulses*RpmRatioConversion); 


            string logline;

            logline.append(to_string(flushCount) + ",");
            logline.append(to_string(pulses) + ",");
            logline.append(to_string(currenttime) + ",");
            logline.append(to_string(rpm)  + "\n");

            pc.printf("%s", logline.c_str());
      

            encoder.reset();
            FlushSpeedTimer.reset();
            ThisThread::sleep_for(100);
        
            if(SealingUnitAction == SU_Forward)
            {
                loggerSema.release();
            }
            else
            {

            }
        }
    }
}




void Forward_Button_Pressed(void)
{
    if(SealingUnitAction == SU_Stop)
    {
        SealingUnitAction = SU_Forward;
        FlushSpeedTimer.start();
        SealingUnitSema.release();
        loggerSema.release();
        flushCount = flushCount +1;  
    }
}


void Reverse_Button_Released(void)
{    
    ReverseSpeedTimer.start();
    
}


void Forward_Button_Released(void)
{
    if(SealingUnitAction == SU_Forward)
    {
        SealingUnitAction = SU_Stop;
        reverse_speed =  (distance / (ReverseSpeedTimer.read_ms()/1000));
        FlushSpeedTimer.stop();
        ReverseSpeedTimer.reset();
        switchA_Pressed = true;
    }
}



void encoderSW(void)
{
    encoder_switch.mode(PullUp);
    encoder_switch.setAssertValue(0);
    encoder_switch.attach_asserted(&Forward_Button_Pressed);
    encoder_switch.attach_deasserted(&Forward_Button_Released);
    encoder_switch.setSampleFrequency();

}

void plungerSW(void)
{
    plunger_switch.mode(PullUp);
    plunger_switch.setAssertValue(0);
    plunger_switch.attach_deasserted(&Reverse_Button_Released);
    plunger_switch.setSampleFrequency();
}



void SealingUnit_thread(void const *name) 
{
    while (true) 
    {
        SealingUnitSema.acquire();
        switch(SealingUnitAction)
        {
            case SU_Forward:   
            pc.printf("forward");           
                
            break;
            case SU_Backward:
               
            break;
            case SU_Stop:
               
            break;
        }
    }
}