/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

///////////////////////////////////////////////////////
///////// PARTIE 1 : clignotement de la LED //////////
///////////////////////////////////////////////////////
/*
#include "mbed.h"

DigitalOut myled(LED1);

int main()
{
    while (1) {
        myled.write(1);        
        printf("myled = %d \n\r", (uint8_t)myled);
        ThisThread::sleep_for(500);

        myled.write(0);  
        printf("myled = %d \n\r", myled.read());
        ThisThread::sleep_for(500);
    }
}
*/

///////////////////////////////////////////////////////
/////// PARTIE 2 : bouton, interuption et timer ///////
///////////////////////////////////////////////////////


#include "mbed.h"

InterruptIn button(BUTTON1);
DigitalOut led(LED1);

using namespace std::chrono;
Timer t;


void toggle_on()
{
    t.reset();
    t.start();
    led = 1;
}

void toggle_off()
{
    t.stop();
    led = 0;
}


int main()
{
    button.rise(&toggle_on);
    button.fall(&toggle_off); 
    
    while (1) {       
        ThisThread::sleep_for(250);

        printf("The time taken was %llu milliseconds\n", duration_cast<milliseconds>(t.elapsed_time()).count());
        //printf("Hello World! \n");
    }
}


///////////////////////////////////////////////////////
///////// PARTIE 3 : contrôle LED par Ticker /////////
///////////////////////////////////////////////////////