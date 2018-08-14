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

#if 1
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




#if 0
#include "tasks.hpp"
#include "examples/examples.hpp"
#include "io.hpp"

#include "LabGPIO.hpp"
#include "LabGPIOInterrupts.hpp"
#include "LabSPI.hpp"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart0_min.h"
#include "printf_lib.h"
#include "utilities.h"
#include "pwmDriver.hpp"
#include "ADCDriver.hpp"
#include "LabUart.hpp"

#include "handlers.hpp"
#include "i2c2_device.hpp"
#include "temperature_sensor.hpp"

#include "command_handler.hpp"

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include "common_var.hpp"
#include "scheduler_task.hpp"

#include <stdint.h>

#include <stdio.h>
#include "i2c2.hpp"
#include "lpc_pwm.hpp"

#include "event_groups.h"
#include "storage.hpp"
#include <string.h>
#include "ff.h"

#include "DACDriver.hpp"

void task1(void *){
    while(1)
        {
            uart0_puts("aaaaaaaaaaaaaaaaaaaa");
            //printf("aaaaaaaaaaaaaaaaaaaa");
            vTaskDelay(100); // This sleeps the task for 100ms (because 1 RTOS tick = 1 millisecond)
        }

}

void task2(void *){
    while(1)
        {
            uart0_puts("bbbbbbbbbbbbbbbbbbbb");
            //printf("bbbbbbbbbbbbbbbbbbbb");
            vTaskDelay(100); // This sleeps the task for 100ms (because 1 RTOS tick = 1 millisecond)
        }
}


