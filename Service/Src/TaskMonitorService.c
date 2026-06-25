/*
 * 	TaskMonitorService.c
 *
 *  Created on: May 28, 2026
 *  Author: Henry
 */

#include "TaskMonitorService.h"

static osThreadId_t SensorTaskHandleRef = NULL;
static osThreadId_t WarningTaskHandleRef = NULL;
static osThreadId_t CommunicationTaskHandleRef = NULL;

/**	@brief	初始化 Task Monitor Service
 *
 * 	@details
 * 	演算法 [Handle Reference Initialization]
 * 	Step 1: 清除 task handle reference
 * 	Step 2: 等待 freertos.c 註冊實際 task handle
 *
 * 	@param	None
 * 	@return	None
 */
void TaskMonitor_Init(void)
{
	SensorTaskHandleRef = NULL;
	WarningTaskHandleRef = NULL;
	CommunicationTaskHandleRef = NULL;
}

/**	@brief	註冊需要監控的 Task Hasndle
 *
 * 	@details
 * 	演算法 [Task Handle Registration]
 * 	Step 1: 保存 SensorTask handle
 * 	Step 2: 保存 WarningTask handle
 * 	Step 3: 保存 CommunicationTask handle
 *
 * 	@param	SensorTaskHandle	SensorTask handle
 * 	@param	WarningTaskHandle	WarningTask handle
 * 	@param	CommunicationTaskHandle		CommunicationTask handle
 *
 * 	@return	None
 */
void TaskMonitor_Register(osThreadId_t SensorTaskHandle, osThreadId_t WarningTaskHandle, osThreadId_t CommunicationTaskHandle)
{
	SensorTaskHandleRef = SensorTaskHandle;
	WarningTaskHandleRef = WarningTaskHandle;
	CommunicationTaskHandleRef = CommunicationTaskHandle;
}

/**	@brief	取得 Task Stack High Water Mark Snapshot
 *
 * 	@details
 * 	Step 1: 檢查 Snapshot 指標
 * 	Step 2: 若 task handle 有效，讀取 osThreadGetStackSpace()
 * 	Step 3: 若 task handle 無效，回傳 0
 *
 * 	@param	Snapshot	輸出 task monitor snapshot
 *
 * 	@return	bool	true : snapshot 取得成功 ? false: Snapshot == NULL
 */
bool TaskMonitor_GetSnapshot(TaskMonitorSnapshot_t *Snapshot)
{
	if (Snapshot == NULL)
	{
		return false;
	}

	Snapshot->SensorTaskStackWords = 0U;
	Snapshot->WarningTaskStackWords = 0U;
	Snapshot->CommunicationTaskStackWords = 0U;

	if (SensorTaskHandleRef != NULL)
	{
		Snapshot->SensorTaskStackWords = osThreadGetStackSpace(SensorTaskHandleRef);
	}

	if (WarningTaskHandleRef != NULL)
	{
		Snapshot->WarningTaskStackWords = osThreadGetStackSpace(WarningTaskHandleRef);
	}

	if (CommunicationTaskHandleRef != NULL)
	{
		Snapshot->CommunicationTaskStackWords = osThreadGetStackSpace(CommunicationTaskHandleRef);
	}

	return true;
}
