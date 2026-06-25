/*
 *  RingBuffer.h
 *
 *  Created on: 2026年5月15日
 *  Author: User
 */

#ifndef SERVICE_RINGBUFFER_H_
#define SERVICE_RINGBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 	256U

typedef struct
{
	uint16_t Head;
	uint16_t Tail;
	uint16_t Count;
	uint8_t Buffer[RING_BUFFER_SIZE];
} RingBuffer_t;

void RingBuffer_Init(RingBuffer_t *RingBuffer);
bool RingBuffer_IsFull(const RingBuffer_t *RingBuffer);
bool RingBuffer_IsEmpty(const RingBuffer_t *RingBuffer);
uint16_t RingBuffer_GetCount(const RingBuffer_t *RingBuffer);
bool RingBuffer_ReadByte(RingBuffer_t *RingBuffer, uint8_t *Data);
bool RingBuffer_WriteByte(RingBuffer_t *RingBuffer, uint8_t Data);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_RINGBUFFER_H_ */
