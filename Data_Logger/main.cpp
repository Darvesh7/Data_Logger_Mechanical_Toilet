#include "mbed.h"
#include "motor.h"
#include "QEI.h"
#include "PinDetect.h"
#include "states.h"
#include "FATFileSystem.h"
#include "SDBlockDevice.h"
#include "ds3231.h"
#include <stdio.h>
#include <errno.h>
#include <string>
using std::string;


#include "platform/mbed_retarget.h"


//////////////////////////
/////// Buttons ////////// 
/////////////////////////

Ds3231 rtc(PB_9, PB_8);
SDBlockDevice sd(SPI_MOSI, SPI_MISO, SPI_SCK, PA_9);
FATFileSystem fs("sd", &sd);

InterruptIn Flush_switch (PC_13);
PinDetect Reverse_switch (PA_0);
QEI encoder(PC_1,PC_0,NC,64);


time_t epoch_time;
LowPowerTimer t,t_Return;

//////////////////////////
//////// Variables ///////
/////////////////////////
#define distance 0.05

bool update_film_value = false;
bool update_rpm_value = false;
bool sdopened = false;

int flush_count = 0;
float speed = 0.0;


//////////////////////////
////////////RTOS ///////// 
/////////////////////////

Semaphore motorSema(1);
Semaphore loggerSema(1);
Semaphore timestampSema(1);

void SystemStates_thread(void const *name);
void motorDrive_thread(void const *name); 
void logger(void const *name);
void timestamp(void const *name);

Thread t1;
Thread t2;
Thread loggerThread;
Thread timestampThread;

EventFlags stateChanged;
EventQueue queue;
LowPowerTimeout  flush_end;

FILE * fd;


void start_flush(void);
void end_flush(void);
void start_returnFlush(void);
void end_returnFlush(void);
void Reverse_SW(void);


Serial pc(USBTX, USBRX);


void init_SD(void)
{
    pc.baud(115200);

    sd.init();
    fs.mount(&sd);

   
    fd = fopen("/sd/testdata.csv", "r+");
    if (fd != nullptr)
    {
        pc.printf("SD Mounted - Ready");

    }
    else
    {
        fd = fopen("/sd/testdata.csv", "w+");
        fclose(fd);
        pc.printf("File Closed\r\n");
        sd.deinit();
        fs.unmount();
        pc.printf("Creating new file - No existing file found\r\n");
    }
}

void setup()
{
    pc.printf("SystemCoreClock is %d Hz\r\n", SystemCoreClock);


    Reverse_SW();

    t1.start(callback(motorDrive_thread, (void *)"MotorDriveThread"));
    t2.start(callback(SystemStates_thread, (void *)"SystemStateThread"));
    loggerThread.start(callback(logger, (void *)"DataLoggerThread"));
    timestampThread.start(callback(timestamp,(void*)"TimeStampThread"));

    sysState_struct.sysMode = S_RUN;
  
}

void LowPowerConfiguration(void)
{
    RCC->AHBENR |= (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN |
                    RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOEEN |RCC_AHBENR_GPIOHEN);
                    
    GPIO_InitTypeDef GPIO_InitStruct;
    // All other ports are analog input mode
    GPIO_InitStruct.Pin = GPIO_PIN_All;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    //HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    //HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    
    RCC->AHBENR &= ~(RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIODEN| RCC_AHBENR_GPIOEEN | RCC_AHBENR_GPIOHEN);
}

int main()

{
    
    init_SD();
    setup(); 
    Flush_switch.fall(&start_flush);
    Flush_switch.rise(&end_flush);  


    while (true) 
    {
        if(update_film_value)
        {
            fprintf(fd, "Return Speed %s%f",",",speed);
            fprintf(fd, "\r\n");
            fflush(stdout);
            fprintf(fd, "\r\n");
            fclose(fd);
            sd.deinit();
            fs.unmount();
            sdopened = false;

            ThisThread::sleep_for(200); //important to have Delay - > Give SD time to flush data.
            update_film_value = false;
            LowPowerConfiguration();
       
            
            Ds3231 rtc(PB_9, PB_8);
            epoch_time = rtc.get_epoch();
            ThisThread::sleep_for(100);


            HAL_SuspendTick();
            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
            

            SetSysClock();

            HAL_ResumeTick();
        
            
            setup();       
            pc.printf("exiting sleep mode");
        
             
        }          
   
    }

}

void timestamp(void const *name)
{
    while(1)
    {
        timestampSema.acquire();
        epoch_time = rtc.get_epoch();
    }
}




void logger(void const *name)
{
    volatile int currenttime = 0;
    volatile int pulses = 0;
    volatile float rpm = 0.0;
    float RpmRatioConversion = ((600/32)*(1/149.25));
    epoch_time = rtc.get_epoch();   

    
    while(true)
    {
        
       
        if(loggerSema.try_acquire_until(20000))
        {
            timestampSema.release();

            if (!sdopened)
            {
                sd.init();
                fs.mount(&sd);
                fd = fopen("/sd/testdata.csv", "a");
                fflush(stdout);
                fprintf(fd, "%s\n", ctime(&epoch_time));
                sdopened = (fd != nullptr);
            }

           
            pulses = encoder.getPulses();
            currenttime = t.read_ms();
            rpm = (pulses*RpmRatioConversion);
    


            string logline;

            logline.append(to_string(flush_count) + ",");
            logline.append(to_string(pulses) + ",");
            logline.append(to_string(currenttime) + ",");
            logline.append(to_string(rpm)  + "\n");

            pc.printf("%s", logline.c_str());

            if (fd != nullptr)
            {
                fprintf(fd, " %s%s",",",logline.c_str());
            }

            encoder.reset();
            t.reset();
            ThisThread::sleep_for(50);
        
            if(gMotorAction == MA_Forward)
            {
                loggerSema.release();
            }
            else
            {
                if (fd != nullptr)
                {
                    
                }
            }
        }
    }
}


void start_flush(void)
{
   
    if(sysState_struct.sysMode == S_RUN)
    {
        if(gMotorAction == MA_Stop)
        {
            
            flush_count = flush_count + 1;
            gMotorAction = MA_Forward;
            t.start();
            motorSema.release();
            loggerSema.release();
            timestampSema.release();
            //flush_end.attach(&end_flush, 4.0); //timeout - duration of flush
       
        }
    }
}

void end_flush(void)
{

    if(gMotorAction == MA_Forward)
    {
        gMotorAction = MA_Stop;
        motorSema.release();
        t.stop(); 
        speed = distance/(t_Return.read()/1000.0);
        t_Return.reset();
       
        update_film_value = true;  
        flush_end.attach(&end_flush, 0.01); //timeout - duration of flush
    }
   
}



void end_returnFlush(void)
{
    t_Return.start();

}


void Reverse_SW(void)
{
    Reverse_switch.mode(PullUp);
    Reverse_switch.setAssertValue(0);
    Reverse_switch.attach_deasserted(&end_returnFlush);
    Reverse_switch.setSampleFrequency(5000);
}




void SystemStates_thread(void const *name) 
{
  while (true) 
  {
 
    switch (sysState_struct.sysMode) 
    {
    case S_RUN:
      sysState_struct.sysMode = S_RUN;
      
      
    break;

    default:
      printf("Error: Undefined Mode\n");
      break;
    }
  }
}


void motorDrive_thread(void const *name) 
{
    while (true) 
    {
        motorSema.acquire();
        switch(gMotorAction)
        {
            case MA_Forward:              
                
            break;
            case MA_Backward:
               
            break;
            case MA_Brake:
              
            break;
            case MA_Stop:
             

            break;
        }
    }
}