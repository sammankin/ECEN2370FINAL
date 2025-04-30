/*
 * Scheduler.c
 *
 *  Created on: Jan 28, 2025
 *      Author: sammankin
 */

#include <Scheduler.h>
#include <stdint.h>

static uint32_t scheduledEvents;

uint32_t getScheduledEvents(){

	return scheduledEvents;

}

void addSchedulerEvent(uint32_t input){

	scheduledEvents |= input;

}

void removeSchedulerEvent(uint32_t input){

	scheduledEvents &= ~input;

}

