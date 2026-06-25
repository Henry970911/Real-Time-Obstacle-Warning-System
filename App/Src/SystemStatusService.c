/*
 * 	SystemStatusService.c
 *
 *  Created on: 2026.05.17
 *  Author: Henry
 */

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "SystemStatusService.h"

static SystemStatusSnapshot_t SystemStatusSnapshot;

/**
 * 	@brief 初始化系統狀態服務
 *
 * 	@details
 * 	Step 1. 將 DistanceMm 設為 0
 * 	Step 2. 將 WarningLevel 設為 UNKNOWN
 * 	Step 3. 將 BuzzerActive 設為 0
 * 	Step 4. 清除 SensorUpdateCount
 * 	Step 5. 清除 WarningUpdateCount
 *
 * 	@param None
 *
 * 	@return None
 */
void SystemStatus_Init(void)
{
	SystemStatusSnapshot.Distance_Mm = 0U;
	SystemStatusSnapshot.WarningLevel = SYSTEM_WARNING_LEVEL_UNKNOWN;
	SystemStatusSnapshot.BuzzerActive = 0U;
	SystemStatusSnapshot.SensorUpdateCount = 0U;
	SystemStatusSnapshot.WarningUpdateCount = 0U;
}

/**
 * 	@brief 更新目前距離
 *
 * 	@details
 * 	演算法名稱：Snapshot Update Algorithm
 *
 * 	Step 1. 使用 taskENTER_CRITICAL() 保護 shared data。
 * 	Step 2. 更新 DistanceMm。
 * 	Step 3. 增加 SensorUpdateCount。
 * 	Step 4. 使用 taskEXIT_CRITICAL() 結束保護。
 *
 * 	@param DistanceMm 目前距離，單位 mm。
 *
 * 	@return None
 */
void SystemStatus_UpdateDistance(uint16_t Distance_Mm)
{
	taskENTER_CRITICAL();

	SystemStatusSnapshot.Distance_Mm = Distance_Mm;
	SystemStatusSnapshot.SensorUpdateCount++;

	taskEXIT_CRITICAL();
}

/**
 * 	@brief 更新目前警示等級
 *
 * 	@details
 * 	演算法名稱：Snapshot Update Algorithm
 *
 * 	Step 1. 使用 taskENTER_CRITICAL() 保護 shared data。
 * 	Step 2. 更新 WarningLevel。
 * 	Step 3. 增加 WarningUpdateCount。
 * 	Step 4. 使用 taskEXIT_CRITICAL() 結束保護。
 *
 * 	@param Level 目前警示等級。
 *
 * 	@return None
 */
void SystemStatus_UpdateWarningLevel(SystemWarningLevel_t Level)
{
	taskENTER_CRITICAL();

	SystemStatusSnapshot.WarningLevel = Level;
	SystemStatusSnapshot.WarningUpdateCount++;

	taskEXIT_CRITICAL();
}

/**
 * 	@brief 更新蜂鳴器狀態
 *
 * 	@details
 * 	Step 1. 使用 taskENTER_CRITICAL() 保護 shared data。
 * 	Step 2. 更新 BuzzerActive。
 * 	Step 3. 使用 taskEXIT_CRITICAL() 結束保護。
 *
 * 	@param Active
 * 	- 0: inactive
 * 	- 1: active
 *
 * 	@return None
 */
void SystemStatus_UpdateBuzzerState(uint8_t Active)
{
	taskENTER_CRITICAL();

	SystemStatusSnapshot.BuzzerActive = Active;

	taskEXIT_CRITICAL();
}

/**
 * 	@brief 取得目前系統狀態快照
 *
 * 	@details
 * 	演算法名稱：Snapshot Read Algorithm
 *
 * 	Step 1. 檢查 Snapshot 是否為 NULL。
 * 	Step 2. 使用 taskENTER_CRITICAL() 保護 shared data。
 * 	Step 3. 複製 SystemStatusSnapshot。
 * 	Step 4. 使用 taskEXIT_CRITICAL() 結束保護。
 *
 * 	@param Snapshot 輸出 snapshot。
 *
 * 	@return bool
 * 	- true: 取得成功
 * 	- false: Snapshot == NULL
 */
bool SystemStatus_GetSnapshot(SystemStatusSnapshot_t *Snapshot)
{
	if (Snapshot == NULL)
	{
		return false;
	}

	taskENTER_CRITICAL();

	*Snapshot = SystemStatusSnapshot;

	taskEXIT_CRITICAL();

	return true;
}

/**	20260517 Version
 *
 *	@brief 將警示系統轉成字串
 *
 *	@details
 *	演算法名稱: Enum-to-String Mapping
 *
 *	@param Level 警示等級(測距)
 *
 *	@return const char* 警示等級字串
 */
const char* SystemStatus_WarningLevelToString(SystemWarningLevel_t Level)
{
	const char *Text;

	switch (Level)
	{
	case SYSTEM_WARNING_LEVEL_SAFE:
		Text = "SAFE";
		break;

	case SYSTEM_WARNING_LEVEL_CAUTION:
		Text = "CAUTION";
		break;

	case SYSTEM_WARNING_LEVEL_WARNING:
		Text = "WARNING";
		break;

	case SYSTEM_WARNING_LEVEL_DANGER:
		Text = "DANGER";
		break;

	default:
		Text = "UNKNOWN";
		break;
	}

	return Text;
}
