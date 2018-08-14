/*
 * LabSpi.hpp
 *
 *  Created on: Feb 23, 2018
 *      Author: aksha
 */

#ifndef LABSPI_HPP_
#define LABSPI_HPP_

#include "FreeRTOS.h"


//LabSPI instance;

class LabSPI
{
private:
    // SSP register lookup table structure
    //static const LPC_SSP_TypeDef * SSP[] = {LPC_SSP0, LPC_SSP1};
    LPC_SSP_TypeDef * SSP;

public:

    typedef union
    {
        uint8_t CR_0;
        struct
        {
            uint8_t data_size: 4;
            uint8_t frame_format: 2;
            uint8_t cpol: 1;
            uint8_t cpha: 1;
        } __attribute__((packed));
    } controlReg;

    typedef enum
    {
        /* Fill this out based on the datasheet. */
        SPI,
        TI,
        Microwire
    } FrameModes;

    typedef enum
    {
        SSP0,
        SSP1
    } Peripheral;

    /**
     * 1) Powers on SPPn peripheral
     * 2) Set peripheral clock
     * 3) Sets pins for specified peripheral to MOSI, MISO, and SCK
     *
     * @param peripheral which peripheral SSP0 or SSP1 you want to select.
     * @param data_size_select transfer size data width; To optimize the code, look for a pattern in the datasheet
     * @param format is the code format for which synchronous serial protocol you want to use.
     * @param divide is the how much to divide the clock for SSP; take care of error cases such as the value of 0, 1, and odd numbers
     *
     * @return true if initialization was successful
     */
    bool init(Peripheral peripheral, uint8_t data_size_select, FrameModes format, uint8_t divide);

    /**
     * Transfers a byte via SSP to an external device using the SSP data register.
     * This region must be protected by a mutex static to this class.
     *
     * @return received byte from external device via SSP data register.
     */
    uint8_t transfer(uint8_t send);

    bool validDivideParam(uint8_t value);

    LabSPI();
    ~LabSPI();
};

#endif /* LABSPI_HPP_ */
