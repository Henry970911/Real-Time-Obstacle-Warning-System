/*
 *  debug_service.h
 *
 *  Created on: Mar 30, 2026
 *  Author: Henry
 *
 *  模組設計 : Ring Buffer / 非阻塞 / DMA 自動發送 / 主循環 Loop
 */

#ifndef SERVICE_DEBUGSERVICE_H_
#define SERVICE_DEBUGSERVICE_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "UartDma.h"

/**
 *	Debug_Init => 初始化 Debug Ring Buffer
 *
 *	Debug_Print => 格式化輸出 Debug 訊息到 Ring Buffer
 *
 *	Debug_Process => 處理 Ring Buffer 內容並送往 UART DMA
 *
 *	Debug_WriteRaw => 直接將原始資料寫入 Ring Buffer
 */

void Debug_Init(void);
void Debug_Print(const char *Fmt, ...);
void Debug_Process(void);
void Debug_WriteRaw(const uint8_t *Data, uint16_t Len);

#endif /* SERVICE_DEBUGSERVICE_H_ */
