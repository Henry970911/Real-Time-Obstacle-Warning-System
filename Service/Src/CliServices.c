/*
 *  CliServices.c
 *
 *  Created on: 2026.05.17
 *  Author: Henry
 */

#include <string.h>
#include "cmsis_os.h"

#include "CliServices.h"
#include "DebugService.h"
#include "UartStatusService.h"
#include "QueueStatusService.h"
#include "TaskMonitorService.h"
#include "SystemStatusService.h"
#include "DebugControlService.h"
#include "WarningConfigService.h"
#include "ControlCommandService.h"

extern osMessageQueueId_t SensorControlQueueHandle;
extern osMessageQueueId_t BuzzerControlQueueHandle;

typedef void (*CLI_CommandHandler_t)(uint8_t argc, char *argv[]);

typedef struct
{
	const char *NAME;
	const char *DESCRIPTION;
	CLI_CommandHandler_t HANDLER;
} CLI_CommandEntry_t;

static void CLI_HandleHelp(uint8_t argc, char *argv[]);
static void CLI_HandleStatus(uint8_t argc, char *argv[]);
static void CLI_HandleSensor(uint8_t argc, char *argv[]);
static void CLI_HandleQueue(uint8_t argc, char *argv[]);
static void CLI_HandleBuzzer(uint8_t argc, char *argv[]);
static void CLI_HandleThreshold(uint8_t argc, char *argv[]);
static void CLI_HandleUart(uint8_t argc, char *argv[]);
static void CLI_HandleLog(uint8_t argc, char *argv[]);
static void CLI_HandleTask(uint8_t argc, char *argv[]);
static void CLI_HandleMonitor(uint8_t argc, char *argv[]);
static bool CLI_SendSensorControl(SensorControlEvent_t Event);
static bool CLI_SendBuzzerControl(BuzzerControlEvent_t Event);
static void CLI_HandleError(uint8_t argc, char *argv[]);
static bool CLI_SendBuzzerControl(BuzzerControlEvent_t Event);
static void CLI_PrintSensorHealthSnapshot(void);
static void CLI_PrintSystemErrorSnapshot(void);

static uint8_t CLI_Tokenize(char *Line, char *Argv[], uint8_t maxArgs);

static const CLI_CommandEntry_t CLI_CommandTable[] =
{
{ "help", "Show Command List", CLI_HandleHelp },
{ "status", "Show System Status", CLI_HandleStatus },
{ "sensor", "Sensor status / reconnect", CLI_HandleSensor },
{ "queue", "Show Queue Status", CLI_HandleQueue },
{ "buzzer", "Buzzer status / on / off / test / mute / unmute", CLI_HandleBuzzer },
{ "threshold", "Show Threshold Information", CLI_HandleThreshold },
{ "uart", "Show UART RX DMA Status", CLI_HandleUart },
{ "log", "Log status / on / off", CLI_HandleLog },
{ "task", "Show RTOS task stack status", CLI_HandleTask },
{ "monitor", "Show system monitor summary", CLI_HandleMonitor },
{ "error", "Show / reset system error counters", CLI_HandleError } };

#define CLI_COMMAND_COUNT ((uint8_t)(sizeof(CLI_CommandTable) / sizeof(CLI_CommandTable[0])))

/**	@brief	將 command line 切割成 argc / argv
 *
 * 	@details
 * 	演算法名	[Tokenization Algorithm]
 *
 * 	Step 1: 跳過前導空白
 * 	Step 2: 找到 token 起始位置後存入 argv
 * 	Step 3: 繼續掃描直到遇到空白或字串結尾
 * 	Step 4: 若遇到空白，將該位置改成 '\0', 完成單一 Token
 * 	Step 5: 重複直到字串結尾或達到 maxArgs
 *
 * 	@param	Line	可修改的 command line buffer
 * 	@param	argv	token pointer array
 * 	@param	maxArgs	argv 最大數量
 *
 * 	@return	uint8_t argc token 數量
 */
