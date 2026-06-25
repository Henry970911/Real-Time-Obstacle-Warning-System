/*
 * 	SensorHealthService.h
 *
 *  Created on: May 26, 2026
 *  Author: Henry
 */

#ifndef SERVICE_SENSORHEALTHSERVICE_H_
#define SERVICE_SENSORHEALTHSERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/// VL53L1X Sensor 目前狀態
typedef enum
{
	SENSOR_STATE_INIT = 0,
	SENSOR_STATE_ONLINE,
	SENSOR_STATE_ERROR,
	SENSOR_STATE_OFFLINE,
	SENSOR_STATE_RETRYING
} SensorState_t;

/// 紀錄 Sensor 最近一次錯誤來源
typedef enum
{
	SENSOR_ERROR_NONE = 0,
	SENSOR_ERROR_BOOT_STATE,
	SENSOR_ERROR_SENSOR_INIT,
	SENSOR_ERROR_START_RANGING,
	SENSOR_ERROR_CHECK_DATA_READY,
	SENSOR_ERROR_GET_DISTANCE,
	SENSOR_ERROR_CLEAR_INTERRUPT,
	SENSOR_ERROR_I2C_PLATFORM
} SensorError_t;

typedef struct
{
	SensorState_t State;
	SensorError_t LastError;

	uint32_t ErrorCount;
	uint32_t RetryCount;
	uint32_t ConsecutiveErrorCount;

	uint32_t LastSuccessTick;
	uint32_t LastErrorTick;

} SensorHealthSnapshot_t;

void SensorHealth_Init(void);
void SensorHealth_SetState(SensorState_t State);
void SensorHealth_ReportSuccess(void);
void SensorHealth_ReportError(SensorError_t Error);
void SensorHealth_ReportRetry(void);
void SensorHealth_SetOffline(void);
uint32_t SensorHealth_GetConsecutiveErrorCount(void);
bool SensorHealth_GetSnapshot(SensorHealthSnapshot_t *Snapshot);
const char* SensorHealth_StateToString(SensorState_t State);
const char* SensorHealth_ErrorToString(SensorError_t Error);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_SENSORHEALTHSERVICE_H_ */
