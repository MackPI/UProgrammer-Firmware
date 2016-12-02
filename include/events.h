/*
 * events.h
 *
 *  Created on: Sep 2, 2016
 *      Author: Mack
 */

#ifndef EVENTS_H_
#define EVENTS_H_

#include "user_interface.h"

void ICACHE_FLASH_ATTR
taskHandler(os_event_t *);
void ICACHE_FLASH_ATTR
init_adc_timer();
os_timer_t tadc;
bool display_now;
uint16 adcValue;

#endif /* EVENTS_H_ */
