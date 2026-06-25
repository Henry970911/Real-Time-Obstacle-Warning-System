/*
 *  uart_dma.c
 *
 *  Created on: Mar 26, 2026
 *  Author: Henry
 */

#include "UartDma.h"

/**	@brief	UART DMA Busy Flag
 *
 *	@details
 *	控制 DMA 傳輸狀態
 *	- 0: Idle (可發送)
 *	- 1: Busy (DMA 傳輸中)
 *	- volatile: 因為此變數會在 中斷(callback) 中被修改
 */
static volatile uint8_t uart_dma_busy = 0;
volatile uint8_t uart_dma_done_flag = 0;
extern DMA_HandleTypeDef hdma_usart3_tx;

/**	@brief	初始化 UART DMA 狀態
 *
 *	@details
 *	- Step 1: 清除 DMA Busy Flag
 *	- Step 2: 清除 DMA Done Flag
 */
void UART_DMA_Init(void)
{
	uart_dma_busy = 0;

	/// 20260409 版本 => 新增 uart_dma_done_flag 預設狀態
	uart_dma_done_flag = 0;
}

/**	@brief	查詢 UART DMA 是否忙碌中
 *
 *	@return
 *	- 1 : Busy
 *	- 0 : Idle
 */
uint8_t UART_DMA_IsBusy(void)
{
	return uart_dma_busy;
}

/**	@brief 透過 UART DMA 非阻塞送出資料
 *
 *	@details
 * 	Step 1: 檢查參數是否合法
 * 	Step 2: 檢查 UART DMA 是否 busy
 * 	Step 3: 進行 DCache clean，避免 STM32H7 cache coherency 問題
 * 	Step 4: 啟動 HAL_UART_Transmit_DMA()
 * 	Step 5: 若 DMA 啟動失敗，需還原 busy flag
 *
 * 	@param Buffer: 傳送資料位址
 * 	@param Len   : 傳送長度
 *
 * 	@return
 * 	- 1: 啟動 DMA 成功
 * 	- 0: 啟動失敗 / Busy / 參數錯誤
 */
uint8_t UART_DMA_Send(uint8_t *Buffer, uint32_t Len)
{
	HAL_StatusTypeDef HalStatus;
	uint32_t Addr;
	uint32_t Size;

	if ((Buffer == NULL) || (Len == 0U))
	{
		return 0;
	}

	/// 20260515 版本更新 => 更改 TX DMA Service, 不適合在 RX DMA Active 時用, HAL_UART_GetState() 判斷整體 UART 狀態
	/// if ((uart_dma_busy != 0U) || (HAL_UART_GetState(&huart3) != HAL_UART_STATE_READY))
	if ((uart_dma_busy != 0U) || (huart3.gState != HAL_UART_STATE_READY))
	{
		return 0;
	}

	uart_dma_busy = 1U;
	uart_dma_done_flag = 0U;

	/// 20260515 版本更新 => STM32H7 DCache Clean
	/// DMA 讀 RAM, 不會讀 DCache
	/// DMA TX 讀 RAM，因此啟動 DMA 前必須將 DCache 寫回 RAM
	Addr = ((uint32_t) Buffer) & ~((uint32_t) 0x1FU);		/// 將位址對齊到 32 bytes (cache line size)
	Size = ((uint32_t) Buffer + Len) - Addr;				/// 計算需要 clean 的大小 (32 bytes)
	Size = (Size + 31U) & ~((uint32_t) 0x1FU);				/// 將 DCache 資料寫回 RAM

	SCB_CleanDCache_by_Addr((uint32_t*) Addr, Size);

	HalStatus = HAL_UART_Transmit_DMA(&huart3, Buffer, Len);

	if (HalStatus != HAL_OK)
	{
		uart_dma_busy = 0U;
		return 0U;
	}

	return 1U;
}

/**	@brief UART DMA 傳輸完成 callback (由 HAL 呼叫)
 *
 *	@details
 *	HAL callback (override weak function)
 *	Step 1: DMA 傳輸完成後會進入此 callback
 *	Step 2: 將 busy flag 清除
 *	Step 3: 允許下一次傳輸
 *
 * 	@param huart: UART Handle
 *
 * 	@return None
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart3)
	{
		uart_dma_busy = 0U;
		uart_dma_done_flag = 1U;
	}
}
