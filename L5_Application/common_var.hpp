/*
 * common_var.hpp
 *
 *  Created on: Mar 20, 2018
 *      Author: akshay
 *      Temp
 */

#include <Lpc17xx.h>
#include "task.h"
#include <stddef.h>

#ifndef COMMON_VAR_HPP_
#define COMMON_VAR_HPP_


extern TaskHandle_t xHandle;
extern TaskHandle_t xHandle1;
extern TaskHandle_t xHandle2;
extern bool flag_external;
extern bool flag_alert;
extern bool flag_bike_lock;

extern bool flag_blinky;
//extern int direction;
//extern int BRIGHTNESS;
//extern TaskHandle_t xProducerTaskHandle;

//extern TaskHandle_t xConsumerTaskHandle;



#endif /* COMMON_VAR_HPP_ */
