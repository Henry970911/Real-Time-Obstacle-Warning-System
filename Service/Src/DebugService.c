/*
 *  debug_service.c
 *
 *  Created on: Mar 30, 2026
 *  Author: Henry
 */

#include "DebugService.h"

#define DEBUG_TEMP_SIZE 128U
#define DEBUG_BUFFER_SIZE 2048U
#define DEBUG_TX_BUFFER_SIZE 1024U

static uint8_t DebugBuf[DEBUG_BUFFER_SIZE];
static volatile uint16_t Head = 0U;
static volatile uint16_t Tail = 0U;
static uint8_t Tx_Buffer[DEBUG_TX_BUFFER_SIZE];

/**	@brief 計算 Ring Buffer 剩餘空間
 *
 * 	@return Free Size
 */
static uint16_t Debug_GetFreeSpace(void)
{
	if (Head >= Tail)
	{
		return (uint16_t) (DEBUG_BUFFER_SIZE - Head + Tail - 1U);
	}
	else
	{
		return (uint16_t) (Tail - Head - 1U);
	}
}

/**	@brief 將資料寫入 Ring Buffer
 *
 * 	@details
 * 	Step 1: 進入臨界區, 避免 Head/Tail 被中斷(Interrupt)打斷
 * 	Step 2: 檢查空間是否足夠
 * 	Step 3: 逐 byte 寫入
 *
 * 	@param Data: 輸入資料位址
 * 	@param Len : 長度
 *
 * 	@return None
 */
static void WriteBuffer(const uint8_t *Data, uint16_t Len)
{
	uint16_t Free;

	if ((Data == NULL) || (Len == 0U))
	{
		return;
	}

	__disable_irq();

	Free = Debug_GetFreeSpace();

	if (Free < Len)
	{
		__enable_irq();
		return;
	}

	/// 20260409 修改 => [暫且保留]
	/// ----------------------------------------------------
	//	if (Head >= Tail)
	//		Free = DEBUG_BUFFER_SIZE - Head + Tail - 1;
	//	else
	//		Free = Tail - Head - 1;
	//
	//	if (Free < Len)
	//	{
	//		/// 不寫入 => 保持 Log 完整性
	//		return;
	//	}
	/// ----------------------------------------------------

	for (int i = 0U; i < Len; i++)
	{
		DebugBuf[Head] = Data[i];
		Head = (uint16_t) ((Head + 1U) % DEBUG_BUFFER_SIZE);
	}

	__enable_irq();
}

/**	@brief 初始化 Debug Ring Buffer
 *
 * 	@details
 * 	- Head and Tail 歸零
 */
void Debug_Init(void)
{
	Head = 0U;
	Tail = 0U;
}

/**	@brief 直接將原始資料寫入 Ring Buffer
 *
 * 	@param Data: 原始資料位址
 * 	@param Len : 長度
 */
void Debug_WriteRaw(const uint8_t *Data, uint16_t Len)
{
	WriteBuffer(Data, Len);
}

/**	@brief  格式化輸出 Debug 訊息到 Ring Buffer
 *
 * 	@details
 *	Step 1: 使用 vsnprintf 格式化到 暫存(Buffer)
 *	Step 2: 防止回傳長度超過暫存區大小
 *	Step 3: 將有效資料寫入 Ring Buffer
 *
 *	@param Fmt: prinf-style format string
 *
 *	@return None
 */
void Debug_Print(const char *Fmt, ...)
{
	char Temp[DEBUG_TEMP_SIZE];
	va_list Args;
	int Len;

	va_start(Args, Fmt);
	Len = vsnprintf(Temp, sizeof(Temp), Fmt, Args);
	va_end(Args);

	if (Len <= 0)
	{
		return;
	}

	if (Len >= (int) sizeof(Temp))
	{
		Len = (int) (sizeof(Temp) - 1U);
	}

	WriteBuffer((const uint8_t*) Temp, (uint16_t) Len);
}

/**	@brief 處理 Ring Buffer 內容送到 UART DMA
 *
 * 	@details
 * 	Step 1: 若 UART DMA 忙碌中, 直接返回
 * 	Step 2: 若 Ring Buffer 無資料, 直接返回
 * 	Step 3: 先用 TempTail 預讀資料到 Tx_Buffer
 *	Step 4: 只有在 UART_DMA_Send() 成功時, 才正式提交 Tail
 *
 *	@return None
 */
void Debug_Process(void)
{
	uint16_t TempTail;
	uint16_t Len = 0U;

	if (UART_DMA_IsBusy() != 0U)
	{
		return;
	}

	if (Head == Tail)
	{
		return;
	}

	TempTail = Tail;

	while ((TempTail != Head) && (Len < DEBUG_TX_BUFFER_SIZE))
	{
		Tx_Buffer[Len++] = DebugBuf[TempTail];
		TempTail = (uint16_t) ((TempTail + 1U) % DEBUG_BUFFER_SIZE);
	}

	if (Len == 0U)
	{
		return;
	}

	if (UART_DMA_Send(Tx_Buffer, Len) != 0U)
	{
		Tail = TempTail;
	}

	/// 20260409 修改 => [暫且保留]
	/// UART_DMA_Send(Tx_Buffer, Len);
}
