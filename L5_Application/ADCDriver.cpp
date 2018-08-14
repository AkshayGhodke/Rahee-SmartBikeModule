/*
 * ADCDriver.cpp
 *
 *  Created on: Mar 4, 2018
 *      Author: aksha
 */

#include <LPC17xx.h>
#include "ADCDriver.hpp"

#include "ADCDriver.hpp"

ADCDriver::ADCDriver() {

}

 /**
    * 1) Powers up ADC peripheral
    * 2) Set peripheral clock
    * 3) Enable ADC
    * 4) Select ADC channels
    * 5) Enable burst mode
    */
void ADCDriver::adcInitBurstMode() {
    //Power
    LPC_SC->PCONP |= (1<<12); //PCADC
    LPC_ADC->ADCR |= (1<<21); //PDN

    //Peripheral clock
    LPC_SC->PCLKSEL0 |= (2<<25); //CCLK/2
    LPC_ADC->ADCR |= (1<<9); // 0X10

    LPC_ADC->ADCR &= ~( (1 << 26) | (1 << 25) | (1 << 24) );

    //Enable burst mode
    LPC_ADC->ADCR |= (1<<16);

}

/**
   * 1) Selects ADC functionality of any of the ADC pins that are ADC capable
   *
   * @param adc_pin_arg is the ADC_PIN enumeration of the desired pin.
   *
   * WARNING: For proper operation of the SJOne board, do NOT configure any pins
   *           as ADC except for 0.26, 1.31, 1.30
   */
void ADCDriver::adcSelectPin(ADC_PIN adc_pin_arg) {
    printf("PIN: %d\n",adc_pin_arg);
    switch(adc_pin_arg) {
    case 0:
        LPC_PINCON->PINSEL1 |= (1<<18); //AD0.2
        LPC_ADC->ADCR |= (1<<2);
        break;

    case 1:
        LPC_PINCON->PINSEL1 |= (1<<20); //ADO.3
        LPC_ADC->ADCR |= (1<<3);
        break;

    case 2:
        LPC_PINCON->PINSEL3 |= ((1<<29) | (1<<28)); //ADO.4
        LPC_ADC->ADCR |= (1<<4);
        break;

    case 3:
        LPC_PINCON->PINSEL3 |= ((1<<31) | (1<<30)); //ADO.5
        LPC_ADC->ADCR |= (1<<5);
        break;
    }
}


/**
* 1) Returns the voltage reading of the 12bit register of a given ADC channel
*
* @param adc_channel_arg is the number (0 through 7) of the desired ADC channel.
*/
float ADCDriver::readADCVoltageByChannel(uint8_t adc_channel_arg) {
    float val = 0.0;
    switch(adc_channel_arg) {
        case 2:
            if(LPC_ADC->ADDR2 & (1<<31))
                val = (LPC_ADC->ADDR2>>4) & 0XFFF;
            break;

        case 3:
            if(LPC_ADC->ADDR3 & (1<<31))
                val = (LPC_ADC->ADDR3>>4) & 0XFFF;
            break;

        case 4:
            printf("In 4\n");
            if(LPC_ADC->ADDR4 & (1<<31))
                val = (LPC_ADC->ADDR4>>4) & 0XFFF;
            break;

        case 5:
            printf("In 5\n");
            if(LPC_ADC->ADDR5 & (1<<31))
                val = (LPC_ADC->ADDR5>>4) & 0XFFF;
            break;
    }
    return val;
}


