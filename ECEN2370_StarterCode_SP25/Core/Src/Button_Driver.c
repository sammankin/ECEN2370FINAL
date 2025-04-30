/*
 * Button_Driver.c
 *
 *  Created on: Feb 13, 2025
 *      Author: sammankin
 */


#include "Button_Driver.h"

void buttonInit(){

	GPIO_InitTypeDef pinConfig;
	pinConfig.Pin = buttonPinNumber;
	pinConfig.Mode = GPIO_MODE_INPUT;
	pinConfig.Speed = GPIO_SPEED_LOW;
	pinConfig.Pull = GPIO_PULLUP;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &pinConfig);

}

void buttonInit_InterruptMode(){

	GPIO_InitTypeDef pinConfig;
	pinConfig.Pin = GPIO_PIN_0;
	pinConfig.Mode = GPIO_MODE_IT_RISING;
	pinConfig.Speed = GPIO_SPEED_LOW;
	pinConfig.Pull = GPIO_NOPULL;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &pinConfig);
	enable_IRQ(EXTI0_IRQn);

}

bool buttonPressed(){

	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == ButtonPressed){
		return true;
	}
	else{
		return false;
	}

}