static uint8_t CLI_Tokenize(char *Line, char *Argv[], uint8_t MaxArgs)
{
	uint8_t Argc = 0U;
	char *Cursor = Line;

	if ((Line == NULL) || (Argv == NULL) || (MaxArgs == 0U))
	{
		return 0U;
	}

	while ((*Cursor != '\0') && (Argc < MaxArgs))
	{
		while ((*Cursor == ' ') || (*Cursor == '\t'))
		{
			Cursor++;
		}

		if (*Cursor == '\0')
		{
			break;
		}

		Argv[Argc] = Cursor;
		Argc++;

		while ((*Cursor != '\0') && (*Cursor != ' ') && (*Cursor != '\t'))
		{
			Cursor++;
		}

		if (*Cursor == '\0')
		{
			break;
		}

		*Cursor = '\0';
		Cursor++;
	}

	return Argc;
}

/**	@brief	Send sensor control event to SensorTask
 *
 * 	@details
 * 	演算法	[Producer-Consumer Event Queue]
 *
 * 	Step 1: 檢查 SensorControlQueueHandle 是否有效
 * 	Step 2: 將 SensorControlEvent_t 轉成 uint8_t
 * 	Step 3: 使用 osMessageQueuePut() 送入 SensorControlQueue
 *
 * 	@param	Event	Sensor control event
 *
 * 	@return	bool	true : send success ? false: send failed
 */
static bool CLI_SendSensorControl(SensorControlEvent_t Event)
{
	uint8_t RawEvent = (uint8_t) Event;

	if (SensorControlQueueHandle == NULL)
	{
		return false;
	}

	if (osMessageQueuePut(SensorControlQueueHandle, &RawEvent, 0U, 0U) != osOK)
	{
		return false;
	}

	return true;
}

/**
 * 	@brief	輸出 SensorHealthService 狀態快照
 *
 * 	@details
 * 	演算法	[Sensor Health Snapshot Query]
 *
 * 	Step 1: 建立 SensorHealthSnapshot_t 暫存變數
 * 	Step 2: 呼叫 SensorHealth_GetSnapshot() 取得目前 Sensor 狀態
 * 	Step 3: 若取得失敗，輸出錯誤訊息後返回
 * 	Step 4: 輸出 Sensor state、last error、error count、retry count、
 * 	        consecutive error count、last success tick、last error tick。
 *
 * 	@param	None
 *
 * 	@return	None
 */
static void CLI_PrintSensorHealthSnapshot(void)
{
	SensorHealthSnapshot_t Snapshot;

	if (SensorHealth_GetSnapshot(&Snapshot) == false)
	{
		Debug_Print("Sensor Health\r\n");
		Debug_Print("State : Snapshot Failed\r\n");
		return;
	}

	Debug_Print("[Sensor Health]\r\n");
	Debug_Print("State : %s\r\n", SensorHealth_StateToString(Snapshot.State));
	Debug_Print("LastError : %s\r\n", SensorHealth_ErrorToString(Snapshot.LastError));
	Debug_Print("ErrorCount : %lu\r\n", (unsigned long) Snapshot.ErrorCount);
	Debug_Print("RetryCount : %lu\r\n", (unsigned long) Snapshot.RetryCount);
	Debug_Print("ConsecutiveError : %lu\r\n", (unsigned long) Snapshot.ConsecutiveErrorCount);
	Debug_Print("LastSuccessTick : %lu ms\r\n", (unsigned long) Snapshot.LastSuccessTick);
	Debug_Print("LastErrorTick : %lu ms\r\n", (unsigned long) Snapshot.LastErrorTick);
}

/**
 * @brief	Send buzzer control event to WarningTask
 *
 * @details
 * 演算法	[Producer-Consumer Event Queue]
 *
 * Step 1: 檢查 BuzzerControlQueueHandle 是否有效
 * Step 2: 將 BuzzerControlEvent_t 轉成 uint8_t
 * Step 3: 使用 osMessageQueuePut() 送入 BuzzerControlQueue
 *
 * @param	Event	Buzzer control event
 *
 * @return	bool	true:send success ? false:send failed
 */
static bool CLI_SendBuzzerControl(BuzzerControlEvent_t Event)
{
	uint8_t RawEvent = (uint8_t) Event;

	if (BuzzerControlQueueHandle == NULL)
	{
		return false;
	}

	if (osMessageQueuePut(BuzzerControlQueueHandle, &RawEvent, 0U, 0U) != osOK)
	{
		return false;
	}

	return true;
}

void CLI_Init(void)
{
	Debug_Print("[CLI] Ready\r\n");
	Debug_Process();
}

