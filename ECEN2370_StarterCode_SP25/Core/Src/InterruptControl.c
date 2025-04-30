/*
 * InterruptControl.c
 *
 *  Created on: Feb 18, 2025
 *      Author: sammankin
 */

#include "interruptControl.h"

void enable_IRQ(uint8_t IRQnum){

	HAL_NVIC_EnableIRQ(IRQnum);

}

void disable_IRQ(uint8_t IRQnum){

	HAL_NVIC_DisableIRQ(IRQnum);

}

void clear_pend_IRQ(uint8_t IRQnum){

	HAL_NVIC_ClearPendingIRQ(IRQnum);

}

void set_pend_IRQ(uint8_t IRQnum);

void EXTI_ClearPendingIRQ (uint8_t pinNum){

	EXTI->PR |= (1 << pinNum);

}

