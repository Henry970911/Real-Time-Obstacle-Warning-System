/*
 *  WarningConfigService.h
 *
 *  Created on: May 19, 2026
 *  Author: Henry
 *
 *  新增 WarningConfigService 檔案
 *
 *  設計: 不建議 CliService.c 直接讀 freertos.c 裡面的 macro, 耦合太強烈
 *
 *  用途: 集中管理 Warning Threshold 設定, 讓 WarningTask 和 CLI 都從同一份設定取得門檻值
 *
 *  架構: WarningConfigService
 *  	    |
 *  	    --- WarningTask 讀 threshold 進行判斷
 *  	    |
 *  	    --- CLI threshold command 讀 threshold 顯示
 */

#ifndef SERVICE_WARNINGCONFIGSERVICE_H_
#define SERVICE_WARNINGCONFIGSERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	uint16_t CautionEnterMm;
	uint16_t CautionExitMm;

	uint16_t WarningEnterMm;
	uint16_t WarningExitMm;

	uint16_t DangerEnterMm;
	uint16_t DangerExitMm;
} WarningThresholdSnapshot_t;

void WarningConfig_Init(void);					/// 初始化 WarningConfigService
bool WarningConfig_GetThresholdSnapshot(WarningThresholdSnapshot_t *Snapshot);	/// 取得目前 warning threshold snapshot
uint16_t WarningConfig_GetCautionEnterMm(void);	/// 取得 CAUTION enter threshold
uint16_t WarningConfig_GetCautionExitMm(void);	/// 取得 CAUTION exit threshold
uint16_t WarningConfig_GetWarningEnterMm(void);	/// 取得 WARNING enter threshold
uint16_t WarningConfig_GetWarningExitMm(void);	/// 取得 WARNING exit threshold
uint16_t WarningConfig_GetDangerEnterMm(void);	/// 取得 DANGER enter threshold
uint16_t WarningConfig_GetDangerExitMm(void);	/// 取得 DANGER exit threshold

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_WARNINGCONFIGSERVICE_H_ */