/** 20260517 Version
 * 	@brief	處理一行完整 CLI command
 *
 * 	@details
 * 	演算法 [Pattern]:
 * 	- Tokenization Algorithm
 * 	- Linear Search Algorithm
 * 	- Command Pattern
 * 	- Dispatch Table
 *
 * 	Step 1: 檢查 Line 是否為 NULL
 * 	Step 2: 使用 CLI_Tokenize() 將字串切成 Argc / Argv
 * 	Step 3: 若 Argc == 0，代表空行，直接返回。
 * 	Step 4: 使用 Linear Search 從 CLI_CommandTable 查找 Command Name
 * 	Step 5: 找到後呼叫對應 Handler
 * 	Step 6: 找不到則輸出 unknown command
 *
 * 	@param	Line	Null-terminated command line string.
 *
 * 	@return None
 */
void CLI_ProcessLine(char *LINE)
{
	char *Argv[CLI_MAX_ARGS];
	uint8_t Argc;
	uint8_t Index;

	if (LINE == NULL)
	{
		return;
	}

	Argc = CLI_Tokenize(LINE, Argv, CLI_MAX_ARGS);

	if (Argc == 0U)
	{
		return;
	}

	for (Index = 0U; Index < CLI_COMMAND_COUNT; Index++)
	{
		if (strcmp(Argv[0], CLI_CommandTable[Index].NAME) == 0)
		{
			CLI_CommandTable[Index].HANDLER(Argc, Argv);
			return;
		}
	}

	Debug_Print("[Command Line Interface] Unknown Command: %s\r\n", Argv[0]);
	Debug_Print("[Command Line Interface] Type 'help' For Command List\r\n");
	Debug_Process();
}

static void CLI_HandleHelp(uint8_t argc, char *argv[])
{
	uint8_t Index;

	(void) argc;
	(void) argv;

	Debug_Print("\r\n[Command Line Interface] Command List\r\n");

	for (Index = 0U; Index < CLI_COMMAND_COUNT; Index++)
	{
		Debug_Print("  %-10s - %s\r\n", CLI_CommandTable[Index].NAME, CLI_CommandTable[Index].DESCRIPTION);
	}

	Debug_Process();
}

static void CLI_HandleStatus(uint8_t argc, char *argv[])
{
	SystemStatusSnapshot_t Snapshot;

	(void) argc;
	(void) argv;

	if (SystemStatus_GetSnapshot(&Snapshot) == false)
	{
		Debug_Print("[Command Line Interface] Failed to Get System Status\r\n");
		Debug_Process();
		return;
	}

	Debug_Print("[Command Line Interface] System Status\r\n");
	Debug_Print("RTOS : Running\r\n");
	Debug_Print("CommunicationTask : Running\r\n");
	Debug_Print("Distance : %u (mm)\r\n", Snapshot.Distance_Mm);
	Debug_Print("WarningLevel : %s\r\n", SystemStatus_WarningLevelToString(Snapshot.WarningLevel));
	Debug_Print("Buzzer : %s\r\n", (Snapshot.BuzzerActive) != 0U ? "ACTIVE" : "INACTIVE");
	Debug_Print("SensorUpdates : %lu\r\n", Snapshot.SensorUpdateCount);
	Debug_Print("WarningUpdates : %lu\r\n", Snapshot.WarningUpdateCount);
	Debug_Process();
}

static void CLI_HandleSensor(uint8_t argc, char *argv[])
{
	SystemStatusSnapshot_t Snapshot;

	if (argc == 1U)
	{
		Debug_Print("[Command Line Interface] Sensor Usage\r\n");
		Debug_Print("sensor status\r\n");
		Debug_Print("sensor reconnect\r\n");
		Debug_Process();
		return;
	}

	if (strcmp(argv[1], "status") == 0)
	{
		if (SystemStatus_GetSnapshot(&Snapshot) == false)
		{
			Debug_Print("[Command Line Interface] Failed to Get Sensor Status\r\n");
			Debug_Process();
			return;
		}

		Debug_Print("[Command Line Interface] Sensor Status\r\n");
		Debug_Print("Distance : %u mm\r\n", Snapshot.Distance_Mm);
		Debug_Print("UpdateCount : %lu\r\n", Snapshot.SensorUpdateCount);

		CLI_PrintSensorHealthSnapshot();

		Debug_Process();
		return;
	}

	if (strcmp(argv[1], "reconnect") == 0)
	{
		if (CLI_SendSensorControl(SENSOR_CONTROL_RECONNECT) == true)
		{
			Debug_Print("[Command Line Interface] Sensor reconnect command sent\r\n");
		}
		else
		{
			Debug_Print("[Command Line Interface] Sensor reconnect command failed\r\n");
		}

		Debug_Process();
		return;
	}

	Debug_Print("[Command Line Interface] Usage: sensor <status|reconnect>\r\n");
	Debug_Process();
}

