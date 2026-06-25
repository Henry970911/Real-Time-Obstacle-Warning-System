/*
 * 	DebugControlService.h
 *
 *  Created on: May 27, 2026
 *  Author: Henry
 *
 *  FILE: Core/CLIService/DebugControlService.h
 */

#ifndef CLISERVICE_DEBUGCONTROLSERVICE_H_
#define CLISERVICE_DEBUGCONTROLSERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	uint8_t LogEnable;
} DebugControlSnapshot_t;

void DebugControl_Init(void);
void DebugControl_SetLogEnable(uint8_t Enable);
uint8_t DebugControl_IsLogEnable(void);
bool DebugControl_GetSnapshot(DebugControlSnapshot_t *Snapshot);

#ifdef __cplusplus
}
#endif

#endif /* CLISERVICE_DEBUGCONTROLSERVICE_H_ */
