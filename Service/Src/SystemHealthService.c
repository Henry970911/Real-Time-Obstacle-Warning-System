/*
 * 	SystemHealthService.c
 *
 *  Created on: Jun 1, 2026
 *  Author: CPC
 */

#include <string.h>
#include "SystemHealthService.h"

/// 此 static 變數用來保存所有 runtime system error counters。
/// 外部模組不能直接存取此變數，必須透過 SystemHealthService API 操作。
static SystemErrorSnapshot_t SystemErrorSnapshot;

/**	@brief	初始化 SystemHealthService。
 *
 * 	@details
 * 	Step 1: 清除內部系統錯誤統計資料
 * 	此函式沒有外部輸入參數, 因此不需要進行參數檢查
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_Init(void)
{
	memset(&SystemErrorSnapshot, 0, sizeof(SystemErrorSnapshot));
}

/**	@brief	重置所有系統錯誤統計資料
 *
 * 	@details
 * 	Step 1: 清除內部系統錯誤統計資料
 * 	判斷條件: 當 CLI 或系統邏輯需要重新統計錯誤時, 可呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_Reset(void)
{
	memset(&SystemErrorSnapshot, 0, sizeof(SystemErrorSnapshot));
}

/**	@brief	取得目前系統錯誤統計快照
 *
 * 	@details
 *  Step 1: 檢查 snapshot 指標是否有效
 * 	Step 2: 將內部錯誤統計資料複製到外部 buffer
 *
 * 	判斷條件:
 * 	如果 snapshot 為 NULL，回傳 false
 * 	如果 snapshot 有效, 完成複製後回傳 true
 *
 * 	@param	snapshot 輸出的錯誤統計快照 buffer
 *
 * 	@return true 表示複製成功 : false 表示傳入參數為 NULL
 */
bool SystemHealth_GetSnapshot(SystemErrorSnapshot_t *Snapshot)
{
	if (Snapshot == NULL)
	{
		return false;
	}

	memcpy(Snapshot, &SystemErrorSnapshot, sizeof(SystemErrorSnapshot));

	return true;
}

/**	@brief	紀錄 Sensor 距離讀取失敗
 *
 * 	@details
 * 	Step 1: 將 SensorReadFailCount 加 1
 *
 * 	判斷條件: 當 VL53L1X distance read failed 時呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordSensorReadFail(void)
{
	SystemErrorSnapshot.SensorReadFailCount++;
}

/**	@brief	紀錄 Sensor DataReady timeout
 *
 * 	@details
 *  Step 1: 將 SensorDataReadyTimeoutCount 加 1
 *
 * 	判斷條件: 當 Sensor DataReady event 在預期時間內沒有發生時呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordSensorDataReadyTimeout(void)
{
	SystemErrorSnapshot.SensorDataReadyTimeoutCount++;
}

/**	@brief	紀錄 Sensor reconnect 嘗試
 *
 * 	@details
 *  Step 1: 將 SensorReconnectCount 加 1
 *
 * 	判斷條件: 當系統或 CLI 要求 sensor reconnect 時呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordSensorReconnect(void)
{
	SystemErrorSnapshot.SensorReconnectCount++;
}

/**	@brief	紀錄 Sensor reconnect 失敗
 *
 * 	@details
 *  Step 1: 將 SensorReconnectFailCount 加 1
 *
 *  判斷條件: 當 sensor reconnect flow 執行失敗時呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordSensorReconnectFail(void)
{
	SystemErrorSnapshot.SensorReconnectFailCount++;
}

/**	@brief	紀錄一般 I2C 錯誤
 *
 * 	@details
 * 	Step 1: 將 I2cErrorCount 加 1
 *
 * 	判斷條件: 當 HAL I2C 操作回傳 error 時呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordI2cError(void)
{
	SystemErrorSnapshot.I2cErrorCount++;
}

/**	@brief	紀錄 I2C timeout
 *
 * 	@details
 *  Step 1: 將 I2cTimeoutCount 加 1
 *
 * 	判斷條件: 當 HAL I2C 操作回傳 timeout 時呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordI2cTimeout(void)
{
	SystemErrorSnapshot.I2cTimeoutCount++;
}

/**	@brief	紀錄 RTOS Queue overflow 或 queue send failed
 *
 * 	@details
 *  Step 1: 將 QueueOverflowCount 加 1
 *
 * 	判斷條件: 當 osMessageQueuePut failed 或 queue 已滿時呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordQueueOverflow(void)
{
	SystemErrorSnapshot.QueueOverFlowCount++;
}

/**	@brief	紀錄 UART RX Ring Buffer overflow
 *
 * 	@details
 * 	Step 1: 將 UartRxOverflowCount 加 1
 *
 * 	判斷條件: 當 UART RX Ring Buffer 無法再保存新的接收資料時呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordUartRxOverflow(void)
{
	SystemErrorSnapshot.UartRxOverFlowCount++;
}

/**	@brief	紀錄 UART TX busy。
 *
 * 	@details
 * 	將 UartTxBusyCount 加 1
 * 	當 UART TX DMA 正忙, 導致新的 TX request 無法被接受時呼叫此函式
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordUartTxBusy(void)
{
	SystemErrorSnapshot.UartRxOverFlowCount++;
}