static void CLI_HandleQueue(uint8_t argc, char *argv[])
{
	QueueStatusSnapshot_t Snapshot;

	(void) argc;
	(void) argv;

	if (QueueStatus_GetSnapshot(&Snapshot) == false)
	{
		Debug_Print("[Command Line Interface] Queue Status\r\n");
		Debug_Print("QueueStatusService : Not Registered\r\n");
		Debug_Process();
		return;
	}

	Debug_Print("[Command Line Interface] Queue Status\r\n");
	Debug_Print("QueueEventQueue\r\n");
	Debug_Print("Count : %lu\r\n", Snapshot.SensorEventQueueCount);
	Debug_Print("Space : %lu\r\n", Snapshot.SensorEventQueueSpace);

	Debug_Print("DistanceQueue\r\n");
	Debug_Print("Count : %lu\r\n", Snapshot.DistanceQueueCount);
	Debug_Print("Space : %lu\r\n", Snapshot.DistanceQueueSpace);

	Debug_Print("SensorControlQueue\r\n");
	Debug_Print("Count : %lu\r\n", Snapshot.SensorControlQueueCount);
	Debug_Print("Space : %lu\r\n", Snapshot.SensorControlQueueSpace);

	Debug_Print("BuzzerControlQueue\r\n");
	Debug_Print("Count : %lu\r\n", Snapshot.BuzzerControlQueueCount);
	Debug_Print("Space : %lu\r\n", Snapshot.BuzzerControlQueueSpace);

	Debug_Process();
}

static void CLI_HandleBuzzer(uint8_t argc, char *argv[])
{
	SystemStatusSnapshot_t Snapshot;
	BuzzerControlEvent_t Event = BUZZER_CONTROL_NONE;

	if (argc == 1U)
	{
		Debug_Print("[Command Line Interface] Buzzer Usage\r\n");
		Debug_Print("buzzer status\r\n");
		Debug_Print("buzzer on\r\n");
		Debug_Print("buzzer off\r\n");
		Debug_Print("buzzer test\r\n");
		Debug_Print("buzzer mute\r\n");
		Debug_Print("buzzer unmute\r\n");
		Debug_Process();
		return;
	}

	if (strcmp(argv[1], "status") == 0)
	{
		if (SystemStatus_GetSnapshot(&Snapshot) == false)
		{
			Debug_Print("[Command Line Interface] Failed to Get Buzzer Status\r\n");
			Debug_Process();
			return;
		}

		Debug_Print("[Command Line Interface] Buzzer Status\r\n");
		Debug_Print("State : %s\r\n", (Snapshot.BuzzerActive != 0U) ? "ACTIVE" : "INACTIVE");
		Debug_Process();
		return;
	}
	else if (strcmp(argv[1], "on") == 0)
	{
		Event = BUZZER_CONTROL_ON;
	}
	else if (strcmp(argv[1], "off") == 0)
	{
		Event = BUZZER_CONTROL_OFF;
	}
	else if (strcmp(argv[1], "test") == 0)
	{
		Event = BUZZER_CONTROL_TEST;
	}
	else if (strcmp(argv[1], "mute") == 0)
	{
		Event = BUZZER_CONTROL_MUTE;
	}
	else if (strcmp(argv[1], "unmute") == 0)
	{
		Event = BUZZER_CONTROL_UNMUTE;
	}
	else
	{
		Debug_Print("[Command Line Interface] Usage: buzzer <status|on|off|test|mute|unmute>\r\n");
		Debug_Process();
		return;
	}

	if (CLI_SendBuzzerControl(Event) == true)
	{
		Debug_Print("[Command Line Interface] Buzzer command sent: %s\r\n", argv[1]);
	}
	else
	{
		Debug_Print("[Command Line Interface] Buzzer command failed: %s\r\n", argv[1]);
	}

	Debug_Process();
}

