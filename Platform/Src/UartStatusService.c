/*
 * 	UartStatusService.c
 *
 *  Created on: May 21, 2026
 *  Author: Henry
 */

#include "UartStatusService.h"

static volatile uint32_t Rx_CallBackCount = 0U;
static volatile uint32_t Rx_OverFlowCount = 0U;
static volatile uint16_t Last_Rx_Size = 0U;

/**
 * 	@brief 初始化 UART Status Service
 *
 * 	@details
 * 	清除 Rx_CallBackCount / Rx_OverFlowCount / Last_Rx_Size
 *
 * 	@param None
 *
 * 	@return None
 */
void UartStatus_Init(void)
{
	Rx_CallBackCount = 0U;
	Rx_OverFlowCount = 0U;
	Last_Rx_Size = 0U;
}

/**
 * 	@brief 記錄一次 UART RX DMA Event
 *
 * 	@details 演算法 [Counter Accumulation]
 *
 * 	Step 1: 當 HAL_UARTEx_RxEventCallback() 發生時呼叫
 * 	Step 2: 累加 RX Callback Count
 * 	Step 3: 保存本次 RX DMA 接收長度
 *
 * 	@param Size 本次 Receive-To-Idle DMA 收到的資料長度
 *
 * 	@return None
 */
void UartStatus_OnRxEvent(uint16_t Size)
{
	Rx_CallBackCount++;
	Last_Rx_Size = Size;
}

/**
 * 	@brief 累加 UART RX overflow counter
 *
 * 	@details 演算法 [Overflow Counting]
 *
 * 	Step 1. 當 Ring Buffer 寫入失敗時呼叫
 * 	Step 2. 累加 RxOverflowCount，供 CLI uart command 查詢
 *
 * 	@param None
 *
 * 	@return None
 */
void UartStatus_AddOverflow(void)
{
	Rx_OverFlowCount++;
}

/**
 * 	@brief 取得 UART status snapshot
 *
 * 	@details 演算法 [Snapshot Read Algorithm]
 *
 * 	Step 1. 檢查 Snapshot 指標是否有效
 * 	Step 2. 複製 volatile counter 到輸出 snapshot
 *
 * 	@param Snapshot 輸出 UART status snapshot。
 *
 * 	@return bool
 * 	- true: 取得成功
 * 	- false: Snapshot == NULL
 */
bool UartStatus_GetSnapshot(UartStatusSnapshot_t *Snapshot)
{
	if (Snapshot == NULL)
	{
		return false;
	}

	Snapshot->RxCallbackCount = Rx_CallBackCount;
	Snapshot->RxOverflowCount = Rx_OverFlowCount;
	Snapshot->LastRxSize = Last_Rx_Size;

	return true;
}
