/*
 * ADXL362.h
 *
 *  Created on: May 23, 2018
 *      Author: aksha
 */

#ifndef ADXL362_H_
#define ADXL362_H_

GPIO acc(P0_1); // chip select
// P0.0 Neopixel

uint16_t test = 0;
uint16_t test1 = 0;
uint16_t test2 = 0;
uint16_t result = 0;


uint16_t accReadX()
{
    // Reading LSB from register 0x0E for X-axis reading
    acc.setLow();
    ssp1_exchange_byte(0x0B);
    ssp1_exchange_byte(0x0E);
    test = ssp1_exchange_byte(0x00);
    acc.setHigh();
    // Reading LSB from register 0x0F for X-axis reading
    acc.setLow();
    ssp1_exchange_byte(0x0B);
    ssp1_exchange_byte(0x0F);
    test1 = ssp1_exchange_byte(0x00);
    acc.setHigh();
    result = (test1 << 8) + test;
    return result;
}
uint16_t accReadY()
{
    // Reading LSB from register 0x10 for Y-axis reading
    acc.setLow();
    ssp1_exchange_byte(0x0B);
    ssp1_exchange_byte(0x10);
    test = ssp1_exchange_byte(0x00);
    acc.setHigh();

    // Reading LSB from register 0x11 for Y-axis reading
    acc.setLow();
    ssp1_exchange_byte(0x0B);
    ssp1_exchange_byte(0x11);
    test1 = ssp1_exchange_byte(0x00);
    acc.setHigh();
    result = (test1 << 8) + test;
    return result;
}
uint16_t accReadZ()
{
    // Reading LSB from register 0x12 for Z-axis reading
    acc.setLow();
    ssp1_exchange_byte(0x0B);
    ssp1_exchange_byte(0x12);
    test = ssp1_exchange_byte(0x00);
    acc.setHigh();

    // Reading MSB from register 0x13 for Z-axis reading
    acc.setLow();
    ssp1_exchange_byte(0x0B);
    ssp1_exchange_byte(0x13);
    test1 = ssp1_exchange_byte(0x00);
    acc.setHigh();
    result = (test1 << 8) + test;
    return result;
}

#endif /* ADXL362_H_ */
