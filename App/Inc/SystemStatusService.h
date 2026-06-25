/*
 * 	SystemStatusService.h
 *
 *  Created on: 2026.05.17
 *  Author: Henry
 *
 *	架構圖流程:
 *	------------------------------------------
 *	SensorTask
 *		| --> Distance
 *	SystemStatus_UpdateDistance()
 *	------------------------------------------
 *
 *	------------------------------------------
 *	WarningTask
 *		| --> warning level / buzzer state
 *	SystemStatus_UpdateWarningLevel()
 *	SystemStatus_UpdateBuzzerState()
 *	------------------------------------------
 *
 *	------------------------------------------
 *	CLI command
 *		|
 *	SystemStatus_GetSnapshot()
 *		|
 *	Debug_Print()
 *	------------------------------------------
 */

#ifndef SERVICE_SYSTEMSTATUSSERVICE_H_
#define SERVICE_SYSTEMSTATUSSERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	SYSTEM_WARNING_LEVEL_SAFE = 0,
	SYSTEM_WARNING_LEVEL_CAUTION,
	SYSTEM_WARNING_LEVEL_WARNING,
	SYSTEM_WARNING_LEVEL_DANGER,
	SYSTEM_WARNING_LEVEL_UNKNOWN
} SystemWarningLevel_t;

typedef struct
{
	uint16_t Distance_Mm;
	SystemWarningLevel_t WarningLevel;
	uint8_t BuzzerActive;
	uint32_t SensorUpdateCount;
	uint32_t WarningUpdateCount;
} SystemStatusSnapshot_t;

void SystemStatus_Init(void);
void SystemStatus_UpdateDistance(uint16_t Distance_Mm);
void SystemStatus_UpdateWarningLevel(SystemWarningLevel_t Level);
void SystemStatus_UpdateBuzzerState(uint8_t Active);
bool SystemStatus_GetSnapshot(SystemStatusSnapshot_t *Snapshot);
const char* SystemStatus_WarningLevelToString(SystemWarningLevel_t Level);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_SYSTEMSTATUSSERVICE_H_ */
