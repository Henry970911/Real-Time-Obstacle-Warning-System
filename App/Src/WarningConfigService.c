/*
 *  WarningConfigService.c
 *
 *  Created on: May 19, 2026
 *  Author: Henry
 */

#include "WarningConfigService.h"

/**	Phase F-5.7:
 *
 * 	Warning hysteresis threshold configuration.
 *
 * 	注意: 這裡的數值要與目前 WarningTask 原本使用的 threshold 保持一致。
 */
#define WARNING_CONFIG_CAUTION_ENTER_MM     800U
#define WARNING_CONFIG_CAUTION_EXIT_MM      900U

#define WARNING_CONFIG_WARNING_ENTER_MM     500U
#define WARNING_CONFIG_WARNING_EXIT_MM      600U

#define WARNING_CONFIG_DANGER_ENTER_MM      200U
#define WARNING_CONFIG_DANGER_EXIT_MM       300U

static WarningThresholdSnapshot_t WarningThreshold;

void WarningConfig_Init(void)
{
	WarningThreshold.CautionEnterMm = WARNING_CONFIG_CAUTION_ENTER_MM;
	WarningThreshold.CautionExitMm = WARNING_CONFIG_CAUTION_EXIT_MM;

	WarningThreshold.WarningEnterMm = WARNING_CONFIG_WARNING_ENTER_MM;
	WarningThreshold.WarningExitMm = WARNING_CONFIG_WARNING_EXIT_MM;

	WarningThreshold.DangerEnterMm = WARNING_CONFIG_DANGER_ENTER_MM;
	WarningThreshold.DangerExitMm = WARNING_CONFIG_DANGER_EXIT_MM;
}

bool WarningConfig_GetThresholdSnapshot(WarningThresholdSnapshot_t *Snapshot)
{
	if (Snapshot == NULL)
	{
		return false;
	}

	*Snapshot = WarningThreshold;

	return true;
}

uint16_t WarningConfig_GetCautionEnterMm(void)
{
	return WarningThreshold.CautionEnterMm;
}

uint16_t WarningConfig_GetCautionExitMm(void)
{
	return WarningThreshold.CautionExitMm;
}

uint16_t WarningConfig_GetWarningEnterMm(void)
{
	return WarningThreshold.WarningEnterMm;
}

uint16_t WarningConfig_GetWarningExitMm(void)
{
	return WarningThreshold.WarningExitMm;
}

uint16_t WarningConfig_GetDangerEnterMm(void)
{
	return WarningThreshold.DangerEnterMm;
}

uint16_t WarningConfig_GetDangerExitMm(void)
{
	return WarningThreshold.DangerExitMm;
}