static void CLI_HandleThreshold(uint8_t argc, char *argv[])
{
	WarningThresholdSnapshot_t Snapshot;

	(void) argc;
	(void) argv;

	if (WarningConfig_GetThresholdSnapshot(&Snapshot) == false)
	{
		Debug_Print("[Command Line Interface] Failed to Get Threshold Status\r\n");
		Debug_Process();
		return;
	}

	if (argc == 1U)
	{
		Debug_Print("[Command Line Interface] Threshold Status\r\n");
	}
	else if ((argc == 2U) && (strcmp(argv[1], "show") == 0))
	{
		Debug_Print("[Command Line Interface] Threshold Status\r\n");
	}
	else
	{
		Debug_Print("[Command Line Interface] Usage: Threshold [Show]\r\n");
		Debug_Process();
		return;
	}

	Debug_Print("[Caution]\r\n");
	Debug_Print("Enter : %u (mm)\r\n", Snapshot.CautionEnterMm);
	Debug_Print("Exit  : %u (mm)\r\n", Snapshot.CautionExitMm);
	Debug_Print("[Warning]\r\n");
	Debug_Print("Enter : %u (mm)\r\n", Snapshot.WarningEnterMm);
	Debug_Print("Exit  : %u (mm)\r\n", Snapshot.WarningExitMm);
	Debug_Print("[Danger]\r\n");
	Debug_Print("Enter : %u (mm)\r\n", Snapshot.DangerEnterMm);
	Debug_Print("Exit  : %u (mm)\r\n", Snapshot.DangerExitMm);

	Debug_Process();
}

/**	@brief	處理 Uart Command
 *
 * 	@details
 * 	演算法 [Status Snapshot Query]
 *
 * 	Step 1: 檢查 Command 參數
 * 	Step 2: 呼叫 UartStatus GetSnapshot() 取得 Uart RX DMA 狀態
 * 	Step 3: 將 RX CallBack Count、Last RX Size、OverFlow Count 輸出到 CLI
 *
 * 	@param	argc	command argument count
 * 	@param	argv	command argument vector
 *
 * 	@return	None
 */
static void CLI_HandleUart(uint8_t argc, char *argv[])
{
	UartStatusSnapshot_t Snapshot;

	(void) argv;

	if (argc != 1U)
	{
		Debug_Print("[Command Line Interface] Usage: Uart\r\n");
		Debug_Process();
		return;
	}

	if (UartStatus_GetSnapshot(&Snapshot) == false)
	{
		Debug_Print("[Command Line Interface] Failed to get UART Status\r\n");
		Debug_Process();
		return;
	}

	Debug_Print("[Command Line Interface] UART Status\r\n\r\n");
	Debug_Print("RX DMA Callback Count : %lu\r\n", Snapshot.RxCallbackCount);
	Debug_Print("Last RX Size : %u\r\n", Snapshot.LastRxSize);
	Debug_Print("RX Overflow Count : %lu\r\n", Snapshot.RxOverflowCount);

	Debug_Process();
}

static void CLI_HandleLog(uint8_t argc, char *argv[])
{
	DebugControlSnapshot_t Snapshot;

	if (argc == 1U)
	{
		if (DebugControl_GetSnapshot(&Snapshot) == false)
		{
			Debug_Print("[Command Line Interface] Failed to Get Log Status\r\n");
			Debug_Process();
			return;
		}

		Debug_Print("[Command Line Interface] Log Status\r\n");
		Debug_Print("State : %s\r\n", (Snapshot.LogEnable != 0U) ? "ON" : "OFF");
		Debug_Print("Usage : log <status|on|off>\r\n");
		Debug_Process();
		return;
	}

	if (strcmp(argv[1], "status") == 0)
	{
		if (DebugControl_GetSnapshot(&Snapshot) == false)
		{
			Debug_Print("[Command Line Interface] Failed to Get Log Status\r\n");
			Debug_Process();
			return;
		}

		Debug_Print("[Command Line Interface] Log Status\r\n");
		Debug_Print("State : %s\r\n", (Snapshot.LogEnable != 0U) ? "ON" : "OFF");
		Debug_Process();
		return;
	}

	if (strcmp(argv[1], "on") == 0)
	{
		DebugControl_SetLogEnable(1U);
		Debug_Print("[Command Line Interface] Log ON\r\n");
		Debug_Process();
		return;
	}

	if (strcmp(argv[1], "off") == 0)
	{
		DebugControl_SetLogEnable(0U);
		Debug_Print("[Command Line Interface] Log OFF\r\n");
		Debug_Process();
		return;
	}

	Debug_Print("[Command Line Interface] Usage: log <status|on|off>\r\n");
	Debug_Process();
}

