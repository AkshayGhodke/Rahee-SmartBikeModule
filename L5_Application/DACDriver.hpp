/*
 * DACDriver.hpp
 *
 *  Created on: Apr 21, 2018
 *      Author: aksha
 */

#ifndef DACDRIVER_HPP_
#define DACDRIVER_HPP_

#include <LPC17xx.h>


class DACDriver{
private:

public:
    DACDriver();
    void init();
    uint8_t Read();
    void write(uint8_t);
};

#endif /* DACDRIVER_HPP_ */
