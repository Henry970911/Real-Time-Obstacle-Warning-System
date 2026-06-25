/*
 *  RingBuffer.c
 *
 *  Created on: 2026年5月15日
 *  Author: User
 */

#include "RingBuffer.h"

/**
 * 	@brief 初始化 Ring Buffer
 *
 * 	@details
 * 	Step 1. 檢查 RingBuffer 指標是否有效。
 * 	Step 2. 將 Head 歸零。
 * 	Step 3. 將 Tail 歸零。
 * 	Step 4. 將 Count 歸零。
 *
 * 	@param RingBuffer: Ring Buffer 物件指標。
 *
 * 	@return None
 */
void RingBuffer_Init(RingBuffer_t *RingBuffer)
{
	if (RingBuffer == NULL)
	{
		return;
	}

	RingBuffer->Head = 0U;
	RingBuffer->Tail = 0U;
	RingBuffer->Count = 0U;
}

/**
 * 	@brief 查詢 Ring Buffer 是否已滿
 *
 * 	@param RingBuffer: Ring Buffer 物件指標
 *
 *	@return => True(Full) ? False(Not Full)
 */
bool RingBuffer_IsFull(const RingBuffer_t *RingBuffer)
{
	if (RingBuffer == NULL)
	{
		return false;
	}

	return (RingBuffer->Count >= RING_BUFFER_SIZE);
}

/**
 * 	@brief 查詢 Ring Buffer 是否已滿
 *
 * 	@param RingBuffer: Ring Buffer 物件指標
 *
 * 	@return => True(Full) ? Fasle(Not Full)
 */
bool RingBuffer_IsEmpty(const RingBuffer_t *RingBuffer)
{
	if (RingBuffer == NULL)
	{
		return true;
	}

	return (RingBuffer->Count == 0U);
}

/**
 * 	@brief 查詢 Ring Buffer 資料筆數
 *
 * 	@param RingBuffer: Ring Buffer 物件指標
 *
 * 	@return =>  uint16_t 目前 Buffer 中的 Byte 數
 */
uint16_t RingBuffer_GetCount(const RingBuffer_t *RingBuffer)
{
	if (RingBuffer == NULL)
	{
		return 0U;
	}

	return RingBuffer->Count;
}

/**
 * 	@brief 從 Ring Buffer 讀出一個 byte
 *
 * 	@details
 * 	演算法名稱：Circular Buffer Read Algorithm
 *
 * 	Step 1. 檢查 RingBuffer 與 Data 指標是否有效
 * 	Step 2. 檢查 buffer 是否為空
 * 	Step 3. 從 Tail 位置讀出資料
 * 	Step 4. Tail 往下一格移動，若到尾端則回到 0
 * 	Step 5. Count 減 1
 *
 * 	@param RingBuffer: Ring Buffer 物件指標
 * 	@param Data: 輸出 Byte 指標
 *
 * 	@return bool => true: 讀取成功 ? false: 讀取失敗
 */
bool RingBuffer_ReadByte(RingBuffer_t *RingBuffer, uint8_t *Data)
{
	if ((RingBuffer == NULL) || (Data == NULL))
	{
		return false;
	}

	if (RingBuffer->Count == 0U)
	{
		return false;
	}

	*Data = RingBuffer->Buffer[RingBuffer->Tail];
	RingBuffer->Tail = (uint16_t) ((RingBuffer->Tail + 1U) % RING_BUFFER_SIZE);
	RingBuffer->Count--;

	return true;
}

/**
 * 	@brief 將一個 byte 寫入 Ring Buffer
 *
 * 	@details
 * 	演算法名稱：Circular Buffer Write Algorithm
 *
 * 	Step 1. 檢查 RingBuffer 指標是否有效
 * 	Step 2. 檢查 buffer 是否已滿
 * 	Step 3. 將資料寫入 Head 位置
 * 	Step 4. Head 往下一格移動，若到尾端則回到 0
 * 	Step 5. Count 加 1
 *
 * 	@param RingBuffer: Ring Buffer 物件指標
 * 	@param Data: 愈寫入的 Byte
 *
 * 	@return => bool True (寫入成功) ? False (寫入失敗)
 */
bool RingBuffer_WriteByte(RingBuffer_t *RingBuffer, uint8_t Data)
{
	if (RingBuffer == NULL)
	{
		return false;
	}

	if (RingBuffer->Count >= RING_BUFFER_SIZE)
	{
		return false;
	}

	RingBuffer->Buffer[RingBuffer->Head] = Data;
	RingBuffer->Head = (uint16_t) ((RingBuffer->Head + 1U) % RING_BUFFER_SIZE);
	RingBuffer->Count++;

	return true;
}
