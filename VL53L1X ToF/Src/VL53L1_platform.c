/**
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "i2c.h"
#include "cmsis_os2.h"
#include "DebugService.h"
#include "VL53L1_platform.h"

/// 20260522 Version
/// VL53L1X I2C Error Log Interval: 底層 I2C Read/Write Error Log 最多每 1000 ms 輸出一次
#define VL53L1X_I2C_ERROR_LOG_INTERVAL_MS	1000U
#define VL53L1_I2C_TIMEOUT 500U

/// 20260424 版本修改 => 修改腳位名稱
/// #define VL53L1_XSHUT_PIN ARD_D4_Pin
/// #define VL53L1_XSHUT_GPIO_PORT ARD_D4_GPIO_Port
#define VL53L1_XSHUT_PIN VL53L1X_XSHUT_Pin
#define VL53L1_XSHUT_GPIO_PORT VL53L1X_XSHUT_GPIO_Port

static uint32_t VL53L1X_I2C_ReadErrorLastTick = 0U;		/// 用於 Time-Based Rate Limiting Algorithm
static uint32_t VL53L1X_I2C_WriteErrorLastTick = 0U;	/// 用於 Time-Based Rate Limiting Algorithm

extern I2C_HandleTypeDef hi2c4;

/**
 * 	@brief 取得系統 tick
 *
 * 	@details
 * 	Step 1: 若 RTOS 已啟動，使用 osKernelGetTickCount()。
 * 	Step 2: 若 RTOS 尚未啟動，使用 HAL_GetTick()。
 *
 * 	@param None
 *
 * 	@return uint32_t tick count
 */
static uint32_t VL53L1X_I2C_GetTickSafe(void)
{
	if (osKernelGetState() == osKernelRunning)
	{
		return osKernelGetTickCount();
	}

	return HAL_GetTick();
}

/**
 * 	@brief VL53L1X I2C read error log throttle
 *
 * 	@details
 * 	演算法 [Time-Based Rate Limiting Algorithm]
 *
 * 	Step 1: 取得目前 RTOS tick。
 * 	Step 2: 若第一次錯誤，立即輸出。
 * 	Step 3: 若距離上次輸出已超過 VL53L1X_I2C_ERROR_LOG_INTERVAL_MS，才再次輸出。
 * 	Step 4: 更新 read error last tick。
 *
 * 	@param Dev    I2C device address
 * 	@param Reg    VL53L1X register address
 * 	@param Count  read byte count
 * 	@param HalErr HAL I2C error code
 *
 * 	@return None
 */
static void VL53L1X_I2C_LogReadErrorThrottled(uint16_t Dev, uint16_t Reg, uint32_t Count, uint32_t HalErr)
{
	uint32_t CurrentTick;

	CurrentTick = VL53L1X_I2C_GetTickSafe();

	if ((VL53L1X_I2C_ReadErrorLastTick == 0U) || ((CurrentTick - VL53L1X_I2C_ReadErrorLastTick) >= VL53L1X_I2C_ERROR_LOG_INTERVAL_MS))
	{
		Debug_Print("I2C Read Fail: Dev = 0x%02X, Reg = 0x%04X, Cnt = %lu, Err = 0x%08lX\r\n", Dev, Reg, (unsigned long) Count, (unsigned long) HalErr);

		Debug_Process();

		VL53L1X_I2C_ReadErrorLastTick = CurrentTick;
	}
}

/**
 * 	@brief VL53L1X I2C write error log throttle
 *
 * 	@details
 * 	演算法 [Time-Based Rate Limiting Algorithm]
 *
 * 	Step 1: 取得目前 RTOS tick。
 * 	Step 2: 若第一次錯誤，立即輸出。
 * 	Step 3: 若距離上次輸出已超過 VL53L1X_I2C_ERROR_LOG_INTERVAL_MS，才再次輸出。
 * 	Step 4: 更新 write error last tick。
 *
 * 	@param Dev    I2C device address
 * 	@param Reg    VL53L1X register address
 * 	@param Count  write byte count
 * 	@param HalErr HAL I2C error code
 *
 * 	@return None
 */
