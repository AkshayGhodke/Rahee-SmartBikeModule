/*
 * Lab_GPIO.cpp
 *
 *  Created on: Feb 16, 2018
 *      Author: aksha
 */

#include "LPC17xx.h"     // LPC_UART0_BASE
#include "sys_config.h"  // sys_get_cpu_clock()
#include "LabGPIO.hpp"


/**
 * IRQ Handler needs to be enclosed in extern "C" because this is C++ file, and
 * we don't want C++ to "mangle" our function name.
 * This ISR Function need needs to be named precisely to override "WEAK" ISR
 * handler defined at startup.cpp
 */


LabGPIO::LabGPIO(uint8_t PIN, uint8_t PORT)
{

    this->pin = PIN;
    this->port = PORT;
    switch (port){
         case 0: gpio = LPC_GPIO0;
                 break;
         case 1: gpio = LPC_GPIO1;
                 break;
         case 2: gpio = LPC_GPIO2;
                 break;
         default:gpio = LPC_GPIO1;
                 break;
    }
}
    /**
     * Should alter the hardware registers to set the pin as an input
     */
    void LabGPIO::setAsInput()
    {
        gpio->FIODIR &= ~( 1 << pin );
    }
    /**
     * Should alter the hardware registers to set the pin as an input
     */
    void LabGPIO::setAsOutput()
    {
        gpio->FIODIR |= ( 1 << pin );
    }
    /**
     * Should alter the set the direction output or input depending on the input.
     *
     * @param {bool} output - true => output, false => set pin to input
     */
    void LabGPIO::setDirection(bool output)
    {
        if(output)
        {
            gpio->FIODIR |= ( 1 << pin );
        }
        else
        {
            gpio->FIODIR &= ~( 1 << pin );
        }
    }
    /**
     * Should alter the hardware registers to set the pin as high
     */
    void LabGPIO::setHigh()
    {
        gpio->FIOSET = ( 1 << pin );
    }
    /**
     * Should alter the hardware registers to set the pin as low
     */
    void LabGPIO::setLow()
    {
        gpio->FIOCLR = ( 1 << pin );
    }
    /**
     * Should alter the hardware registers to set the pin as low
     *
     * @param {bool} high - true => pin high, false => pin low
     */
    void LabGPIO::set(bool high)
    {
        if(high)
        {
            gpio->FIOSET = ( 1 << pin );
        }
        else
        {
            gpio->FIOCLR = ( 1 << pin );
        }
    }
    /**
     * Should return the state of the pin (input or output, doesn't matter)
     *
     * @return {bool} level of pin high => true, low => false
     */
    bool LabGPIO::getLevel()
    {
        bool result;

        if(LPC_GPIO1->FIOPIN & ( 1 << pin ))
        {
            result = true;
        }
        else
        {
            result = false;
        }

        return result;
    }
    LabGPIO::~LabGPIO()
    {

    }

