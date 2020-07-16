#include "mbed.h"
#include "FATFileSystem.h"
#include "SDBlockDevice.h"
#include "ds3231.h"
#include "RotaryEncoder.h"

#include <stdio.h>
#include <errno.h>
#include <string>
using std::string;


#include "platform/mbed_retarget.h"


#define distance 0.3


InterruptIn switchA (PC_13);
InterruptIn switchB (PA_1);
Ds3231 rtc(PB_9, PB_8);
SDBlockDevice sd(SPI_MOSI, SPI_MISO, SPI_SCK, PA_9);
FATFileSystem fs("sd", &sd);
RotaryEncoder encoder (PC_1,PC_0);

time_t epoch_time;
FILE * fd;


LowPowerTimer switchA_Timer;
LowPowerTimeout  flush_end;


Semaphore loggerSema(1);



void start_flush(void);
void end_flush(void);

bool switchA_Pressed = false;

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


void setup(void)
{


}


int main()
{
    init_SD();
    setup(); 
    switchA.fall(&start_flush);
    switchA.rise(&end_flush);
  

    while(true)
    {
        

    }
    
}


bool sdopened = false;

void logger(void const *name)
{
    volatile int currenttime = 0;
    volatile float voltage = 0.0;
    volatile float current = 0.0;
    volatile int pulses = 0;
    volatile float rpm = 0.0;
    float RpmRatioConversion = ((600/32)*(1/149.25));
    volatile float power = 0.0;
    volatile float torque = 0.0;


    
    while(true)
    {
        
        epoch_time = rtc.get_epoch();
        if(loggerSema.try_acquire_until(20000))
        {
            if (!sdopened)
            {
                sd.init();
                fs.mount(&sd);
                fd = fopen("/sd/testdata.csv", "a");
                fflush(stdout);
                fprintf(fd, "%s\n", ctime(&epoch_time));
                sdopened = (fd != nullptr);
            }




            string logline;

          
            //logline.append(to_string(voltage) + ",");
            //logline.append(to_string(current) + ",");
            //logline.append(to_string(power)  + ",");
           

            pc.printf("%s", logline.c_str());

            if (fd != nullptr)
            {
                
                fprintf(fd, " %s%s",",",logline.c_str());
    
            }

            encoder.reset();
            switchA_Timer.reset();
            ThisThread::sleep_for(100);
        
            if(motor._MState == MFORWARD)
            {
                loggerSema.release();
            }
            else
            {
                if (fd != nullptr)
                {
                    fflush(stdout);
                    fprintf(fd, "\r\n");
                    fclose(fd);
                    sd.deinit();
                    fs.unmount();
                    sdopened = false;
                }
            }
        }
    }
}




void start_flush(void)
{           
   
    switchA_Timer.start();
    loggerSema.release();
    flush_end.attach(&end_flush, 4.0); //timeout - duration of flush    
}

void end_flush(void)
{        
    switchA_Timer.stop(); 
       
    flush_end.attach(&end_flush, 0.1); //timeout - duration of flush
}
   





