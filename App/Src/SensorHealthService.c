/*
 * 	SensorHealthService.c
 *
 *  Created on: May 26, 2026
 *  Author: Henry
 */

#include "cmsis_os2.h"
#include "stm32h7xx_hal.h"
#include "SensorHealthService.h"

static volatile SensorState_t Sensor_State = SENSOR_STATE_INIT;
static volatile SensorError_t Last_Error = SENSOR_ERROR_NONE;

static volatile uint32_t Error_Count = 0U;
static volatile uint32_t Retry_Count = 0U;
static volatile uint32_t Consecutive_Error_Count = 0U;

static volatile uint32_t Last_Success_Tick = 0U;
static volatile uint32_t Last_Error_Tick = 0U;

/**	@brief	取得安全 Tick
 *
 * 	@details
 * 	Step 1: 若 RTOS 已啟動, 使用 osKernelGetTickCount
 * 	Step 2: 若 RTOS 尚未啟動, 使用 HAL_GetTick
 */
static uint32_t SensorHealth_GetTickSafe(void)
{
	if (osKernelGetState() == osKernelRunning)
	{
		return osKernelGetTickCount();
	}

	return HAL_GetTick();
}

/** @brief	初始化 Sensor Health Service
 *
 * 	@details
 * 	Step 1: 將 Sensor state 設為 INIT。
 * 	Step 2: 清除錯誤資訊與統計值。
 *
 * 	@return	None
 */
void SensorHealth_Init(void)
{
	Sensor_State = SENSOR_STATE_INIT;
	Last_Error = SENSOR_ERROR_NONE;

	Error_Count = 0U;
	Retry_Count = 0U;
	Consecutive_Error_Count = 0U;

	Last_Success_Tick = 0U;
	Last_Error_Tick = 0U;
}

/**	@brief	設定 Sensor State
 *
 * 	@param	State Sensor 狀態
 *
 * 	@return	None
 */
void SensorHealth_SetState(SensorState_t State)
{
	Sensor_State = State;
}

/**	@brief	回報 Sensor 成功讀取
 *
 * 	@details
 * 	演算法 [Success State Update]
 * 	Step 1 -- 將 Sensor State Update
 * 	Step 2 -- 清除 LastError
 * 	Step 3 -- 更新 LastSuccessTick
 *
 * 	@return	None
 */
void SensorHealth_ReportSuccess(void)
{
	Sensor_State = SENSOR_STATE_ONLINE;
	/// Last_Error = SENSOR_ERROR_NONE;
	Consecutive_Error_Count = 0U;
	Last_Success_Tick = SensorHealth_GetTickSafe();
}

/**	@brief	回報 Sensor 錯誤
 *
 * 	@details
 * 	演算法 [Error Counter Algorithm]
 * 	Step 1: 將 Sensor State 設為 None
 * 	Step 2: 紀錄最後一次錯誤類型
 * 	Step 3: 累加 Error Count
 * 	Step 4: 更新 LastErrorTick
 *
 * 	@param	Error Sensor	錯誤類型
 *
 * 	@return	None
 */
void SensorHealth_ReportError(SensorError_t Error)
{
	Sensor_State = SENSOR_STATE_ERROR;
	Last_Error = Error;
	Error_Count++;
	Consecutive_Error_Count++;
	Last_Error_Tick = SensorHealth_GetTickSafe();
}

/**	@brief	回報 Sensor retry
 *
 * 	@details
 * 	演算法 [Retry Counter Algorithm]
 * 	Step 1: 將 Sensor state 設為 RETRYING。
 * 	Step 2: 累加 RetryCount。
 *
 * 	@return	None
 */
void SensorHealth_ReportRetry(void)
{
	Sensor_State = SENSOR_STATE_RETRYING;
	Retry_Count++;
}

/**	@brief	設定 Sensor 為 OFFLINE
 *
 * 	@details
 * 	演算法 [Offline State Transition]
 *
 * 	Step 1: 將 Sensor state 設為 OFFLINE。
 * 	Step 2: 保留 LastError，供 CLI 診斷。
 *
 * 	@return	None
 */
void SensorHealth_SetOffline(void)
{
	Sensor_State = SENSOR_STATE_OFFLINE;
}

/**	@brief	取得 Sensor 連續錯誤次數
 *
 * 	@details
 * 	用於 SensorTask 判斷是否要切換到 OFFLINE。
 *
 * 	@return	uint32_t	consecutive error count
 */
uint32_t SensorHealth_GetConsecutiveErrorCount(void)
{
	return Consecutive_Error_Count;
}

/**	@brief 取得 Sensor Health Snapshot
 *
 * 	@details
 * 	演算法 [Snapshot Read Pattern]
 * 	Step 1: 檢查 Snapshot 指標是否有效
 * 	Step 2: 複製目前 Sensor Health 資訊
 *
 * 	@param	Snapshot	Sensor Health Snapshot output
 *
 * 	@return	bool	true : 取得成功 ? false : Snapshot == NULL
 */
bool SensorHealth_GetSnapshot(SensorHealthSnapshot_t *Snapshot)
{
	if (Snapshot == NULL)
	{
		return false;
	}

	Snapshot->State = Sensor_State;
	Snapshot->LastError = Last_Error;
	Snapshot->ErrorCount = Error_Count;
	Snapshot->RetryCount = Retry_Count;
	Snapshot->ConsecutiveErrorCount = Consecutive_Error_Count;
	Snapshot->LastSuccessTick = Last_Success_Tick;
	Snapshot->LastErrorTick = Last_Error_Tick;

	return true;
}

/**	@brief	Sensor	State to String
 *
 * 	@param	State	Sensor State
 *
 * 	@return	const char*
 */
const char* SensorHealth_StateToString(SensorState_t State)
{
	switch (State)
	{

	case SENSOR_STATE_INIT:
		return "INIT";

	case SENSOR_STATE_ONLINE:
		return "ONLINE";

	case SENSOR_STATE_ERROR:
		return "ERROR";

	case SENSOR_STATE_OFFLINE:
		return "OFFLINE";

	case SENSOR_STATE_RETRYING:
		return "RETRYING";

	default:
		return "UNKNOWN";
	}
}

/**	@brief	Sensor error to string
 *
 * 	@param 	Error	Sensor error
 *
 * 	@return const char*
 */
const char* SensorHealth_ErrorToString(SensorError_t Error)
{
	switch (Error)
	{
	case SENSOR_ERROR_NONE:
		return "NONE";

	case SENSOR_ERROR_BOOT_STATE:
		return "BOOT_STATE_FAILED";

	case SENSOR_ERROR_SENSOR_INIT:
		return "SENSOR_INIT_FAILED";

	case SENSOR_ERROR_START_RANGING:
		return "START_RANGING_FAILED";

	case SENSOR_ERROR_CHECK_DATA_READY:
		return "CHECK_DATA_READY_FAILED";

	case SENSOR_ERROR_GET_DISTANCE:
		return "GET_DISTANCE_FAILED";

	case SENSOR_ERROR_CLEAR_INTERRUPT:
		return "CLEAR_INTERRUPT_FAILED";

	case SENSOR_ERROR_I2C_PLATFORM:
		return "I2C_PLATFORM_FAILED";

	default:
		return "UNKNOWN";
	}
}
