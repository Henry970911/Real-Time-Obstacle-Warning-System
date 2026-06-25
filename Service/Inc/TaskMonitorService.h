/*
 * 	TaskMonitorService.h
 *
 *  Created on: May 28, 2026
 *  Author: Henry
 */

#ifndef CLISERVICE_TASKMONITORSERVICE_H_
#define CLISERVICE_TASKMONITORSERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

/**	@brief
 *
 * 	@details
 * 	保存各 RTOS Task 的 Stack High Water Mark
 * 	數值越小, 代表剩餘 Stack 越少
 *
 */
typedef struct
{
	uint32_t SensorTaskStackWords;
	uint32_t WarningTaskStackWords;
	uint32_t CommunicationTaskStackWords;
} TaskMonitorSnapshot_t;

void TaskMonitor_Init(void);

void TaskMonitor_Register(osThreadId_t SensorTaskHandle, osThreadId_t WarningTaskHandle, osThreadId_t CommunicationTaskHandle);

bool TaskMonitor_GetSnapshot(TaskMonitorSnapshot_t *Snapshot);

#ifdef __cplusplus
}
#endif

#endif /* CLISERVICE_TASKMONITORSERVICE_H_ */
