/*
 * LabSpi.cpp
 *
 *  Created on: Feb 23, 2018
 *      Author: aksha
 */

#include "FreeRTOS.h"
#include "LabSPI.hpp"
#include "printf_lib.h"
#include "semphr.h"

static SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

LabSPI::LabSPI()
{
    SSP = LPC_SSP1;
}
;

bool validDivideParam(uint8_t value)
{
    bool result = true;

    if (value % 2 == 0 && value != 0) {
        result = true;
    }
    else {
        result = false;
    }

    return result;
}

bool LabSPI::init(Peripheral peripheral, uint8_t data_size_select, FrameModes format, uint8_t divide)
{
    bool success = true;
    switch (peripheral) {
        case 0:
            SSP = LPC_SSP0;
            LPC_SC->PCONP |= (1 << 21);
            LPC_SC->PCLKSEL1 &= ~(3 << 10);
            LPC_SC->PCLKSEL1 |= (1 << 10);
            LPC_PINCON->PINSEL0 |= (1 << 31);
            LPC_PINCON->PINSEL1 |= (1 << 3 | 1 << 5);
            break;
        case 1:
            SSP = LPC_SSP1;
            LPC_SC->PCONP |= (1 << 10);
            LPC_SC->PCLKSEL0 &= ~(3 << 20);
            LPC_SC->PCLKSEL0 |= (1 << 20);

            LPC_PINCON->PINSEL0 &= ~((3 << 14) | (3 << 16) | (3 << 18));
            LPC_PINCON->PINSEL0 |= ((2 << 14) | (2 << 16) | (2 << 18));
            break;
        default:
            SSP = LPC_SSP1;
            LPC_SC->PCONP |= (1 << 10);
            LPC_SC->PCLKSEL0 &= ~(3 << 20);
            LPC_SC->PCLKSEL0 |= (1 << 20);
            LPC_PINCON->PINSEL0 &= ~((3 << 14) | (3 << 16) | (3 << 18));
            LPC_PINCON->PINSEL0 |= ((2 << 14) | (2 << 16) | (2 << 18));
            break;
    }

    SSP->CR0 = data_size_select;          // 8-bit mode
    SSP->CR1 = (1 << 1);   // Enable SSP as Master
    //SSP->CPSR = divide;         // SCK speed = CPU / 8

    if (divide % 2 == 0 && divide != 0) {
        SSP->CPSR |= divide;
    }
    else {
        success = false;
        u0_dbg_printf("invalid entry");
    }

    // Data size and code format
    controlReg cont;
    if (2 < data_size_select && data_size_select <= 16) {
        cont.data_size = data_size_select;
        cont.frame_format = SPI;
    }
    else {
        success = false;
        u0_dbg_printf("invalid entry");
    }

    uint32_t CREG = cont.CR_0;

    SSP->CR0 = CREG;

    SSP->CR1 = (1 << 1);

    return success;
}

uint8_t LabSPI::transfer(uint8_t send)
{
    if (xSemaphoreTake(mutex, 1000)) {
        LPC_SSP1->DR = send;
        while (LPC_SSP1->SR & (1 << 4))
            ;
        xSemaphoreGive(mutex);
    }
    return LPC_SSP1->DR;
}

LabSPI::~LabSPI()
{

}
;

