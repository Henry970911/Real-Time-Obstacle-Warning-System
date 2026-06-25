/*
 * 	ControlCommandService.h
 *
 *  Created on: May 27, 2026
 *  Author: Henry
 *
 *  FILE: Core/CLIService/ControlCommandService.h
 */

#ifndef CLISERVICE_CONTROLCOMMANDSERVICE_H_
#define CLISERVICE_CONTROLCOMMANDSERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

/**	@brief	Sensor Control Event Definition
 *
 * 	@details
 * 	用於 CLI 傳送 Sensor 控制命令給 SensorTask
 */
typedef enum
{
	SENSOR_CONTROL_NONE = 0U,
	SENSOR_CONTROL_RECONNECT = 1U
} SensorControlEvent_t;

/**	@brief	Buzzer Control Event Definition
 *
 * 	@details
 * 	CLI 傳送 Sensor 控制命令給 SensorTask
 */
typedef enum
{
	BUZZER_CONTROL_NONE = 0U,
	BUZZER_CONTROL_ON,
	BUZZER_CONTROL_OFF,
	BUZZER_CONTROL_TEST,
	BUZZER_CONTROL_MUTE,
	BUZZER_CONTROL_UNMUTE
} BuzzerControlEvent_t;

#ifdef __cplusplus
}
#endif

#endif /* CLISERVICE_CONTROLCOMMANDSERVICE_H_ */
