/*
 *  uart_dma.h
 *
 *  Created on: Mar 26, 2026
 *  Author: Henry
 */

#ifndef INC_UART_DMA_H_
#define INC_UART_DMA_H_

#include <stdio.h>
#include <stdint.h>
#include "usart.h"
#include "DebugService.h"

extern volatile uint8_t uart_dma_done_flag;

/**
 * 	@brief	Initialize UART DMA Status
 */
void UART_DMA_Init(void);

/**
 * 	@brief	Data is Sent out Non-Blocking via UART DMA (透過 uart DMA 非阻塞送資料)
 */
uint8_t UART_DMA_IsBusy(void);

/**
 * 	@brief	Check UART DMA is Busy ?
 */
uint8_t UART_DMA_Send(uint8_t *Buffer, uint32_t Len);

#endif /* INC_UART_DMA_H_ */
