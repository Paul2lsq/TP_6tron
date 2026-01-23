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
/*

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

/*
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
*/

///////////////////////////////////////////////////////
////// PARTIE 3 : Température, humidité, pression /////
///////////////////////////////////////////////////////
/*
#include "mbed.h"
#include "bme280.h"
using namespace sixtron;

I2C i2c(I2C1_SDA, I2C1_SCL);
BME280 sensor(&i2c);

int main() {
    sensor.initialize();

    sensor.set_sampling();

    while (1) {

    bme280_environment_t env_data;

    sensor.read_env_data(env_data);

    printf("Temperature: %.2f 0 C | ", env_data.temperature);
    printf("Humidity: %.2f %% | ", env_data.humidity);
    printf("Pressure: %.2f hPa\n", env_data.pressure);

    ThisThread::sleep_for(1000ms);
    }
}
*/

///////////////////////////////////////////////////////
//////////////// PARTIE 4 : Ping Pong /////////////////
///////////////////////////////////////////////////////
/*
#include "mbed.h"

DigitalOut led1(LED1);

Mutex stdio_mutex;

Thread thread_ping;
Thread thread_pong;

void ping()
{
    int cpt = 0;
    while (cpt < 100) {
        stdio_mutex.lock();
        printf("PUFF ");
        cpt ++;
        stdio_mutex.unlock();
    }
}

void pong()
{
    int cpt = 0;
    while (cpt < 100) {
        stdio_mutex.lock();
        printf("PAFF\n");
        cpt ++;
        stdio_mutex.unlock();
    }
}

int main()
{
    thread_ping.start(ping);
    thread_pong.start(pong);
    
    while (true) {
        stdio_mutex.lock();
        led1 = !led1;
        printf("Alive!\n");
        ThisThread::sleep_for(200);
        stdio_mutex.unlock();
    }
}
*/

///////////////////////////////////////////////////////
////////// PARTIE 4 : Tâches station météo ////////////
///////////////////////////////////////////////////////

#include "mbed.h"
#include "bme280.h"
using namespace sixtron;

I2C i2c(I2C1_SDA, I2C1_SCL);
BME280 sensor(&i2c);

DigitalOut led1(LED1);
InterruptIn button(BUTTON1);

Mutex stdio_mutex;

EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread queue_thread;

Ticker tick_temp_hum;
Ticker tick_led;

void task_temp_hum()
{
    bme280_environment_t env;
    sensor.read_env_data(env);

    stdio_mutex.lock();
    printf("Temperature: %.2f C | Humidity: %.2f %%\n", env.temperature, env.humidity);
    stdio_mutex.unlock();
}

void task_pressure()
{
    bme280_environment_t env;
    sensor.read_env_data(env);

    stdio_mutex.lock();
    printf("Pressure: %.2f hPa\n", env.pressure / 100.0f);
    stdio_mutex.unlock();
}

void task_toggle_led()
{
    led1 = !led1;
}

void on_tick_temp_hum()
{
    queue.call(task_temp_hum);
}

void on_tick_led()
{
    queue.call(task_toggle_led);
}

void on_button_rise()
{
    queue.call(task_pressure);
}

int main()
{
    sensor.initialize();
    sensor.set_sampling(); 
    queue_thread.start(callback(&queue, &EventQueue::dispatch_forever));

    button.rise(on_button_rise);

    tick_temp_hum.attach(on_tick_temp_hum, 2s);
    tick_led.attach(on_tick_led, 5s);

    while (true) {
        ThisThread::sleep_for(1s);
    }
}