/**	@brief	Handle task command
 *
 * 	@details
 * 	Step 1: 取得 TaskMonitor snapshot
 * 	Step 2: 顯示各 task 剩餘 stack space
 * 	Step 3: 數值越小代表 stack 越危險
 *
 * 	@param	argc	argument count
 * 	@param	argv	argument vector
 *
 * 	@return None
 */
static void CLI_HandleTask(uint8_t argc, char *argv[])
{
	TaskMonitorSnapshot_t Snapshot;

	(void) argc;
	(void) argv;

	if (TaskMonitor_GetSnapshot(&Snapshot) == false)
	{
		Debug_Print("[Command Line Interface] Failed to Get Task Status\r\n");
		Debug_Process();
		return;
	}

	Debug_Print("[Command Line Interface] Task Stack Status\r\n");
	Debug_Print("[SensorTask]\r\n");
	Debug_Print("StackSpace : %lu (bytes)\r\n", Snapshot.SensorTaskStackWords);

	Debug_Print("[WarningTask]\r\n");
	Debug_Print("StackSpace : %lu (bytes)\r\n", Snapshot.WarningTaskStackWords);

	Debug_Print("[CommunicationTask]\r\n");
	Debug_Print("StackSpace : %lu (bytes)\r\n", Snapshot.CommunicationTaskStackWords);

	Debug_Print("Note: Smaller value means less remaining stack.\r\n");

	Debug_Process();
}

/**	@brief	輸出 SystemHealthService 錯誤統計快照
 *
 * 	@details
 * 	演算法 [Snapshot Query Algorithm]
 * 	Step 1: 建立 SystemErrorSnapshot_t 暫存變數
 * 	Step 2: 呼叫 SystemHealth_GetSnapshot() 取得目前錯誤統計
 * 	Step 3: 若取得失敗，輸出錯誤訊息後返回
 * 	Step 4: 逐項輸出錯誤統計 counter
 *
 * 	@param	None
 *
 * 	@return	None
 */
static void CLI_PrintSystemErrorSnapshot(void)
{
	SystemErrorSnapshot_t Snapshot;

	if (SystemHealth_GetSnapshot(&Snapshot) == false)
	{
		Debug_Print("[Command Line Interface] Failed to Get System Error Snapshot\r\n");
		return;
	}

	Debug_Print("[System Error Snapshot]\r\n");
	Debug_Print("SensorReadFail : %lu\r\n", (unsigned long) Snapshot.SensorReadFailCount);
	Debug_Print("SensorDataReadyTimeout : %lu\r\n", (unsigned long) Snapshot.SensorDataReadyTimeoutCount);
	Debug_Print("SensorReconnect : %lu\r\n", (unsigned long) Snapshot.SensorReconnectCount);
	Debug_Print("SensorReconnectFail : %lu\r\n", (unsigned long) Snapshot.SensorReconnectFailCount);
	Debug_Print("I2cError : %lu\r\n", (unsigned long) Snapshot.I2cErrorCount);
	Debug_Print("I2cTimeout : %lu\r\n", (unsigned long) Snapshot.I2cTimeoutCount);
	Debug_Print("QueueOverflow : %lu\r\n", (unsigned long) Snapshot.QueueOverFlowCount);
	Debug_Print("UartRxOverflow : %lu\r\n", (unsigned long) Snapshot.UartRxOverFlowCount);
	Debug_Print("UartTxBusy : %lu\r\n", (unsigned long) Snapshot.UartTxBusyCount);
}

static void CLI_HandleError(uint8_t argc, char *argv[])
{
	if (argc == 1U)
	{
		Debug_Print("[Command Line Interface] Error Counters\r\n");
		CLI_PrintSystemErrorSnapshot();
		Debug_Process();
		return;
	}

	if ((argc == 2U) && (strcmp(argv[1], "reset") == 0))
	{
		SystemHealth_Reset();

		Debug_Print("[Command Line Interface] System error counters reset\r\n");
		Debug_Process();
		return;
	}

	Debug_Print("[Command Line Interface] Usage: Error [reset]\r\n");
	Debug_Print("Error\r\n");
	Debug_Print("Error Reset\r\n");
	Debug_Process();
}

