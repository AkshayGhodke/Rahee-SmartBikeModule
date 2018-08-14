/*
 * LabUart.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: aksha
 */

#include "LabUart.hpp"
#include <LPC17xx.h>

LabUART::LabUART()
{
    uart = LPC_UART2;
}

LabUART::~LabUART()
{
}

bool LabUART::available(){
   return true;
}

void LabUART::uartInit(void (*rx_isr)(void), uint8_t port)
{

    switch (port) {
        case 2:
            uart = LPC_UART2;
            LPC_SC->PCONP |= (1 << 24);
            LPC_SC->PCLKSEL1 &= ~(3 << 16);
            LPC_SC->PCLKSEL1 |= (1 << 16);
            LPC_PINCON->PINSEL4 &= ~(3 << 16 | 3 << 18);
            LPC_PINCON->PINSEL4 |= (1 << 17 | 1 << 19);
            NVIC_EnableIRQ(UART2_IRQn);
            isr_register(UART2_IRQn, rx_isr);
            break;
        case 3:
            uart = LPC_UART3;
            LPC_SC->PCONP |= (1 << 25);
            LPC_SC->PCLKSEL1 &= ~(3 << 18);
            LPC_SC->PCLKSEL1 |= (1 << 18);
            //LPC_PINCON->PINSEL9 &= ~(3 << 26 | 3 << 24);
            //LPC_PINCON->PINSEL9 |= (3 << 26 | 3 << 24);
            //LPC_PINCON->PINSEL0 &= ~(3 <<  0| 3 << 2);
            //LPC_PINCON->PINSEL0 |= (1 << 1 | 1 << 3);
            LPC_PINCON->PINSEL1 &= ~(3 << 18 | 3 << 20);
            LPC_PINCON->PINSEL1 |= (3 << 18 | 3 << 20);
            NVIC_EnableIRQ(UART3_IRQn);
            isr_register(UART3_IRQn, rx_isr);
            break;
        default:
            uart = LPC_UART2;
            LPC_SC->PCONP |= (1 << 24);
            LPC_SC->PCLKSEL1 &= ~(3 << 16);
            LPC_SC->PCLKSEL1 |= (1 << 16);
            LPC_PINCON->PINSEL4 &= ~(3 << 16 | 3 << 18);
            LPC_PINCON->PINSEL4 |= (1 << 17 | 1 << 19);
            NVIC_EnableIRQ(UART2_IRQn);
            isr_register(UART2_IRQn, rx_isr);
            break;
    }

    uint32_t baud = 9600;
    // DLL and DLM setting
    //uint8_t dll = sys_get_cpu_clock()/(16 * baud);
    uart->LCR |= (1 << 7);       // DLAB
    uart->DLL = sys_get_cpu_clock() / (16 * baud);

    // DLAB reset
    uart->LCR |= (0 << 7);

    uart->LCR = 3;

    // FIFO enable
    uart->FCR |= (1 << 0);

    uart->FCR &= ~(3 << 1);
    uart->FCR |= (3 << 1);

    // RBR interrupt enable
    uart->IER |= (1 << 0);

    uart->TER = (1 << 7);
}

void LabUART::transmit(uint8_t data)
{
    printf("start sending received data.... \n ");
    uart->THR = data;
    delay_ms(50);
}

uint8_t LabUART::receive()
{
    uint8_t rx_data = 0;

    rx_data = uart->RBR;

    return rx_data;
}

