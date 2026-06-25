/*
 * 	QueueStatusService.c
 *
 *  Created on: May 19, 2026
 *  Author: Henry
 */

#include "QueueStatusService.h"

static osMessageQueueId_t SensorEventQueueHandleRef = NULL;
static osMessageQueueId_t DistanceQueueHandleRef = NULL;
static osMessageQueueId_t SensorControlQueueRef = NULL;
static osMessageQueueId_t BuzzerControlQueueRef = NULL;

/**
 * 	@brief 註冊 RTOS Queue handle
 *
 * 	@details
 * 	演算法名稱：Handle Registration Pattern
 *
 * 	Step 1. 保存 SensorEventQueue handle。
 * 	Step 2. 保存 DistanceQueue handle。
 * 	Step 3. 後續 CLI 可透過 QueueStatus_GetSnapshot() 查詢 queue 狀態。
 *
 * 	@param SensorEventQueue Sensor event queue handle。
 * 	@param DistanceQueue Distance queue handle。
 *
 * 	@return None
 */
void QueueStatus_Register(osMessageQueueId_t SensorEventQueue, osMessageQueueId_t Distance, osMessageQueueId_t SensorControlQueue, osMessageQueueId_t BuzzerControlQueue)
{
	SensorEventQueueHandleRef = SensorEventQueue;
	DistanceQueueHandleRef = Distance;
	SensorControlQueueRef = SensorControlQueue;
	BuzzerControlQueueRef = BuzzerControlQueue;
}

/**
 * 	@brief 取得 Queue 狀態快照
 *
 * 	@details
 * 	演算法名稱：Queue Snapshot Read Algorithm
 *
 * 	Step 1. 檢查 Snapshot 指標是否有效。
 * 	Step 2. 檢查 Queue handle 是否已註冊。
 * 	Step 3. 呼叫 osMessageQueueGetCount() 取得目前 queue item 數量。
 * 	Step 4. 呼叫 osMessageQueueGetSpace() 取得 queue 剩餘空間。
 * 	Step 5. 將結果寫入 Snapshot。
 *
 * 	@param Snapshot 輸出 Queue 狀態快照。
 *
 * 	@return bool
 * 	- true: 取得成功
 * 	- false: Snapshot 無效或 queue 尚未註冊
 */
bool QueueStatus_GetSnapshot(QueueStatusSnapshot_t *Snapshot)
{
	if (Snapshot == NULL)
	{
		return false;
	}

	Snapshot->SensorEventQueueCount = 0U;
	Snapshot->SensorEventQueueSpace = 0U;
	Snapshot->DistanceQueueCount = 0U;
	Snapshot->DistanceQueueSpace = 0U;
	Snapshot->SensorControlQueueCount = 0U;
	Snapshot->SensorControlQueueSpace = 0U;
	Snapshot->BuzzerControlQueueCount = 0U;
	Snapshot->BuzzerControlQueueSpace = 0U;
	Snapshot->IsRegistered = 0U;

	if ((SensorEventQueueHandleRef == NULL) || (DistanceQueueHandleRef == NULL))
	{
		return false;
	}

	if ((SensorControlQueueRef == NULL) || (BuzzerControlQueueRef == NULL))
	{
		return false;
	}

	Snapshot->SensorEventQueueCount = osMessageQueueGetCount(SensorEventQueueHandleRef);
	Snapshot->SensorEventQueueSpace = osMessageQueueGetSpace(SensorEventQueueHandleRef);
	Snapshot->DistanceQueueCount = osMessageQueueGetCount(DistanceQueueHandleRef);
	Snapshot->DistanceQueueSpace = osMessageQueueGetSpace(DistanceQueueHandleRef);
	Snapshot->SensorControlQueueCount = osMessageQueueGetCount(SensorControlQueueRef);
	Snapshot->SensorControlQueueSpace = osMessageQueueGetSpace(SensorControlQueueRef);
	Snapshot->BuzzerControlQueueCount = osMessageQueueGetCount(BuzzerControlQueueRef);
	Snapshot->BuzzerControlQueueSpace = osMessageQueueGetSpace(BuzzerControlQueueRef);
	Snapshot->IsRegistered = 1U;

	return true;
}

