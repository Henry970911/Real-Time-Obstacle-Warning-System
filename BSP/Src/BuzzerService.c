/*
 *  Buzzer_service.c
 *
 *  Created on: May 11, 2026
 *  Author: CPC
 */

#include "BuzzerService.h"

void Buzzer_Init(void)
{
	HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
}

void Buzzer_On(void)
{
	HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
}

void Buzzer_Off(void)
{
	HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
}

void Buzzer_Toggle(void)
{
	HAL_GPIO_TogglePin(Buzzer_GPIO_Port, Buzzer_Pin);
}

void Buzzer_BeepBlocking(uint32_t Duration_ms)
{
	Buzzer_On();
	HAL_Delay(Duration_ms);
	Buzzer_Off();
}
