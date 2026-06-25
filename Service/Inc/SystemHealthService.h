/*
 * 	SystemHealthService.h
 *
 *  Created on: Jun 1, 2026
 *  Author: Henry
 */

#ifndef SYSTEMHEALTHSERVICE_H_
#define SYSTEMHEALTHSERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/**
 * 	@brief 系統錯誤統計結構
 *
 * 	@details
 * 	This structure stores system-level error counters.
 * 	It is used by the monitor command and long-run test diagnostics.
 *
 * 	Counter meaning:
 * 	- SensorReadFailCount: 			VL53L1X 距離讀取失敗次數
 * 	- SensorDataReadyTimeoutCount:	Sensor DataReady 逾時次數
 * 	- SensorReconnectCount:			Sensor reconnect 請求或嘗試次數
 * 	- SensorReconnectFailCount:		Sensor reconnect 失敗次數
 * 	- I2cErrorCount:				一般 I2C 錯誤次數
 * 	- I2cTimeoutCount:				I2C timeout 次數
 * 	- QueueOverflowCount:			RTOS Queue 滿載、overflow 或 queue send failed 次數
 * 	- UartRxOverflowCount:			UART RX Ring Buffer overflow 次數
 * 	- UartTxBusyCount:				UART TX DMA busy 或送出請求被拒絕次數
 */
typedef struct
{
	uint32_t SensorReadFailCount;
	uint32_t SensorDataReadyTimeoutCount;
	uint32_t SensorReconnectCount;
	uint32_t SensorReconnectFailCount;

	uint32_t I2cErrorCount;
	uint32_t I2cTimeoutCount;

	uint32_t QueueOverFlowCount;
	uint32_t UartRxOverFlowCount;
	uint32_t UartTxBusyCount;
} SystemErrorSnapshot_t;

/**	@brief	初始化 SystemHealthService
 *
 * 	@details
 * 	清除所有系統錯誤統計資料, RTOS scheduler 啟動前呼叫一次
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_Init(void);

/**	@brief	重置所有系統錯誤統計資料
 *
 * 	@details
 * 	將所有 runtime error counter 清 0, 由 CLI 指令呼叫, Example: :error reset" or "monitor reset"
 *
 * 	@param	None
 * 	@return	None
 */
void SystemHealth_Reset(void);

/**	@brief	取得目前系統錯誤統計快照
 *
 * 	@details
 * 	此函式會將內部錯誤統計資料複製到外部傳入的 snapshot buffer
 *
 * 	@param	snapshot 輸出的錯誤統計快照 buffer
 * 	@return	true 表示複製成功；false 表示傳入參數為 NULL
 */
bool SystemHealth_GetSnapshot(SystemErrorSnapshot_t *Snapshot);

/**	@brief	紀錄 Sensor 距離讀取失敗。
 *
 * 	@details
 * 	將 SensorReadFailCount 加 1
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordSensorReadFail(void);

/**	@brief	紀錄 Sensor DataReady timeout
 *
 * 	@details
 * 	此函式會將 SensorDataReadyTimeoutCount 加 1
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordSensorDataReadyTimeout(void);

/**	@brief	紀錄 Sensor reconnect 嘗試
 *
 * 	@details
 * 	此函式會將 SensorReconnectCount 加 1
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordSensorReconnect(void);

/**	@brief	紀錄 Sensor reconnect 失敗
 *
 * 	@details
 * 	此函式會將 SensorReconnectFailCount 加 1
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordSensorReconnectFail(void);

/**	@brief	紀錄一般 I2C 錯誤
 *
 * 	@details
 * 	此函式會將 I2cErrorCount 加 1
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordI2cError(void);

/**
 * 	@brief	紀錄 I2C timeout
 *
 * 	@details
 * 	此函式會將 I2cTimeoutCount 加 1
 *
 * 	@param	None
 * 	@return None
 */
void SystemHealth_RecordI2cTimeout(void);

/**	@brief	紀錄 RTOS Queue overflow 或 queue send failed
 *
 * 	@details
 * 	此函式會將 QueueOverflowCount 加 1
 *
 * 	@param	None
 * 	@return	None
 */
void SystemHealth_RecordQueueOverflow(void);

/**	@brief	紀錄 UART RX Ring Buffer overflow
 *
 * 	@details
 * 	此函式會將 UartRxOverflowCount 加 1
 *
 * 	@param	None
 * 	@return	None
 */
void SystemHealth_RecordUartRxOverflow(void);

/**	@brief	紀錄 UART TX busy
 *
 * 	@details
 * 	此函式會將 UartTxBusyCount 加 1
 *
 * 	@param	None
 * 	@return	None
 */
void SystemHealth_RecordUartTxBusy(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEMHEALTHSERVICE_H_ */
