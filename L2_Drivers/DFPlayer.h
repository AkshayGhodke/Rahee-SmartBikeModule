/*
 * DFPlayer.hpp
 *
 *  Created on: May 15, 2018
 *      Author: aksha
 */

#if 1

#ifndef DFPLAYER_HPP_
#define DFPLAYER_HPP_

#include "tasks.hpp"
#include "examples/examples.hpp"
#include "io.hpp"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart0_min.h"
#include "printf_lib.h"
#include "utilities.h"

#include "uart2.hpp"

// DFPlayer mini Fixed Commands
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x01 //Returns info with command 0x41 [0x01: info, 0x00: no info]
// checksum
# define  Voice_Alert 0xF6
# define  Voice_Right 0xF5
# define  Voice_Left 0xF4
# define  Voice_Forward 0xF3
# define  Voice_Reverse 0xF2

// DFplayer

void execute_Command(Uart2 serial, uint8_t CMD, uint8_t par1, uint8_t par2, uint8_t checksum)
{
    uint8_t Command_line[10] = { Start_Byte, Version_Byte, Command_Length, 0x0C, Acknowledge, 0x00, 0x02, 0xFE, 0xEE, End_Byte };

    Command_line[3] = CMD;
    Command_line[5] = par1;
    Command_line[6] = par2;
    Command_line[8] = checksum;

    for (int i = 0; i < 10; i++) {
        serial.putChar(Command_line[i]);
    }
    delay_ms(3500); // Requires for initialization
}

#endif

#endif /* DFPLAYER_HPP_ */

