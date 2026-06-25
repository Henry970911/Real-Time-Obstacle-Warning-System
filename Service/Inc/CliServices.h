/*
 * 	CliServices.h
 *
 *  Created on: 2026.05.17
 *  Author: Henry
 */

#ifndef SERVICE_CLISERVICES_H_
#define SERVICE_CLISERVICES_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "DebugService.h"
#include "UartStatusService.h"
#include "QueueStatusService.h"
#include "SystemStatusService.h"
#include "SensorHealthService.h"
#include "SystemHealthService.h"
#include "WarningConfigService.h"

#define CLI_MAX_ARGS 		4U
#define CLI_MAX_LINE_SIZE	64U

void CLI_Init(void);
void CLI_ProcessLine(char *LINE);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_CLISERVICES_H_ */
