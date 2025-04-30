/*
 * InterruptControl.h
 *
 *  Created on: Feb 18, 2025
 *      Author: sammankin
 */

#ifndef INTERRUPTCONTROL_H_
#define INTERRUPTCONTROL_H_

#include <stdint.h>
#include <stm32f4xx.h>

#define EXTI0_IRQ_NUMBER 6

void enable_IRQ(uint8_t IRQnum);
void disable_IRQ(uint8_t IRQnum);
void clear_pend_IRQ(uint8_t IRQnum);
void set_pend_IRQ(uint8_t IRQnum);
void EXTI_ClearPendingIRQ (uint8_t pinNum);

#endif /* INTERRUPTCONTROL_H_ */