static void VL53L1X_I2C_LogWriteErrorThrottled(uint16_t Dev, uint16_t Reg, uint32_t Count, uint32_t HalErr)
{
	uint32_t CurrentTick;

	CurrentTick = VL53L1X_I2C_GetTickSafe();

	if ((VL53L1X_I2C_WriteErrorLastTick == 0U) || ((CurrentTick - VL53L1X_I2C_WriteErrorLastTick) >= VL53L1X_I2C_ERROR_LOG_INTERVAL_MS))
	{
		Debug_Print("I2C Write Fail: Dev = 0x%02X, Reg = 0x%04X, Cnt = %lu, Err = 0x%08lX\r\n", Dev, Reg, (unsigned long) Count, (unsigned long) HalErr);

		Debug_Process();

		VL53L1X_I2C_WriteErrorLastTick = CurrentTick;
	}
}

/**	@brief VL53L1 multi-byte write
 *
 * 	@details
 * 	- Step 1: 以 16-bit register address 寫入指定長度資料
 * 	- Step 2: 若 HAL I2C 失敗，印出錯誤資訊
 *
 * 	@param dev	: VL53L1X 7-bit device address
 * 	@param index: 16-bit Register Address
 * 	@param pdata: 資料位址
 * 	@param count: 資料長度
 *
 * 	@return 0 ? -1 => Success : HAL I2C Transmit Failed
 */
int8_t VL53L1_WriteMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
	HAL_StatusTypeDef Status;

	/// 20260409 版本修改 : 新增 VL53L1 狀態診斷
	Status = HAL_I2C_Mem_Write(&hi2c4, (uint16_t) (dev << 1), index, I2C_MEMADD_SIZE_16BIT, pdata, (uint16_t) count, VL53L1_I2C_TIMEOUT);

	if (Status != HAL_OK)
	{
		/// Debug_Print("\r I2C Write Fail : Dev = 0x%02X, Reg = 0x%04X, Cnt = %lu, Hel = %d, Err = 0x%08lX \r\n", dev, index, count, Status, hi2c4.ErrorCode);
		VL53L1X_I2C_LogWriteErrorThrottled(dev, index, count, HAL_I2C_GetError(&hi2c4));

		return -1;
	}

	return 0;
}

/**	@brief VL53L1 multi-byte read
 *
 * 	@details
 * 	- Step 1: 以 16-bit register address 讀取指定長度資料
 * 	- Step 2: 若 HAL I2C 失敗，印出錯誤資訊
 *
 * 	@param dev	: VL53L1X 7-bit device address
 * 	@param index: 16-bit register address
 * 	@param pdata: 接收資料位址
 * 	@param count: 資料長度
 *
 * 	@return 0 ? -1 => Success : HAL I2C transmit/receive failed
 */
int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
	HAL_StatusTypeDef Status;

	Status = HAL_I2C_Mem_Read(&hi2c4, (uint16_t) (dev << 1), index, I2C_MEMADD_SIZE_16BIT, pdata, (uint16_t) count, VL53L1_I2C_TIMEOUT);

	if (Status != HAL_OK)
	{
		VL53L1X_I2C_LogReadErrorThrottled(dev, index, count, HAL_I2C_GetError(&hi2c4));

		return -1;
	}

	return 0;
}

/**
 * @brief 寫入 1 byte 至 VL53L1 暫存器
 *
 * @param dev	: device address
 * @param index	: register address
 * @param pdata	: one byte data
 *
 * @return 0 ? -1 => Success : Fail
 */
int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data)
{
	return VL53L1_WriteMulti(dev, index, &data, 1U);
}

/**
 * @brief 寫入 2 bytes 至 VL53L1 暫存器
 *
 * @details
 * - Step 1: 將 uint16_t 拆成 High Byte / Low Byte
 * - Step 2: 呼叫 WriteMulti() 執行實際寫入
 *
 * @param dev	: Device Address
 * @param index	: Register Address
 * @param data	: 16 bit(2 Bytes) Data
 *
 * @return 0 : -1 => Success : Failed
 */
int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data)
{
	uint8_t Buffer[2];

	Buffer[0] = (uint8_t) ((data >> 8) & 0xFFU);
	Buffer[1] = (uint8_t) (data & 0xFFU);

	return VL53L1_WriteMulti(dev, index, Buffer, 2U);
}

