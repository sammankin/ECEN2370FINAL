/*
 * Button_Driver.h
 *
 *  Created on: Feb 13, 2025
 *      Author: sammankin
 */

#ifndef BUTTON_DRIVER_H_
#define BUTTON_DRIVER_H_
//#include <stdbool.h>
//#include <stm32f4xx.h>
//#include <interruptControl.h>
//#include <ApplicationCode.h>
#include "stm32f4xx.h"
#include "Scheduler.h"
#include "InterruptControl.h"

#define buttonPortNumber GPIOA
#define buttonPinNumber 0
#define ButtonPressed 1
#define	isButtonPressed 0

void buttonInit();
bool buttonPressed();
void buttonInit_InterruptMode();


#endif /* BUTTON_DRIVER_H_ */
