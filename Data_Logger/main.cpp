// Depending on the LED connections either the LED is off the 2 seconds
// the target spends in deepsleep(), and on for the other second. Or it is inverted 
 
#include "mbed.h"

DigitalOut myled(LED2);
InterruptIn button(USER_BUTTON);
 

 
int main() {

printf("HELLO WORLD");

    while(1) {

        myled = !myled;
        wait(.2);
    }
}



