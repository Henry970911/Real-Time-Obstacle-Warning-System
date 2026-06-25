/*
 *  Buzzer_service.h
 *
 *  Created on: May 11, 2026
 *  Author: CPC
 */

#ifndef SERVICE_BUZZERSERVICE_H_
#define SERVICE_BUZZERSERVICE_H_

#include <stdint.h>

#include "gpio.h"

void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Init(void);
void Buzzer_Toggle(void);
void Buzzer_BeepBlocking(uint32_t Duration_ms);

#endif /* SERVICE_BUZZERSERVICE_H_ */
