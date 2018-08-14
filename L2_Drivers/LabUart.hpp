/*
 * LabUart.hpp
 *
 *  Created on: Mar 9, 2018
 *      Author: aksha
 */

#include "tasks.hpp"
#include "examples/examples.hpp"
#include "io.hpp"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart0_min.h"
#include "printf_lib.h"
#include "utilities.h"

#ifndef LABUART_HPP_
#define LABUART_HPP_

class LabUART
{
private:
    LPC_UART_TypeDef *uart;
public:
    LabUART();
    ~LabUART();
    void uartInit(void (*)(void), uint8_t);
    void transmit(uint8_t);
    uint8_t receive();
    bool available();


    // TODO: Fill in methods for init(), transmit(), receive() etc.


    // Optional: For the adventurous types, you may inherit from "CharDev" class to get a lot of funcionality for free
};

#endif /* LABUART_HPP_ */
