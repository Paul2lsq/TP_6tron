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

/*
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
*/

///////////////////////////////////////////////////////
///////// PARTIE 2 : contrôle LED par Ticker /////////
///////////////////////////////////////////////////////

/*
#include "mbed.h"

Ticker flipper;
DigitalOut led1(LED1);

void flip()
{
    led1 = !led1;
}

int main()
{
    led1 = 1;
    flipper.attach(&flip, 2.0); // 2 secondes

    while (1) {
        ThisThread::sleep_for(200);
    }
}
*/

///////////////////////////////////////////////////////
///////// PARTIE 2 : contrôle LED par Ticker /////////
///////// et changement fréquence par bouton /////////
///////////////////////////////////////////////////////

#include "mbed.h"

Ticker flipper;
DigitalOut led1(LED1);
InterruptIn button(BUTTON1);

int i = 0;
float freq[5] = {2.0, 1.5, 1.0, 0.5, 0.2};

void flip()
{
    led1 = !led1;
}

void change_interval()
{
    i = (i+1) % 5;
    flipper.detach();
    flipper.attach(&flip, freq[i]);    
}

int main()
{
    led1 = 1;
    flipper.attach(&flip, freq[i]);
    button.fall(&change_interval);

    while (1) {
        ThisThread::sleep_for(200);
    }
}

///////////////////////////////////////////////////////
///////// PARTIE 3 :                          /////////
///////////////////////////////////////////////////////