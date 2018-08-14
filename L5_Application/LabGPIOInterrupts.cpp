/*
 * LabGPIOInterrupt.cpp
 *
 *  Created on: Feb 20, 2018
 *      Author: aksha
 */




#include "LPC17xx.h"     // LPC_UART0_BASE
#include "sys_config.h"  // sys_get_cpu_clock()
#include <stddef.h>
#include "LabGPIOInterrupts.hpp"
#include "FreeRTOS.h"
#include "semphr.h"
#include "uart0_min.h"

#include "eint.h"

//For isr_register

#include "lpc_isr.h"

//SemaphoreHandle_t xSemaphore;

//LabGPIOInterrupts* LabGPIOInterrupts::instance = 0;

/**
 * IRQ Handler needs to be enclosed in extern "C" because this is C++ file, and
 * we don't want C++ to "mangle" our function name.
 * This ISR Function need needs to be named precisely to override "WEAK" ISR
 * handler defined at startup.cpp
 */



    LabGPIOInterrupts::LabGPIOInterrupts(){
    }

    /*LabGPIOInterrupts* LabGPIOInterrupts::Instance(){
        if( instance == NULL){
            instance = new LabGPIOInterrupts();
        }
        return instance;
    }*/

    void LabGPIOInterrupts::init(){
            //xSemaphore = xSemaphoreCreateBinary();
            NVIC_EnableIRQ(EINT3_IRQn);
    }

    /*void LabGPIOInterrupts::myVectorFunc(uint8_t pin, uint32_t port, void (*p)(void))
           {
                myVector[pin][port] = p;
           }*/

    bool LabGPIOInterrupts::attachInterruptHandler(uint8_t newPort, uint32_t newPin, void (*pin_isr)(void), InterruptCondition_E condition){
    switch (newPort){
        case 0:
            LPC_GPIO0->FIODIR &= ~(1 << newPin);
            LPC_GPIOINT->IO0IntEnR |= (1 << newPin);
            break;
        case 2:
            LPC_GPIO2->FIODIR &= ~(1 << newPin);
            LPC_GPIOINT->IO2IntEnR |= (1 << newPin);
            break;
        default:
            LPC_GPIO2->FIODIR &= ~(1 << newPin);
            LPC_GPIOINT->IO2IntEnR |= (1 << newPin);
            break;
        }

        //myVectorFunc(newPin, newPort, pin_isr);

        return true;


        /*eint3_enable_port2(newPin, condition, pin_isr);


            return true;*/
    }

    void LabGPIOInterrupts::externalIRQHandler(void){

        //Create a function in eint.h and eint.c to call externalIRQHandler
        //hook up the semaphore giver in the main with the function in eint.h

        //Search in the lookup table and find address pin_isr also create a lookup table for interrupt condition
        //Pass port, pin, pin_isr and interrupt condition to attachInterruptHandler()
        //Check for return value from attachInterruptHandler, if true -> give semaphore , false -> don't give semaphore
        //xSemaphoreGiveFromISR(xSemaphore, NULL);

        //myVector[2][0]();

        //instance->result = instance->attachInterruptHandler(0, 1, rising_edge);


        /*if(instance->attachInterruptHandler(0, 1, *pin_isr, rising_edge))
        {

        }*/
        //uart0_puts("hgiuilkj;lk;");
          //  LPC_GPIOINT->IO2IntClr = (1<<0);*/
    }

    LabGPIOInterrupts::~LabGPIOInterrupts(){

    }