int main(){

    const int STACK_SIZE = 1024;

    xTaskCreate(task1, "t1", STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(task2, "t2", STACK_SIZE, NULL, 1, NULL);


    vTaskStartScheduler();
    return 0;

}

#endif




#if 0

/*Print and observe the structure (one printout with packed, and one without packed).
 *Also try allocating an array of the structure
 *Also and print the addresses of two struct array members.*/

/* Ignoring the comments given in the assignments in front of every         struct member.
 * Performing according to the comments would give the same result for packed
 * and without packed structure. This is because, if we define the struct members
 * as per the comments, it would be properly aligned in the 32 bit processor.
 * As per the comments
 typedef struct {
    float f1; // 4
    char c1[4]; // 4
    float f2; // 4
    char c2; // 1
    char c3[3]; // 3
    float f3; // 4
 } my_s1; and for }__attribute__((packed)) my_s;
    The data members will be stored as follows in the memory in both cases(packed and unpacked)
     *
     * f1 f1 f1 f1 c1 c1 c1 c1 f2 f2 f2 f2 c2 c3 c3 c3 f3 f3 f3 f3
     * 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19
     *
     * This obeys the rule of alignment which says :
     * 1 byte - the address at which it is stored should be a multiple of 1 byte
     * 2 byte - the address at which it is stored should be a multiple of 2 byte
     * 4 byte - the address at which it is stored should be a multiple of 4 byte
     * 8 byte - the address at which it is stored should be a multiple of 8 byte
 */



typedef struct {
float f1;
char c1;
float f2;
char c2;
char c3;
float f3;
}__attribute__((packed)) my_s;

typedef struct {
float f1;
char c1;
float f2;
char c2;
char c3;
float f3;
} my_s1;

int main(){
    my_s arr[10];
    my_s1 arr1[10];


    printf("Size of struct my_s with packed = %d\n", sizeof(my_s));
    printf("Size of struct my_s1 without packed = %d\n", sizeof(my_s1));

    /* Observation EXPLANATION
     * In a 32 bit processor, the processor reads data 32 bits, i.e, 4 bytes at a time.
     * So in structure padding is done for process optimization
     * Eg 1. consider
     *
     *  typedef struct {
     *  float f1;
        char c1;
        float f2;
        char c2;
        char c3;
        float f3;
     *  }my_s1;
     *
     * The data members will be stored as follows in the memory
     *
     * f1 f1 f1 f1 c1 - - - f2 f2 f2 f2 c2 c3  -  - f3 f3 f3 f3
     * 0  1  2  3  4  5 6 7 8  9  10 11 12 13 14 15 16 17 18 19
     *
     * Therefore, the size of the struct without packed (with padding) comes out to be 20
     *
     * Eg 2. consider
     *
     *  typedef struct {
     *  float f1;
        char c1;
        float f2;
        char c2;
        char c3;
        float f3;
     *  }__attribute__((packed)) my_s;
     *
     * The data members will be stored as follows in the memory
     *
     * f1 f1 f1 f1 c1 f2 f2 f2 f2 c2 c3 f3 f3 f3 f3
     * 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
     *
     * Since we have used __attribute__((packed)), it tells compiler to disable padding and
     * store the struct in a packed allignment.
     *
     * Therefore, the size of packed comes out to be 15
     * */





    printf("Address of struct my_s array member 1 : %x\n", &arr[0].f1);
    printf("Address of struct my_s array member 2 : %x\n", &arr[0].c2);

    printf("Address of struct my_s1 array member 1 : %x\n", &arr1[0].f1);
    printf("Address of struct my_s1 array member 2 : %x\n", &arr1[0].c2);

    /* As we can seen (in the output screenshot) we get different difference in the memory address in both the scenario
     * i.e, for packed and unpacked
     *
     * Address calculation for packed
     *
     *      Address of struct my_s array member 1 : 20083e78
            Address of struct my_s array member 2 : 20083e81
            difference = 9 bytes
     *
     *
     * Address calculation for unpacked
     *
     *      Address of struct my_s1 array member 1 : 20083f10
            Address of struct my_s1 array member 2 : 20083f1c
     *      difference = 12 bytes
     *
     *      As we can tally the result from the above OBSERVATION performed for bothe cases
     * */


    return 0;
}


#endif



#if 0

void ReadWAV(void *){
    while(1){
        /* Do Nothing */
    }

}

void digitalToAnalog(void *){
    DACDriver D;

    while(1){
        D.write(0x67);
    }
    vTaskDelay(1);

}

int main(){

    const int STACK_SIZE = 1024;

    // Read a .wav file from SDD

    // Convert the digital data to analog

    DACDriver D;

    D.init();

    D.write(0x67);

    xTaskCreate(digitalToAnalog, "digitalToAnalog", STACK_SIZE, NULL, 2, NULL);
    //xTaskCreate(ReadWAV, "ReadWAV", STACK_SIZE, NULL, 2, &xHandle1);

    // Use the analog output to drive the speaker

    //scheduler_add_task(new terminalTask(4));

    /*queue = xQueueCreate(1,sizeof(float));
    xCreatedEventGroup = xEventGroupCreate();

    xTaskCreate(producerLab, "producer", STACK_SIZE, NULL, 2, &xHandle1);
    xTaskCreate(consumerLab, "consumer", STACK_SIZE, NULL, 2, &xHandle2);
    xTaskCreate(watchdog_task, "watch_dog_task", STACK_SIZE, NULL, 3, NULL);*/

    //scheduler_start();

    vTaskStartScheduler();

    return 0;
}

#endif


#if 0

#define BIT_0 (1<<0)
#define BIT_1 (1<<1)

TaskHandle_t xHandle1;
TaskHandle_t xHandle2;

EventGroupHandle_t xCreatedEventGroup;

EventBits_t uxBits;

QueueHandle_t queue;


void cpuUsage()

{
    FILE *fd = fopen("1:cpu.txt" ,"r");

    Storage::write("1:cpu.txt", (void*)"\n", 10, 0);




    const unsigned portBASE_TYPE maxTasks = 16;

    TaskStatus_t status[maxTasks];

    uint32_t totalRunTime = 0;

    uint32_t tasksRunTime = 0;

    const unsigned portBASE_TYPE uxArraySize = uxTaskGetSystemState(&status[0], maxTasks, &totalRunTime);

    char buffer[10] = {0};

    for (unsigned priorityNum = 0; priorityNum < configMAX_PRIORITIES; priorityNum++) {

        /* Print in sorted priority order */

        for (unsigned i = 0; i < uxArraySize; i++) {

            TaskStatus_t *e = &status[i];

            if (e->uxBasePriority == priorityNum) {

                tasksRunTime += e->ulRunTimeCounter;



                const uint32_t cpuPercent = (0 == totalRunTime) ? 0 : e->ulRunTimeCounter / (totalRunTime / 100);
                printf("cpu usage  :   %lu \n",cpuPercent);
                sprintf(buffer, "%lu\n",cpuPercent);
                Storage::append("1:cpu.txt", buffer, sizeof(uint32_t), 0);
            }

        }



    }

    }

void producerLab(void *){
    int count = 0;
    float avg = 0;
while(1){
    if( count == 100){
        u0_dbg_printf("send \n");
        avg = avg / count;
        xQueueSend(queue, &avg, 0);
        count = 0;
    }
    else
    {
        avg = avg + LS.getRawValue();
        count++;
    }
    uxBits = xEventGroupSetBits(xCreatedEventGroup, 1);
    vTaskDelay(1);
}
}


void consumerLab(void *){
    float rx_avg = 0;
    char my_buffer[30];
    int count = 0;
    int count1 = 0;
    while(1){
        //u0_dbg_printf("receive \n");
        if(xQueueReceive(queue, &rx_avg, portMAX_DELAY)){
            u0_dbg_printf("count : %d , rx value : %f \n", count, rx_avg);
            //count1 += sprintf(my_buffer,"%d %f ", rx_avg);
            count++;
            if(count % 10 == 0){
                count1 += sprintf(my_buffer,"%d, %f\n", count1, rx_avg);
                Storage::append("1:sensor.txt", &my_buffer, sizeof(my_buffer[499]), 0);
                count = 0;
            }
            //vTaskSuspend(xHandle2);
            //uxBits = xEventGroupSetBits(xCreatedEventGroup, BIT_1);
        }
        uxBits = xEventGroupSetBits(xCreatedEventGroup, 2);
        u0_dbg_printf("receive \n");
    }
}

void watchdog_task (void *) {
    FILE *fd = fopen("1:stuck.txt" ,"r");

        Storage::write("1:stuck.txt", (void*)"Stuck\n", 10, 0);

    while(1) {

        uxBits = xEventGroupWaitBits(xCreatedEventGroup, 0x03, pdTRUE, pdTRUE, 1000);
        u0_dbg_printf("uxBits %lu\n",uxBits);

        if( (uxBits & (3<<0)) == (3<<0) ) {
            cpuUsage();
            u0_dbg_printf("Normal\n");
            xEventGroupClearBits(xCreatedEventGroup, (BIT_0|BIT_1));

        }
        else if((uxBits & BIT_0) == 1) {
            cpuUsage();
            u0_dbg_printf("Task2 Stuck\n");
            const char* buf1 = "Task2 Stuck\n";
            Storage::append("1:stuck.txt", (void*)buf1, 50, 0);
            xEventGroupClearBits(xCreatedEventGroup, (BIT_0));
        }
        else if((uxBits & BIT_1) == 2) {
            cpuUsage();
            u0_dbg_printf("Task1 Stuck\n");
            const char* buf2 = "Task1 Stuck\n";
            Storage::append("1:stuck.txt", (void*)buf2, 50, 0);
            xEventGroupClearBits(xCreatedEventGroup, (BIT_1));
        }

        else
        {
            cpuUsage();
            u0_dbg_printf("Both Stuck\n");
            const char* buf3 = "Both Stuck\n";
            Storage::append("1:stuck.txt", (void*)buf3, 50, 0);
        }
    }

}





int main(){

    const int STACK_SIZE = 1024;

    scheduler_add_task(new terminalTask(4));

    queue = xQueueCreate(1,sizeof(float));
    xCreatedEventGroup = xEventGroupCreate();

    xTaskCreate(producerLab, "producer", STACK_SIZE, NULL, 2, &xHandle1);
    xTaskCreate(consumerLab, "consumer", STACK_SIZE, NULL, 2, &xHandle2);
    xTaskCreate(watchdog_task, "watch_dog_task", STACK_SIZE, NULL, 3, NULL);

    scheduler_start();
    vTaskStartScheduler();

    return 0;
}

#endif


#if 0
// TODO: Modify the handleInterrupt at the I2C base class
// 1. Add your slave init method
// 2. Add the "slave address recognized" state in your I2C slave driver and print a msg when you hit this state inside the ISR
// 3. To test that your slave driver init is working, invoke "i2c discover" on the Master board

// Slave Board sample code reference
int main(void)
{

    scheduler_add_task(new terminalTask(3));
    I2C2& i2c = I2C2::getInstance(); // Get I2C driver instance
    const uint8_t slaveAddr = 0xD0;  // Pick any address other than an existing one at i2c2.hpp
    volatile uint8_t buffer[256] = { 0 }; // Our slave read/write buffer (This is the memory your other master board will read/write)

    i2c.initSlave(slaveAddr, &buffer[0], sizeof(buffer));

    // ie: If buffer[0] == 0, then LED ON, else LED OFF
    uint8_t prev = buffer[0];
    uint8_t prev1 = buffer[1];
    uint8_t prev2 = buffer[2];

    while (1)
    {
        if (prev != buffer[0] || prev1 != buffer[1] || prev2 != buffer[2])
        {
            prev = buffer[0];
            prev1 = buffer[1];
            prev2 = buffer[2];

            switch (buffer[0])
            {
                uint8_t temp;

            case 0x01:

                    temp = buffer[1] + buffer[2];
                    LD.setNumber(temp);
                    break;

                case 0x02:

                    if(buffer[1] >= buffer[2]) {

                        temp = buffer[1] - buffer[2];

                    }
                    else {

                        temp = buffer[2] - buffer[1];
                    }
                    LD.setNumber(temp);
                    break;

                default:
                    break;

            }
        }

    }

    scheduler_start();
    return 0;
}
#endif

#if 0
QueueHandle_t q;
TaskHandle_t xHandle = NULL;

typedef enum {
    invalid = 0,
    up = 1,
    down = 2,
    left = 3,
    right = 4
} orientation_t;

void rProducer(void *p) /* LOW priority */
{
    float X = AS.getX();
    float Y = AS.getY();
    float Z = AS.getZ();
    float X1;
    float Y1;
    float Z1;
    float X2;
    float Y2;
    float Z2;

    uint32_t block_time = 0;

    while (1) {
        // This xQueueSend() will internally switch context over to the "consumer" task
        // because it is higher priority than this "producer" task
        // Then, when the consumer task sleeps, we will resume out of xQueueSend()
        // and go over to the next line
        //orientation_t w;

        X1 = AS.getX();
        Y1 = AS.getY();
        Z1 = AS.getZ();

        orientation_t w;

        w = invalid;
        X2 = abs(X - X1);
        Y2 = abs(Y - Y1);
        Z2 = abs(Z - Z1);

        if(Z1 <= 0) {
            w = down;
            //    u0_dbg_printf(" Orientation w : %f \n", w);
            u0_dbg_printf(" Before sending\n");
            xQueueSend(q, &w, block_time);
            u0_dbg_printf(" After sending\n");
        }
        else {
            w = up;
            //            u0_dbg_printf(" Orientation w : %f \n", w);
            u0_dbg_printf(" Before sending\n");
            xQueueSend(q, &w, block_time);
            u0_dbg_printf(" After sending\n");
        }
        if(X2 > 15 && X1 <= 0) {
            if(w == up)
            w = right;
            else
            w = left;
            u0_dbg_printf(" Before sending\n");
            xQueueSend(q, &w, block_time);
            u0_dbg_printf(" After sending\n");
        }
        else if(X2 > 15 && X1 > 0) {
            if(w == up)
            w = left;
            else
            w = right;
            u0_dbg_printf(" Before sending \n");
            xQueueSend(q, &w, block_time);
            u0_dbg_printf(" After sending\n");
        }
        /*if(Y2 > 25 && (Y > Y1)){
         w = right;
         //u0_dbg_printf(" Orientation w : %f \n", w);
         xQueueSend(q, &w, 0);
         }
         else if(Y2 > 25 && (Y < Y1)){
         w = left;
         //u0_dbg_printf(" Orientation w : %f \n", w);
         xQueueSend(q, &w, 0);
         }*/

        //u0_dbg_printf(" No Value X = %f  X1 = %f \n", X, X1 );
        //u0_dbg_printf(" No Value Y = %f  Y1 = %f \n", Y, Y1 );
        //u0_dbg_printf(" No Value Z = %f  Z1 = %f \n", Z, Z1 );
        //u0_dbg_printf(" Orientation X : %f Y : %f Z : %f\n", X, Y, Z);
        //u0_dbg_printf(" Orientation Z : %f \n", Z1);

        vTaskDelay(1000);
    }
}

void rConsumer(void *p) /* HIGH priority */
{
    puts("Starting....Consumer");
    orientation_t x = invalid;
    while (1) {
        if(xQueueReceive(q, &x, portMAX_DELAY)){
        switch (x) {
            case 0:
                u0_dbg_printf(">>>>>>>>>>>>>>>>> Invalid orientation \n");
                break;
            case 1:
                u0_dbg_printf(">>>>>>>>>>>>>>>>> up \n");
                break;
            case 2:
                u0_dbg_printf(">>>>>>>>>>>>>>>>> down \n");
                break;
            case 3:
                u0_dbg_printf(">>>>>>>>>>>>>>>>> left \n");
                break;
            case 4:
                u0_dbg_printf(">>>>>>>>>>>>>>>>> right \n");
                break;

        }
        u0_dbg_printf(" After receiving \n");
        }
        else{
            u0_dbg_printf(" Failed to receive item from the queue \n");
        }
    }
}

int main(void)
{
    const int STACK_SIZE = 1024;
    //while(1){
    puts("Starting....");
    //}
    scheduler_add_task(new terminalTask(3));
    // Queue handle is not valid until you create it
    q = xQueueCreate(10, sizeof(int));
    puts("Starting....");
    //u0_dbg_printf("after receiving %d \n",X );

    xTaskCreate(rConsumer, "consumer", STACK_SIZE, (void *) 1, 1, NULL);
    xTaskCreate(rProducer, "producer", STACK_SIZE, (void *) 1, 2, &xHandle);


    /*while(1){
        TS.getCelsius();
    TS.readReg(0x00);
    TS.writeReg(0x02, 0x56);
    TS.readReg(0x02);
    }
*/
    scheduler_start();
    vTaskStartScheduler();
    return 0;
}

#endif

#if 0
QueueHandle_t q;
char arr[3] = {0};

#if 0
void uart_rx_isr(void)
{
    xQueueSendFromISR(q, &x, NULL); // TODO: Find out the significance of the parameters
}

void queue_rx_task(void *p)
{
    int x;

    // Receive is the usual receive because we are not inside an ISR
    while (1) {
        xQueueReceive(q, &x, portMAX_DELAY);
    }
}

void my_uart2_rx_intr(void)
{
    // TODO: Queue your data and clear UART Rx interrupt
}

void init_my_uart2(void)
{
    // Init PINSEL, baud rate, frame size, etc.

    // Init UART Rx interrupt (TX interrupt is optional)
    isr_register(Uart2, my_uart2_rx_intr);
}

void my_task(void *p)
{
    while (1) {
        if (xQueueReceive(..., portMAX_DELAY)) {
            printf("Got %c char from my UART... job is half done!");
        }
    }
}
#endif

void vReadSwitch(void * pvParameters)
{

    int count = 0;
    int i = 0;

    while (1) {
        printf("switch value : %u \n", SW.getSwitchValues());
        if( i < 2 ) {
            switch(SW.getSwitchValues()) {
                case 1: count++;
                LD.setNumber(char(count));
                break;
                case 2:
                i++;
                if(i == 1) {
                    arr[0] = count;
                    arr[1] = '+';
                    count = 0;
                }
                else {
                    arr[2] = count;
                    count = 0;
                }
                break;
                case 4:
                i++;
                if(i == 1) {
                    arr[0] = count;
                    arr[1] = '-';
                    count = 0;
                }
                else {
                    arr[2] = count;
                    count = 0;
                }
                break;
                case 8:
                i++;
                if(i == 1) {
                    arr[0] = count;
                    arr[1] = '*';
                    count = 0;
                }
                else {
                    arr[2] = count;
                    count = 0;
                }
                break;
                default:break;
            }
        }
        else {
            i=0;
            LabUART tx1;
            printf("sent data 1: %u \n",arr[0]);
            printf("sent data 2: %u \n",arr[1]);
            printf("sent data 3: %u \n",arr[2]);
            tx1.transmit(arr[0]);
            tx1.transmit(arr[2]);
            tx1.transmit(arr[1]);
        }
        vTaskDelay(200);
    }
}

void rx_isr() {
    LabUART rx1;
    uint8_t rx = rx1.receive();
    xQueueSendFromISR(q, &rx, NULL);
}

void tx_task(void *) {
    LabUART tx1;
    //uint8_t data = 5;
    while(1) {
        tx1.transmit(5);
        tx1.transmit(6);
        tx1.transmit('+');
        vTaskDelay(1000);
    }
}

void rx_task(void *) {
    LabUART rx1;
    char rx = 0;

    while(1) {
        if(xQueueReceive(q, &rx, 1000)) {

            printf("received data : %u \n",rx);
            LD.setNumber(rx);
            //LPC_UART2->FCR |= ( 1 << 1 );
        }
        //isr_register(UART2_IRQn, rx_isr);

    }
    //printf("iteration : \n");
    vTaskDelay(1);
    //}
}

int main(void)
{
    const uint32_t STACK_SIZE = 1024;

    TaskHandle_t xHandle = NULL;

    q = xQueueCreate(10, sizeof(int));

    LabUART q1;

    q1.uartInit(rx_isr, 2);

    //xTaskCreate(tx_task, "txTask", STACK_SIZE, (void *) 2, 1, &xHandle);
    xTaskCreate(rx_task, "rxTask", STACK_SIZE, (void *) 1, 1, &xHandle);
    xTaskCreate(vReadSwitch, "vReadSwitch", STACK_SIZE, (void *) 1, 1, &xHandle);
    vTaskStartScheduler();

    //init_my_uart2();
    //create_task(my_task);
    return 0;
}
#endif

// ADC PWM
#if 0
int main(int argc, char const *argv[])
{

    float val = 0.0;

    uint16_t X;

    LabGPIO test(0,0);

    test.setAsOutput();
    test.setHigh();

    ADCDriver adc;
    adc.adcInitBurstMode();
    adc.adcSelectPin(adc.ADC_PIN_1_31);

    PWMDriver test1;
    test1.pwmSelectAllPins();
    //test1.pwmSelectPin(test1.PWM_PIN_2_3);
    test1.pwmInitSingleEdgeMode(10000);

    // Testing
    while(1) {
        //if(LPC_ADC->ADDR3 & (1<<31)) {

        val = adc.readADCVoltageByChannel(5);

        //val = result;
        /*X = AS.getX();
         Y = AS.getY();
         Z = AS.getZ();*/
        //X = LS.getPercentValue();
        val = (val/4095)*100;
        //printf("Result: %.02f\n",val);
        printf("X : %f \n", val);
        //val = (val/4095)*100;
        test1.setDutyCycle( test1.PWM_PIN_2_1, (val));
        test1.setDutyCycle( test1.PWM_PIN_2_2, (val)/4);
        test1.setDutyCycle( test1.PWM_PIN_2_3, ((100 - val))/2);
        //delay_ms(1000);

        //}

        /* test1.setDutyCycle( test1.PWM_PIN_2_3, 5);
         delay_us(1000000);
         test1.setDutyCycle( test1.PWM_PIN_2_3, 50);
         delay_us(1000000);
         test1.setDutyCycle( test1.PWM_PIN_2_3, 100);
         delay_us(1000000);*/
    }

    return 0;
}
#endif

#if 0

SemaphoreHandle_t gatekeeper = 0;

typedef union {
    uint8_t byte;
    struct {
        uint8_t ready :1;
        uint8_t compare_result :1;
        uint8_t density_code :4;
        uint8_t sector_protect_status :1;
        uint8_t page_size_configuration :1;
    }__attribute__((packed));
}status_reg1;

typedef union {
    uint8_t byte;
    struct {
        uint8_t ready :1;
        uint8_t reserved1 :1;
        uint8_t erase_program_error :1;
        uint8_t reserved2 :1;
        uint8_t sector_lockdown_enabled :1;
        uint8_t program_suspend_1 :1;
        uint8_t program_suspend_2 :1;
        uint8_t erase_suspend :1;
    }__attribute__((packed));
}status_reg2;

typedef union {
    uint8_t partition[16];
    struct {
        uint8_t boot_indicator_bit_flag[1];
        uint8_t starting_head[1];
        uint8_t starting_sector[1];
        uint8_t starting_cylinder[1];
        uint8_t system_id[1];
        uint8_t ending_head[1];
        uint8_t ending_sector[1];
        uint8_t ending_cylinder[1];
        uint8_t relative_sector[4];
        uint8_t total_sector[4];
    }__attribute__((packed));
}partition_table;

typedef union {
    uint8_t page[512];
    struct {
        uint8_t bootstrap_code_area[446];
        partition_table partition1[16];
        partition_table partition2[16];
        partition_table partition3[16];
        partition_table partition4[16];
        uint8_t signature[2];
    }__attribute__((packed));
}fat_12;

void adesto_cs()
{
    LabGPIO cs(6, 0);
    cs.setAsOutput();
    cs.setLow();
    LabGPIO cs1(0, 0);
    cs1.setAsOutput();
    cs1.setLow();
}

void adesto_ds()
{

    LabGPIO cs(6, 0);
    cs.setAsOutput();
    cs.setHigh();
    LabGPIO cs1(0, 0);
    cs1.setAsOutput();
    cs1.setHigh();

}

void read_512_byte_page(LabSPI instance, status_reg1 byte1, status_reg2 byte2)
{

    // Setting up for 512 bytes configuration
    adesto_cs();
    instance.transfer(0x3D);
    instance.transfer(0x2A);
    instance.transfer(0x80);
    instance.transfer(0xA6);
    adesto_ds();

    // Checking busy status of SPI Flash
    adesto_cs();
    instance.transfer(0xD7);
    byte1.byte = instance.transfer(0xD7);
    while (!byte1.ready)
    ;
    adesto_ds();

    // Read page from memory operation
    adesto_cs();

    fat_12 page_zero;
    instance.transfer(0xD2);
    instance.transfer(0x00);
    instance.transfer(0x00);
    instance.transfer(0x00);
    instance.transfer(0x12);
    instance.transfer(0x23);
    instance.transfer(0x32);
    instance.transfer(0x65);
    for (uint16_t i = 0; i < 512; ++i)
    page_zero.page[i] = instance.transfer(0x63);
    adesto_ds();
    u0_dbg_printf(" Boot Signature :   %02X %02X ", page_zero.signature[0], page_zero.signature[1]);

}

void task_sig_reader(void *p)
{
    LabSPI sig;
    while (1) {
        //if (xSemaphoreTake(gatekeeper, 1000)) {
        adesto_cs();

        sig.transfer(0x9F);// Find what to send to read Adesto flash signature

        uint8_t s1 = sig.transfer(0x9F);
        uint8_t s2 = sig.transfer(0x9F);
        adesto_ds();
        puts("sig got access");
        //xSemaphoreGive(gatekeeper);
        if (s1 != 0x1F || s2 != 0x26) {
            puts("Ooops... race condition \n");
            vTaskSuspend(NULL); // Suspend this task
        }
        /*}
         else {
         puts("Sig failed to get access");
         }*/
        vTaskDelay(1);
    }
}

void task_page_reader(void *p)
{
    LabSPI page;
    status_reg1 first_byte;
    status_reg2 second_byte;
    while (1) {
        //if (xSemaphoreTake(gatekeeper, 1000)) {
        //adesto_cs();
        read_512_byte_page(page, first_byte, second_byte);
        puts("page got access");
        //adesto_ds();
        //  xSemaphoreGive(gatekeeper);
        /*}
         else {
         puts("Page failed to get access");
         }*/
        vTaskDelay(1);
    }
}

void printStatusReg1(status_reg1 byte1) {
    if ((byte1.ready)) {
        u0_dbg_printf(" Device is Ready \n");
    }
    else {
        u0_dbg_printf(" Device is Busy with internal task \n");
    }
    if (byte1.compare_result) {
        u0_dbg_printf(" Main memory page data does'nt match page data \n");
    }
    else {
        u0_dbg_printf(" Main memory page data matches page data \n");
    }
    if (byte1.sector_protect_status) {
        u0_dbg_printf(" Sector protection is enabled \n");
    }
    else {
        u0_dbg_printf(" Sector protection is disabled \n");
    }
    if (byte1.page_size_configuration) {
        u0_dbg_printf(" Device is configured for “power of 2” binary page size (512 bytes) \n");
    }
    else {
        u0_dbg_printf(" Device is configured for standard DataFlash page size (528 bytes) \n");
    }
}

void printStatusReg2(status_reg2 byte2) {
    if (byte2.ready) {
        u0_dbg_printf(" Device is Ready \n");
    }
    else {
        u0_dbg_printf(" Device is Busy with internal task \n");
    }
    if (byte2.erase_program_error) {
        u0_dbg_printf(" Erase or program Error \n");
    }
    else {
        u0_dbg_printf(" Erase or program operation was successful. \n");
    }
    if (byte2.erase_suspend) {
        u0_dbg_printf(" A sector is erase suspended. \n");
    }
    else {
        u0_dbg_printf(" No sectors are erase suspended. \n");
    }
    if (byte2.program_suspend_1) {
        u0_dbg_printf(" A sector is program suspended while using Buffer 1 \n");
    }
    else {
        u0_dbg_printf(" No program operation has been suspended while using Buffer 1 \n");
    }
    if (byte2.program_suspend_2) {
        u0_dbg_printf(" A sector is program suspended while using Buffer 2 \n");
    }
    else {
        u0_dbg_printf(" No program operation has been suspended while using Buffer 2 \n");
    }
    if (byte2.sector_lockdown_enabled) {
        u0_dbg_printf(" Sector Lockdown command is enabled \n");
    }
    else {
        u0_dbg_printf(" Sector Lockdown command is disabled \n");
    }
}

int main(int argc, char const *argv[])
{
    const uint32_t STACK_SIZE = 1024;

    TaskHandle_t xHandle = NULL;

    bool success;
    status_reg1 byte1;
    status_reg2 byte2;

    LabSPI instance;

    success = instance.init(instance.SSP1, 7, instance.SPI, 100);

    if (success) {

        //Part 1
        puts("\n");
        puts(" PART 1 \n");
        puts("\n");
        adesto_cs();// Assert a LOW signal on the CS signal connected to Adesto

        // Note: device ID to be printed together
        uint8_t test = instance.transfer(0x9F);// Find what to send to read Adesto flash signature
        test = instance.transfer(0x9F);
        printf("Manufacturer ID: %02X\n", test);
        test = instance.transfer(0x9F);
        printf("Device ID part1: %02X\n", test);
        test = instance.transfer(0x9F);
        printf("Device ID part2: %02X\n", test);

        adesto_ds();// Assert a HIGH signal to de-assert the CS

        // Part 2
        puts("\n");
        puts(" PART 2 \n");
        puts("\n");
        adesto_cs();// Assert a LOW signal on the CS signal connected to Adesto

        byte1.byte = instance.transfer(0xD7);
        byte1.byte = instance.transfer(0xD7);
        printf("Returned data: %02X\n", byte1.byte);
        byte2.byte = instance.transfer(0xD7);
        printf("Returned data: %02X\n", byte2.byte);

        adesto_ds();// Assert a HIGH signal to de-assert the CS

// Under construction
        puts("\n");
        u0_dbg_printf(" Status Register Byte 1 : \n");
        puts("\n");

        printStatusReg1(byte1);
        puts("\n");
        u0_dbg_printf(" Status register Byte 2 :\n");
        puts("\n");

        printStatusReg2(byte2);

        adesto_cs();
        instance.transfer(0x3D);
        instance.transfer(0x2A);
        instance.transfer(0x80);
        instance.transfer(0xA6);
        adesto_ds();

        // Checking busy status of SPI Flash
        adesto_cs();
        instance.transfer(0xD7);
        byte1.byte = instance.transfer(0xD7);
        while (!byte1.ready)
        ;
        adesto_ds();

        // Read page from memory operation
        adesto_cs();

        fat_12 page_zero;
        instance.transfer(0xD2);
        instance.transfer(0x00);
        instance.transfer(0x00);
        instance.transfer(0x00);
        instance.transfer(0x12);
        instance.transfer(0x23);
        instance.transfer(0x32);
        instance.transfer(0x65);
        for (uint16_t i = 0; i < 512; ++i)
        page_zero.page[i] = instance.transfer(0x63);
        adesto_ds();
        //u0_dbg_printf(" Boot Signature :   %02X %02X ", page_zero.signature[0]);
        u0_dbg_printf(" Boot Signature :   %02X %02X ", page_zero.page[510], page_zero.page[511]);

// Under Construction: Need to add all the status bits

        gatekeeper = xSemaphoreCreateMutex();
        // part 3
        puts("\n");
        puts(" PART 3 \n");
        puts("\n");
        //xTaskCreate(task_page_reader, "TaskA", STACK_SIZE, (void *) 1, 2, &xHandle);
        //xTaskCreate(task_sig_reader, "TaskB", STACK_SIZE, (void *) 1, 1, &xHandle);
        //vTaskStartScheduler();
    }
    else {
        u0_dbg_printf(" SPI initialization failed ");
    }
    return 0;
}

#endif

// GPIO Interrupt Drivers

#if 0
SemaphoreHandle_t xSemaphore;
//LabGPIOInterrupts* instance;

bool switch_state = false;

#if 1
void vControlLED( void * pvParameters )
{
    bool param = (bool)(pvParameters);
    uint32_t pin, port;

    if(param)
    {
        pin = 0;
        port = 1;
    }
    else
    {
        pin = 0;
        port = 0;
    }
    printf("shfgiudsjvlikdsxcn");

    LabGPIO l1(pin, port);
    l1.setAsOutput();

    while(1)
    {
        if(xSemaphoreTake(xSemaphore, portMAX_DELAY))
        {
            LPC_GPIO2->FIOPIN ^= (1<<1);
            if(l1.getLevel())
            l1.setLow();
            else
            l1.setHigh();
        }
        switch_state = false;
        vTaskDelay(10);
    }
}

void user_callback(void)
{
    xSemaphoreGiveFromISR(xSemaphore, NULL);
}

void c_eint3_handler(void)
{
    instance->externalIRQHandler();
}

int main()
{

    const uint32_t STACK_SIZE = 1024;

    TaskHandle_t xHandle = NULL;

    xSemaphore = xSemaphoreCreateBinary();

    //LabGPIOInterrupts* I = LabGPIOInterrupts::Instance();
    //I->init();
    LabGPIOInterrupts I;

    I.attachInterruptHandler(2, 3, user_callback, rising_edge);

    isr_register(EINT3_IRQn, c_eint3_handler);

    xTaskCreate( vControlLED, "TaskA", STACK_SIZE, (void * ) 1, 1, &xHandle );
    vTaskStartScheduler();

    return 0;
}
#endif
#endif

#if 0
void vControlLED( void * pvParameters )
{
    bool param = (bool)(pvParameters);
    uint32_t pin, port;

    if(param)
    {
        pin = 0;
        port = 1;
    }
    else
    {
        pin = 1;
        port = 2;
    }

    LabGPIO l1(pin, port);
    l1.setAsOutput();

    while(1)
    {
        if(switch_state)
        {
            if(l1.getLevel())
            l1.setLow();
            else
            l1.setHigh();
        }
        switch_state = false;
        vTaskDelay(10);
    }
}

void vReadSwitch( void * pvParameters)
{
    bool param = (bool)(pvParameters);
    uint32_t pin, port;

    if(param)
    {
        pin = 9;
        port = 1;
    }
    else
    {
        pin = 0;
        port = 2;
    }

    LabGPIO b1(pin, port);
    b1.setAsInput();

    while (1)
    {
        if (b1.getLevel())
        {
            switch_state = true;
        }
        if (!b1.getLevel() && switch_state)
        {
            vTaskDelay(300);
        }
    }
}

void vExtraCredit(void * pvParameters) {

    while (1) {

        LabGPIO l1(0, 1);
        l1.setAsOutput();
        if (switch_state) {
            if (l1.getLevel()) l1.setLow();
            else
            l1.setHigh();
        }
        LabGPIO l2(0, 1);
        l2.setAsOutput();
        if (switch_state) {
            if (!l2.getLevel()) l2.setLow();
            else
            l2.setHigh();
        }

        switch_state = false;
        vTaskDelay(10);
    }

}

int main(int argc, char const *argv[])
{
    const uint32_t STACK_SIZE = 1024;

    TaskHandle_t xHandle = NULL;

    //bool external = true;
    bool external = false;

    xTaskCreate( vControlLED, "TaskA", STACK_SIZE, (void * ) external, 1, &xHandle );
    xTaskCreate( vReadSwitch, "TaskB", STACK_SIZE, (void * ) external, 2, &xHandle );
    vTaskStartScheduler();

    return 0;
}
#endif

#if 0
/*void vTaskOneCode(void * )
 {
 while(1)
 {
 uart0_puts("aaaaaaaaaaaaaaaaaaaa");
 vTaskDelay(100); // This sleeps the task for 100ms (because 1 RTOS tick = 1 millisecond)
 }
 }

 // Create another task and run this code in a while(1) loop
 void vTaskTwoCode(void * )
 {
 while(1)
 {
 uart0_puts("bbbbbbbbbbbbbbbbbbbb");
 vTaskDelay(100);
 }
 }

 // You can comment out the sample code of lpc1758_freertos project and run this code instead
 int main(int argc, char const *argv[])
 {
 /// This "stack" memory is enough for each task to run properly
 const uint32_t STACK_SIZE = 1024;
 //BaseType_t xReturned1;
 //BaseType_t xReturned2;
 TaskHandle_t xHandle = NULL;


 xTaskCreate( vTaskOneCode, "TaskA", STACK_SIZE, (void *) 0, 1, &xHandle );
 xTaskCreate( vTaskTwoCode, "TaskB", STACK_SIZE, (void *) 0, 1, &xHandle );

 vTaskStartScheduler();

 return 0;
 }*/

/**
 * The main() creates tasks or "threads".  See the documentation of scheduler_task class at scheduler_task.hpp
 * for details.  There is a very simple example towards the beginning of this class's declaration.
 *
 * @warning SPI #1 bus usage notes (interfaced to SD & Flash):
 *      - You can read/write files from multiple tasks because it automatically goes through SPI semaphore.
 *      - If you are going to use the SPI Bus in a FreeRTOS task, you need to use the API at L4_IO/fat/spi_sem.h
 *
 * @warning SPI #0 usage notes (Nordic wireless)
 *      - This bus is more tricky to use because if FreeRTOS is not running, the RIT interrupt may use the bus.
 *      - If FreeRTOS is running, then wireless task may use it.
 *        In either case, you should avoid using this bus or interfacing to external components because
 *        there is no semaphore configured for this bus and it should be used exclusively by nordic wireless.
 */
int main(void)
{
    /**
     * A few basic tasks for this bare-bone system :
     *      1.  Terminal task provides gateway to interact with the board through UART terminal.
     *      2.  Remote task allows you to use remote control to interact with the board.
     *      3.  Wireless task responsible to receive, retry, and handle mesh network.
     *
     * Disable remote task if you are not using it.  Also, it needs SYS_CFG_ENABLE_TLM
     * such that it can save remote control codes to non-volatile memory.  IR remote
     * control codes can be learned by typing the "learn" terminal command.
     */
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    /* Consumes very little CPU, but need highest priority to handle mesh network ACKs */
    scheduler_add_task(new wirelessTask(PRIORITY_CRITICAL));

    /* Change "#if 0" to "#if 1" to run period tasks; @see period_callbacks.cpp */
#if 0
    const bool run_1Khz = false;
    scheduler_add_task(new periodicSchedulerTask(run_1Khz));
#endif

    /* The task for the IR receiver to "learn" IR codes */
    // scheduler_add_task(new remoteTask  (PRIORITY_LOW));
    /* Your tasks should probably used PRIORITY_MEDIUM or PRIORITY_LOW because you want the terminal
     * task to always be responsive so you can poke around in case something goes wrong.
     */

    /**
     * This is a the board demonstration task that can be used to test the board.
     * This also shows you how to send a wireless packets to other boards.
     */
#if 0
    scheduler_add_task(new example_io_demo());
#endif

    /**
     * Change "#if 0" to "#if 1" to enable examples.
     * Try these examples one at a time.
     */
#if 0
    scheduler_add_task(new example_task());
    scheduler_add_task(new example_alarm());
    scheduler_add_task(new example_logger_qset());
    scheduler_add_task(new example_nv_vars());
#endif

    /**
     * Try the rx / tx tasks together to see how they queue data to each other.
     */
#if 0
    scheduler_add_task(new queue_tx());
    scheduler_add_task(new queue_rx());
#endif

    /**
     * Another example of shared handles and producer/consumer using a queue.
     * In this example, producer will produce as fast as the consumer can consume.
     */
#if 0
    scheduler_add_task(new producer());
    scheduler_add_task(new consumer());
#endif

    /**
     * If you have RN-XV on your board, you can connect to Wifi using this task.
     * This does two things for us:
     *   1.  The task allows us to perform HTTP web requests (@see wifiTask)
     *   2.  Terminal task can accept commands from TCP/IP through Wifly module.
     *
     * To add terminal command channel, add this at terminal.cpp :: taskEntry() function:
     * @code
     *     // Assuming Wifly is on Uart3
     *     addCommandChannel(Uart3::getInstance(), false);
     * @endcode
     */
#if 0
    Uart3 &u3 = Uart3::getInstance();
    u3.init(WIFI_BAUD_RATE, WIFI_RXQ_SIZE, WIFI_TXQ_SIZE);
    scheduler_add_task(new wifiTask(Uart3::getInstance(), PRIORITY_LOW));
#endif

    scheduler_start(); ///< This shouldn't return
    return -1;
}
#endif

