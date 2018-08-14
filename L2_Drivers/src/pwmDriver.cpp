/*
 * pwmDriver.cpp
 *
 *  Created on: Mar 4, 2018
 *      Author: akshay
 */

#include <LPC17xx.h>
#include "pwmDriver.hpp"


void PWMDriver::pwmSelectAllPins(){
    // Selecting all PWM compatible PINS
    LPC_PINCON->PINSEL4 &= ~((3 << 10) | (3 << 8) | (3 << 6) | (3 << 4) | (3 << 2) | (3 << 0));
    LPC_PINCON->PINSEL4 |= ((1 << 10) | (1 << 8) | (1 << 6) | (1 << 4) | (1 << 2) | (1 << 0));
}

void PWMDriver::pwmSelectPin(PWM_PIN pwm_pin_arg){
    LPC_PINCON->PINSEL4 &= ~( 3 << ( pwm_pin_arg * 2 ));
    LPC_PINCON->PINSEL4 |= ( 1 << ( pwm_pin_arg * 2 ) );
}

void PWMDriver::pwmInitSingleEdgeMode(uint32_t frequency_Hz){
    // Peripheral Clock Select PWMCLK = CCLK (13:12 -> 0:1)
    LPC_SC->PCLKSEL0 &= ~(3 << 12);
    LPC_SC->PCLKSEL0 |= (2 << 12);

    // Setting frequency using pre-scaler register
    LPC_PWM1->PR = 47;
    LPC_PWM1->TC = 0;

    // MR0 = Cclk/freq

    // Setting up values
    LPC_PWM1->MR0 = 1000000/frequency_Hz;
    LPC_PWM1->LER |= ( 1 << 0 );
    LPC_PWM1->PCR |= ( 1 << 9 );

    // Set MR0 to reset
    LPC_PWM1->MCR |= ( 1 << 1 );

    // PWM enable and Counter enable
    LPC_PWM1->TCR |= ((1 << 0) | (1 << 3));

}

void PWMDriver::setDutyCycle(PWM_PIN pwm_pin_arg, float duty_cycle_percentage){
    // y = (MR0 * %DC)/100
    duty_cycle_percentage = (LPC_PWM1->MR0 * duty_cycle_percentage)/100;

    // Output enable
    LPC_PWM1->PCR |= ( 1 << (pwm_pin_arg + 9) );

    switch(pwm_pin_arg){
        case 0:
            LPC_PWM1->MR1 = duty_cycle_percentage;
            break;
        case 1:
            LPC_PWM1->MR2 = duty_cycle_percentage;
            break;
        case 2:
            LPC_PWM1->MR3 = duty_cycle_percentage;
            break;
        case 3:
            LPC_PWM1->MR4 = duty_cycle_percentage;
            break;
        case 4:
            LPC_PWM1->MR5 = duty_cycle_percentage;
            break;
        case 5:
            LPC_PWM1->MR6 = duty_cycle_percentage;
            break;
        default:
            LPC_PWM1->MR1 = duty_cycle_percentage;
            break;
    }
    // Latch Enable match register to be effective in next cycle , might require separate function
    LPC_PWM1->LER |= ( 1 << (pwm_pin_arg + 1) );
}

void PWMDriver::setFrequency(uint32_t frequency_Hz){
    LPC_PWM1->MR0 = 1000000/frequency_Hz;
}
