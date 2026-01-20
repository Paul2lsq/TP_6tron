/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

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