/**
 * @brief 寫入 4 bytes 至 VL53L1 暫存器
 *
 * @details
 * - Step 1: 將 uint32_t 拆成 4 bytes
 * - Step 2: 呼叫 WriteMulti() 執行實際寫入
 *
 * @param dev	: Device Address
 * @param index	: Register Address
 * @param data	: 32 bit(4 Bytes) Data
 *
 * @return 0 : -1 => Success : Failed
 */
int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data)
{
	uint8_t Buffer[4];

	Buffer[0] = (uint8_t) ((data >> 24) & 0xFFU);
	Buffer[1] = (uint8_t) ((data >> 16) & 0xFFU);
	Buffer[2] = (uint8_t) ((data >> 8) & 0xFFU);
	Buffer[3] = (uint8_t) (data & 0xFFU);

	return VL53L1_WriteMulti(dev, index, Buffer, 4U);
}

/**
 * @brief Read 1 byte from VL53L1 register
 *
 * @param dev	: device address
 * @param index	: register address
 * @param pdata	: output pointer
 *
 * @return 0 : -1 => Success : Fail
 */
int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data)
{
	return VL53L1_ReadMulti(dev, index, data, 1U);
}

/**
 * @brief 讀取 2 bytes 從 VL53L1 暫存器
 *
 * @details
 * - Step 1: 先讀 2 bytes 到 buffer
 * - Step 2: 再將 High byte / Low byte 組成 uint16_t
 *
 * @param dev	: Device Address
 * @param index	: Register Address
 * @param data	: output pointer for 16 bit data
 *
 * @return 0 : -1 => Success : Failed
 */
int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data)
{
	uint8_t Buffer[2];
	int8_t Status;

	Status = VL53L1_ReadMulti(dev, index, Buffer, 2U);

	if (Status != 0)
	{
		return -1;
	}

	*data = ((uint16_t) Buffer[0] << 8) | (uint16_t) Buffer[1];

	return 0;
}

/**
 * @brief 讀取 4 bytes 從 VL53L1 暫存器
 *
 * @details
 * - Step 1: 先讀 4 bytes 到 buffer
 * - Step 2: 再組回 uint32_t
 *
 * @param dev	: Device Address
 * @param index	: Register Address
 * @param data	: output pointer for 32-bit data
 *
 * @return 0 : -1 => Success : Failed
 */
int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data)
{
	uint8_t Buffer[4];
	int8_t Status;

	Status = VL53L1_ReadMulti(dev, index, Buffer, 4U);

	if (Status != 0)
	{
		return -1;
	}

	*data = ((uint32_t) Buffer[0] << 24) | ((uint32_t) Buffer[1] << 16) | ((uint32_t) Buffer[2] << 8) | ((uint32_t) Buffer[3]);

	return 0;
}

/**	@brief	Wait millisecond for VL53L1 API
 *
 *	@details
 *	[Delay 方式]
 *	- RTOS 尚未啟動	HAL_Delay()
 *	- RTOS 啟動時	osDelay()
 *
 * 	@param	dev		Device Address(unused)
 * 	@param	wait_ms	Delay Time in ms
 *
 * 	@return	Always 0
 */
int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms)
{
	/// 20260605 版本修改: 防止 Blocking
	(void) dev;

	if (wait_ms <= 0)
	{
		return 0;
	}

	if (osKernelGetState() == osKernelRunning)
	{
		osDelay((uint32_t) wait_ms);
	}
	else
	{
		HAL_Delay((uint32_t) wait_ms);
	}

	return 0;
}

void VL53L1_HardReset(void)
{
	/// Step 1: 拉低 XSHUT (關閉 Sensor)
	HAL_GPIO_WritePin(VL53L1_XSHUT_GPIO_PORT, VL53L1_XSHUT_PIN, GPIO_PIN_RESET);
	(void) VL53L1_WaitMs(0U, 10);

	/// Step 2: 拉高 XSHUT，釋放 reset，重新啟動 Sensor
	HAL_GPIO_WritePin(VL53L1_XSHUT_GPIO_PORT, VL53L1_XSHUT_PIN, GPIO_PIN_SET);
	(void) VL53L1_WaitMs(0U, 10);
}
