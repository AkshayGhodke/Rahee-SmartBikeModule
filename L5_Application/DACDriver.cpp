/*
 * DACDriver.cpp
 *
 *  Created on: Apr 21, 2018
 *      Author: akshay
 */


#include "DACDriver.hpp"
#include "printf_lib.h"

DACDriver::DACDriver(){

}

void DACDriver::init(){
    // Enable DAC

    LPC_PINCON->PINSEL1 &= ~( 3 << 20 );
    LPC_PINCON->PINSEL1 |= ( 2 << 20 );

    // Clock

    LPC_SC->PCLKSEL0 |= ( 3 << 22 );

    // Interrupt Timeout

    LPC_DAC->DACCNTVAL |= ( 0xFFFF );

}

void DACDriver::write(uint8_t data){
    //while(!(LPC_DAC->DACCTRL & ( 1<<0 )));
    u0_dbg_printf("Sent Data : %d", data);
    LPC_DAC->DACR &= ~(0xFF << 6 || 3 << 14);
    LPC_DAC->DACR |= (data << 6);
}



