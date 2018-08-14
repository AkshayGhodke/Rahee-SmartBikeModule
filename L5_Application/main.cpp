/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * @brief This is the application entry point.
 * 			FreeRTOS and stdio printf is pre-configured to use uart0_min.h before main() enters.
 * 			@see L0_LowLevel/lpc_sys.h if you wish to override printf/scanf functions.
 *
 */
#include <DFPlayer.h>
#include "tasks.hpp"
#include "LabGPIO.hpp"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart0_min.h"
#include "utilities.h"
#include "common_var.hpp"
#include "uart2.hpp"
#include "gpio.hpp"
#include "adc0.h"
#include "ssp1.h"

#include "NeoPixel.h"
#include "ADXL362.h"

#if 1

// Anti-theft

QueueHandle_t q;
TaskHandle_t xHandle = NULL;
TaskHandle_t xHandle1 = NULL;
TaskHandle_t xHandle2 = NULL;

uint8_t bike_lock = 0;

GPIO statePin(P1_20); // Bluetooth Module
GPIO led(P1_19); // Headlight


// Task Handles
TaskHandle_t alertTaskHandle;
TaskHandle_t directionTaskHandle;
TaskHandle_t toggleLampTaskHandle;
TaskHandle_t antiTheftTaskHandle;


// Variables

Uart3 &rx_uart = Uart3::getInstance();

char char_rx = '0';

void directionTask(void*)
{
    Uart2 &serial = Uart2::getInstance();
    serial.init(9600);

    while (1) {

        bike_lock = statePin.read();
        if (bike_lock == 1) {
            refresh();

            switch (char_rx) {
                case '0':
                    forward();
                    break;
                case '1':
                    backward();
                    break;
                case '2':
                    right();
                    break;
                case '3':
                    left();
                    break;
            }
            reset();
        }
        else {
            refresh();
            led.setLow();
            neoRing.setAsInput();
            vTaskResume(alertTaskHandle);
            vTaskSuspend(toggleLampTaskHandle);
            vTaskResume(antiTheftTaskHandle);
            vTaskSuspend(0);
        }
        vTaskDelay(1000);
    }
}
// NeoPixel Ring

// Anti-Theft

void alert(void *)
{

    Uart2 &serial = Uart2::getInstance();
    serial.init(9600);
    uint8_t rx = 0;
    // Module initialization
    execute_Command(serial, 0x0C, 0x00, 0x02, 0xEE);
    // Volume set to 30
    execute_Command(serial, 0x06, 0x00, 0x30, 0xEE);

    while (1) {
        if (xQueueReceive(q, &rx, portMAX_DELAY)) {
            execute_Command(serial, 0x03, 0x00, 0x01, Voice_Alert);
        }
        vTaskDelay(1000);
    }
}

void antiTheft(void *)  //LOW priority
{
    uint16_t X, Y, Z, X1, Y1, Z1, X2, Y2, Z2;

    X = accReadX();
    Y = accReadY();
    Z = accReadZ();
    uint8_t tx = 0;

    Uart2 &serial = Uart2::getInstance();
    serial.init(9600);

    while (1) {

        bike_lock = statePin.read();

        if (bike_lock == 1) {
            // Pausing the Alarm
            execute_Command(serial, 0x0E, 0x00, 0x00, 0x00);

            // Suspending and Resuming tasks
            neoRing.setAsOutput();
            vTaskResume(toggleLampTaskHandle);
            vTaskResume(directionTaskHandle);
        }
        if (bike_lock == 0) {
            refresh();
            led.setLow();
            neoRing.setAsInput();
            vTaskSuspend(toggleLampTaskHandle);
            vTaskSuspend(directionTaskHandle);

            // Activity recognition
            X1 = accReadX();
            Y1 = accReadY();
            Z1 = accReadZ();
            X2 = abs(X - X1);
            Y2 = abs(Y - Y1);
            Z2 = abs(Z - Z1);

            // Threshold for the alert is set to 32000, max accelerometer value being 65000

            if (Z2 >= 32000 || Y2 >= 32000 || X2 >= 32000) {
                X = accReadX();
                Y = accReadY();
                Z = accReadZ();
                delay_ms(100);

                xQueueSend(q, &tx, 0);
            }
        }
    }
    vTaskDelay(1000);
}

// Anti-theft


// Automatic headlamp toggling

void toggleLamp(void*)
{

    LPC_PINCON->PINSEL1 |= (1 << 20); // ADC-3 is on P0.26, select this as ADC0.3

    int lightVal = 0;

    while (1) {
        lightVal = adc0_get_reading(3);
        rx_uart.getChar(&char_rx, 0);

        if (lightVal < 500) {
            led.setHigh(); //Turn ON Led
        }
        else {
            led.setLow(); //Turn OFF Led
        }

        vTaskDelay(100);
    }

}

void initAcc()
{
    // SSP initialization
    ssp1_init();
    ssp1_set_max_clock(4);

    // Chip-select GPIO pin
    acc.setAsOutput();

    // Accelerometer initialization
    acc.setLow();
    ssp1_exchange_byte(0x0A);   // Command for write-register
    test = ssp1_exchange_byte(0x2D);    // Command for enabling measurement mode
    test1 = ssp1_exchange_byte(0x02);   // Value to set
    test2 = ssp1_exchange_byte(0x00);   // Dummy bits
    acc.setHigh();
}

void startUp()
{
    refresh();
    reset();
    begin();
    delay_ms(1000);
    reset();
    refresh();
}

int main()
{
    const int STACK_SIZE = 1024;

    Uart2 &serial = Uart2::getInstance();
    serial.init(9600);

    rx_uart.init(38400);

    q = xQueueCreate(1, sizeof(uint8_t));

    neoRing.setAsOutput();
    led.setAsOutput();
    initAcc();  // Accelerator initialization for measure mode

    startUp();  // Initial Neopixel Startup

    xTaskCreate(alert, "alert", STACK_SIZE, NULL, 2, &alertTaskHandle);
    xTaskCreate(directionTask, "directionTask", STACK_SIZE, NULL, 4, &directionTaskHandle);
    xTaskCreate(toggleLamp, "toggleLamps", STACK_SIZE, NULL, 3, &toggleLampTaskHandle);
    xTaskCreate(antiTheft, "antiTheft", STACK_SIZE, NULL, 1, &antiTheftTaskHandle);

    vTaskStartScheduler();

    return 0;
}
#endif
