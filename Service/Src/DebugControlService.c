/*
 * 	DebugControlService.c
 *
 *  Created on: May 27, 2026
 *  Author: Henry
 *
 *  FILE: Core/CLIService/DebugControlService.c
 */

#include "DebugControlService.h"

static volatile uint8_t DebugLogEnable = 1U;

/**
 * 	@brief	初始化 Debug Control Service
 *
 * 	@details
 * 	演算法 [Global Flag Initialization]
 * 	Step 1. 將 DebugLogEnable 預設為 enable
 *
 * 	@param	None
 *
 * 	@return	None
 */
void DebugControl_Init(void)
{
	DebugLogEnable = 1U;
}

/**
 * 	@brief	設定 Debug log 是否啟用
 *
 * 	@details
 * 	演算法 [Boolean State Assignment]
 * 	Step 1. 若 Enable != 0，設定為 1
 * 	Step 2. 若 Enable == 0，設定為 0
 *
 * 	@param	Enable	0: disable log ? 1: enable log
 *
 * 	@return	None
 */
void DebugControl_SetLogEnable(uint8_t Enable)
{
	DebugLogEnable = (Enable != 0U) ? 1U : 0U;
}

/**
 * 	@brief	查詢 Debug log 是否啟用
 *
 * 	@details
 * 	演算法 [Flag Query]
 *
 * 	@param	None
 *
 * 	@return	uint8_t	0: disabled ? 1: enabled
 */
uint8_t DebugControl_IsLogEnable(void)
{
	return DebugLogEnable;
}

/**
 * 	@brief	取得 Debug control snapshot
 *
 * 	@details
 * 	演算法 [Snapshot Read Algorithm]
 * 	Step 1. 檢查 Snapshot 指標
 * 	Step 2. 複製目前 DebugLogEnable 狀態
 *
 * 	@param	Snapshot 輸出 Debug control snapshot
 *
 * 	@return	bool	true: 成功 ? false: Snapshot == NULL
 */
bool DebugControl_GetSnapshot(DebugControlSnapshot_t *Snapshot)
{
	if (Snapshot == NULL)
	{
		return false;
	}

	Snapshot->LogEnable = DebugLogEnable;

	return true;
}
