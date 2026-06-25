/*
 *  QueueStatusService.h
 *
 *  Created on: May 19, 2026
 *  Author: Henry
 */

#ifndef SERVICE_QUEUESTATUSSERVICE_H_
#define SERVICE_QUEUESTATUSSERVICE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

/**	@brief Queue Status Snapshot
 *
 * 	@details
 * 	保存目前 RTOS Queue 的 Count / Space 狀態
 * 	CLI Queue Command 會讀取此 Snapshot 並輸出
 */
typedef struct
{
	uint32_t SensorEventQueueCount;
	uint32_t SensorEventQueueSpace;

	uint32_t DistanceQueueCount;
	uint32_t DistanceQueueSpace;

	uint32_t SensorControlQueueCount;
	uint32_t SensorControlQueueSpace;

	uint32_t BuzzerControlQueueCount;
	uint32_t BuzzerControlQueueSpace;

	uint8_t IsRegistered;

} QueueStatusSnapshot_t;

void QueueStatus_Register(osMessageQueueId_t SensorEventQueue, osMessageQueueId_t Distance, osMessageQueueId_t SensorControlQueue, osMessageQueueId_t BuzzerControlQueue);
bool QueueStatus_GetSnapshot(QueueStatusSnapshot_t *Snapshot);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_QUEUESTATUSSERVICE_H_ */