/**	@brief	Handle monitor command
 *
 * 	@details
 * 	Step 1:	取得 system status snapshot
 * 	Step 2: 取得 queue status snapshot
 * 	Step 3: 取得 UART status snapshot
 * 	Step 4: 取得 task monitor snapshot
 * 	Step 5: 取得 log control snapshot
 * 	Step 6: 統一輸出系統摘要
 *
 * 	@param	argc	argument count
 * 	@param	argv	argument vector
 *
 * 	@return	None
 */
static void CLI_HandleMonitor(uint8_t argc, char *argv[])
{
	SystemStatusSnapshot_t SystemSnapshot;
	QueueStatusSnapshot_t QueueSnapshot;
	UartStatusSnapshot_t UartSnapshot;
	TaskMonitorSnapshot_t TaskSnapshot;
	DebugControlSnapshot_t LogSnapshot;
	uint32_t UptimeMs;

	(void) argc;
	(void) argv;

	UptimeMs = osKernelGetTickCount();

	Debug_Print("[Command Line Interface] System Monitor\r\n");

	Debug_Print(" Uptime\r\n");
	Debug_Print(" Tick : %lu ms\r\n", UptimeMs);

	if (SystemStatus_GetSnapshot(&SystemSnapshot) == true)
	{
		Debug_Print("[System]\r\n");
		Debug_Print("RTOS : Running\r\n");
		Debug_Print("Distance : %u mm\r\n", SystemSnapshot.Distance_Mm);
		Debug_Print("Warning : %s\r\n", SystemStatus_WarningLevelToString(SystemSnapshot.WarningLevel));
		Debug_Print("Buzzer : %s\r\n", (SystemSnapshot.BuzzerActive != 0U) ? "ACTIVE" : "INACTIVE");
		Debug_Print("SensorCnt : %lu\r\n", (unsigned long) SystemSnapshot.SensorUpdateCount);
		Debug_Print("WarningCnt : %lu\r\n", (unsigned long) SystemSnapshot.WarningUpdateCount);
	}

	if (QueueStatus_GetSnapshot(&QueueSnapshot) == true)
	{
		Debug_Print("[Queues]\r\n");
		Debug_Print("SensorEvent : %lu / %lu\r\n", QueueSnapshot.SensorEventQueueCount, QueueSnapshot.SensorEventQueueCount + QueueSnapshot.SensorEventQueueSpace);
		Debug_Print("Distance : %lu / %lu\r\n", QueueSnapshot.DistanceQueueCount, QueueSnapshot.DistanceQueueCount + QueueSnapshot.DistanceQueueSpace);
		Debug_Print("SensorCtrl : %lu / %lu\r\n", QueueSnapshot.SensorControlQueueCount, QueueSnapshot.SensorControlQueueCount + QueueSnapshot.SensorControlQueueSpace);
		Debug_Print("BuzzerCtrl : %lu / %lu\r\n", QueueSnapshot.BuzzerControlQueueCount, QueueSnapshot.BuzzerControlQueueCount + QueueSnapshot.BuzzerControlQueueSpace);
	}

	if (UartStatus_GetSnapshot(&UartSnapshot) == true)
	{
		Debug_Print("[UART RX DMA]\r\n");
		Debug_Print("RxCallback : %lu\r\n", UartSnapshot.RxCallbackCount);
		Debug_Print("LastRxSize : %u\r\n", UartSnapshot.LastRxSize);
		Debug_Print("Overflow : %lu\r\n", UartSnapshot.RxOverflowCount);
	}

	if (TaskMonitor_GetSnapshot(&TaskSnapshot) == true)
	{
		Debug_Print("[Task Stack]\r\n");
		Debug_Print("SensorTask : %lu bytes\r\n", TaskSnapshot.SensorTaskStackWords);
		Debug_Print("WarningTask : %lu bytes\r\n", TaskSnapshot.WarningTaskStackWords);
		Debug_Print("CommTask : %lu bytes\r\n", TaskSnapshot.CommunicationTaskStackWords);
	}

	if (DebugControl_GetSnapshot(&LogSnapshot) == true)
	{
		Debug_Print("Debug Log\r\n");
		Debug_Print("State : %s\r\n", (LogSnapshot.LogEnable != 0U) ? "ON" : "OFF");
	}

	CLI_PrintSystemErrorSnapshot();

	Debug_Process();
}
