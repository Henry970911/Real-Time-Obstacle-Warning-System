/*
 * 	UartStatusService.h
 *
 *  Created on: May 21, 2026
 *  Author: Henry
 */

#ifndef SERVICE_UARTSTATUSSERVICE_H_
#define SERVICE_UARTSTATUSSERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/**
 * 	@brief UART Status Snapshot
 *
 * 	@details
 * 	- 保存 UART RX DMA / Ring Buffer 相關資訊
 * 	- CLI Uart Command 會讀取此 Snapshot 並輸出
 */
typedef struct
{
	uint32_t RxCallbackCount;
	uint32_t RxOverflowCount;
	uint16_t LastRxSize;
} UartStatusSnapshot_t;

void UartStatus_Init(void);
void UartStatus_OnRxEvent(uint16_t Size);
void UartStatus_AddOverflow(void);
bool UartStatus_GetSnapshot(UartStatusSnapshot_t *Snapshot);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_UARTSTATUSSERVICE_H_ */
