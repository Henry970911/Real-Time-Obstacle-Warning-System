/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdint.h>
#include <string.h>

#include "gpio.h"
#include "usart.h"
#include "UartDma.h"
#include "RingBuffer.h"
#include "VL53L1X_api.h"
#include "CliServices.h"
#include "DebugService.h"
#include "BuzzerService.h"
#include "VL53L1_platform.h"
#include "UartStatusService.h"
#include "QueueStatusService.h"
#include "SystemStatusService.h"
#include "SensorHealthService.h"
#include "WarningConfigService.h"
#include "DebugControlService.h"
#include "ControlCommandService.h"
#include "TaskMonitorService.h"
#include "SystemHealthService.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
	SENSOR_ERROR_LOG_BOOT_STATE = 0,
	SENSOR_ERROR_LOG_SENSOR_INIT,
	SENSOR_ERROR_LOG_SET_DISTANCE_MODE,
	SENSOR_ERROR_LOG_SET_TIMING_BUDGET,
	SENSOR_ERROR_LOG_SET_INTER_MEASUREMENT,
	SENSOR_ERROR_LOG_SET_INTERRUPT_POLARITY,
	SENSOR_ERROR_LOG_START_RANGING,
	SENSOR_ERROR_LOG_CHECK_DATA_READY,
	SENSOR_ERROR_LOG_GET_DISTANCE,
	SENSOR_ERROR_LOG_CLEAR_INTERRUPT,
	SENSOR_ERROR_LOG_COUNT
} SensorErrorLogId_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/**	@brief	VL53L1X I2C Device Address
 *
 * 	@details
 * 	VL53L1X	API 使用 7-bit address : 0x29
 */
#define VL53L1X_DEVICE_ADDR    					0x29U

/**	@brief	CLI Command Line Buffer Size
 *
 * 	@details
 * 	CommunicationTask 會從 UART RX Ring Buffer 取出一行 command,
 * 	此值限制單行 CLI command 的最大長度
 */
#define CLI_LINE_BUFFER_SIZE					64U

/**	@brief	USART3 RX DMA buffer size(DMA 接收緩衝區)
 *
 * 	@details
 * 	使用 HAL_UARTEx_ReceiveToIdle_DMA() 接收 UART 資料
 * 	STM32H7 若啟用 DCache, DMA buffer 需要 32-byte alignment
 */
#define UART_RX_DMA_BUFFER_SIZE 				64U

/**	@brief	CommunicationTask UART RX Ready Event Flag
 *
 * 	@details
 * 	UART RX DMA callback 收到資料後, 透過 osThreadFlagsSet()
 * 	通知 CommunicationTask 從 Ring Buffer 讀取 CLI command
 */
#define COMM_FLAG_UART_RX_READY 				(1UL << 0)

/**
 * 	@brief	Sensor DataReady event value.
 *
 * 	@details
 * 	VL53L1X GPIO1 觸發 EXTI 後, ISR 將此事件送入 SensorEventQueue
 */
#define SENSOR_EVENT_DATA_READY   				1U

/**
 * @brief VL53L1X ranging configuration.
 *
 * @details
 * 這些參數會同時用於首次初始化與 reconnect 流程，
 * 避免 StartSensorTask() 和 SensorTask_TryReconnect() 出現重複 magic number。
 */
#define VL53L1X_DISTANCE_MODE_LONG              2U
#define VL53L1X_TIMING_BUDGET_MS                50U
#define VL53L1X_INTER_MEASUREMENT_MS            50U
#define VL53L1X_INTERRUPT_POLARITY_ACTIVE_LOW   0U
#define VL53L1X_BOOT_TIMEOUT_MS                 1000U
#define VL53L1X_BOOT_POLL_INTERVAL_MS           10U

/**	@brief	SensorTask runtime timing configuration.
 *
 * 	@details
 * 	SENSOR_RETRY_INTERVAL_MS：
 * 	Sensor 進入 OFFLINE 後, 每隔此時間嘗試重新初始化 VL53L1X
 *
 * 	SENSOR_NO_DATA_TIMEOUT_MS：
 * 	Sensor 已 ONLINE 但長時間沒有成功更新距離時, 判定為 No-Data Timeout
 *
 * 	SENSOR_OFFLINE_LOOP_DELAY_MS：
 * 	Sensor OFFLINE retry loop 每輪延遲，避免 busy loop 佔用 CPU
 *
 * 	SENSOR_EVENT_QUEUE_WAIT_MS：
 * 	SensorTask 等待 DataReady event 的 timeout
 */
#define SENSOR_OFFLINE_ERROR_THRESHOLD			5U		/// Sensor runtime error 連續達到此數值後，判定 Sensor OFFLINE
#define SENSOR_RETRY_INTERVAL_MS				1000U	/// Sensor OFFLINE 後，每隔此時間嘗試重新初始化 VL53L1X
#define SENSOR_NO_DATA_TIMEOUT_MS 				1000U
#define SENSOR_OFFLINE_LOOP_DELAY_MS            100U
#define SENSOR_EVENT_QUEUE_WAIT_MS              50U

/**	@brief	Log throttle timing configuration.
 *
 * 	@details
 * 	用於限制重複錯誤訊息與 WarningTask log 的輸出頻率,
 * 	避免 UART debug log 洗版或造成 TX ring buffer 壓力
 */
#define WARNING_TASK_LOG_INTERVAL_MS			500U	/// WarningTask 警示等級變化 Log 最多每 500 ms 輸出一次
#define SENSOR_ERROR_LOG_INTERVAL_MS 			1000U
#define SENSOR_RECONNECT_TRY_LOG_INTERVAL_MS    5000U
#define SENSOR_RECONNECT_FAIL_LOG_INTERVAL_MS   5000U

/**	@brief	WarningTask distance queue wait timeout.
 *
 * 	@details
 * 	WarningTask 不能永久阻塞在 DistanceQueue, 否則 Buzzer ON/OFF 節奏無法持續更新
 */
#define WARNING_DISTANCE_QUEUE_WAIT_MS          10U

/**	@brief	Buzzer test mode duration.
 *
 * 	@details
 * 	CLI buzzer test 指令會讓蜂鳴器短暫 ON 一段時間, 時間到後由 WarningTask 自動關閉。
 */
#define BUZZER_TEST_DURATION_MS                 300U

/** @brief	Buzzer warning pattern timing definition
 *
 *  @details
 *  使用不同 ON/OFF 時間產生不同警示節奏
 *  有源蜂鳴器不做頻率控制，只做 GPIO ON/OFF 節奏控制
 *
 *  CAUTION: ON 20 ms / OFF 980 ms
 *  WARNING: ON 20 ms / OFF 480 ms
 *  DANGER : ON 30 ms / OFF 120 ms
 */
#define BUZZER_ON_TIME_CAUTION_MS 				20U
#define BUZZER_OFF_TIME_CAUTION_MS				980U

#define BUZZER_ON_TIME_WARNING_MS 				20U
#define BUZZER_OFF_TIME_WARNING_MS				480U

#define BUZZER_ON_TIME_DANGER_MS 				30U
#define BUZZER_OFF_TIME_DANGER_MS				120U

/// 距離值-警示等級
typedef enum
{
	WARNING_LEVEL_SAFE = 0,
	WARNING_LEVEL_CAUTION,
	WARNING_LEVEL_WARNING,
	WARNING_LEVEL_DANGER
} WarningLevel_t;

/**	@brief Board user LED mapping
 *
 * 	@details
 * 	USER_LED1_Pin : 綠色 LED
 * 	USER_LED2_Pin : 紅色 LED
 *
 * 	Warning output policy:
 * 	- SAFE    : Green OFF | Red OFF
 * 	- CAUTION : Green ON  | Red OFF
 * 	- WARNING : Green OFF | Red ON
 * 	- DANGER  : Green ON  | Red ON
 */
#define WARNING_LED_SAFE()            \
    do {                              \
        HAL_GPIO_WritePin(GPIOC, USER_LED1_Pin, GPIO_PIN_RESET); \
        HAL_GPIO_WritePin(GPIOC, USER_LED2_Pin, GPIO_PIN_RESET); \
    } while (0)

#define WARNING_LED_CAUTION()         \
    do {                              \
        HAL_GPIO_WritePin(GPIOC, USER_LED1_Pin, GPIO_PIN_SET);   \
        HAL_GPIO_WritePin(GPIOC, USER_LED2_Pin, GPIO_PIN_RESET); \
    } while (0)

#define WARNING_LED_WARNING()         \
    do {                              \
        HAL_GPIO_WritePin(GPIOC, USER_LED1_Pin, GPIO_PIN_RESET); \
        HAL_GPIO_WritePin(GPIOC, USER_LED2_Pin, GPIO_PIN_SET);   \
    } while (0)

#define WARNING_LED_DANGER()          \
    do {                              \
        HAL_GPIO_WritePin(GPIOC, USER_LED1_Pin, GPIO_PIN_SET);   \
        HAL_GPIO_WritePin(GPIOC, USER_LED2_Pin, GPIO_PIN_SET);   \
    } while (0)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/// Double Buffering / Buffer Handoff DMA buffer 與 App Buffer 分離
/// Bounds Checking Algorithm 後續限制 copy 長度避免 overflow
/**	20260515 Reversion
 * 	@brief USART3 RX DMA Buffer
 *
 * 	@details
 * 	- 資料結構: Fixed-Size DMA Buffer
 * 	- 此 Buffer 由 USART3 RX DMA 寫入
 * 	- STM32H7 啟用 DCache 時, DMA Buffer 需要 32 Byte Alignment.
 */
static uint8_t UartRxDmaBuffer[UART_RX_DMA_BUFFER_SIZE] __attribute__((aligned(32)));

/**	20260517 Reversion
 * 	@brief UART RX Ring Buffer
 *
 * 	@details
 * 	資料結構名稱：Circular Buffer / Ring Buffer
 *
 * 	HAL_UARTEx_RxEventCallback() 將 RX DMA 收到的 byte stream 寫入此 Ring Buffer。
 * 	CommunicationTask 再從此 Ring Buffer 中取出 byte，組成完整 command line。
 */
static RingBuffer_t UartRxRingBuffer;

/**
 * 	@brief Sensor error log last output tick
 *
 * 	@details
 * 	- 每一種 Sensor error log ID 對應一個 last tick
 * 	- 用於 Time-Based Rate Limiting Algorithm
 */
static uint32_t SensorErrorLastLogTick[SENSOR_ERROR_LOG_COUNT] =
{ 0U };

static uint32_t WarningTaskLastLogTick = 0U;	/// WarningTask Log Throttling 限制警示等級變化 Log 輸出頻率

static uint32_t SensorReconnectTryLastLogTick = 0U;
static uint32_t SensorReconnectFailLastLogTick = 0U;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes =
{ .name = "defaultTask", .stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes =
{ .name = "SensorTask", .stack_size = 512 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for WarningTask */
osThreadId_t WarningTaskHandle;
const osThreadAttr_t WarningTask_attributes =
{ .name = "WarningTask", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityBelowNormal1, };
/* Definitions for CommunicationTa */
osThreadId_t CommunicationTaHandle;
const osThreadAttr_t CommunicationTa_attributes =
{ .name = "CommunicationTa", .stack_size = 512 * 4, .priority = (osPriority_t) osPriorityLow, };
/* Definitions for SensorEventQueue */
osMessageQueueId_t SensorEventQueueHandle;
const osMessageQueueAttr_t SensorEventQueue_attributes =
{ .name = "SensorEventQueue" };
/* Definitions for DistanceQueue */
osMessageQueueId_t DistanceQueueHandle;
const osMessageQueueAttr_t DistanceQueue_attributes =
{ .name = "DistanceQueue" };
/* Definitions for SensorControlQueue */
osMessageQueueId_t SensorControlQueueHandle;
const osMessageQueueAttr_t SensorControlQueue_attributes =
{ .name = "SensorControlQueue" };
/* Definitions for BuzzerControlQueue */
osMessageQueueId_t BuzzerControlQueueHandle;
const osMessageQueueAttr_t BuzzerControlQueue_attributes =
{ .name = "BuzzerControlQueue" };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartSensorTask(void *argument);
void StartWarningTask(void *argument);
void StartCommunicationTask(void *argument);

static void Communication_UART_RxStart(void);	/// 20260515 Reversion 新增
static bool Communication_ReadLine(char *LineBuffer, uint16_t LineBufferSize);	/// 20260517 Reversion 新增
static SystemWarningLevel_t WarningTask_ConvertToSystemLevel(WarningLevel_t Level);	/// 20260518 新增
static void SensorTask_LogErrorThrottled(SensorErrorLogId_t ErrorId, const char *Message, int8_t Status);
static void SensorTask_LogReconnectTryThrottled(void);
static void SensorTask_ResetReconnectLogThrottle(void);
static void SensorTask_LogReconnectFailThrottled(const char *Message, int8_t Status);
static void WarningTask_LogThrottled(WarningLevel_t WarningLevel, uint16_t Distance);

static bool SensorTask_TryReconnect(void);
static void SensorTask_HandleSensorError(SensorError_t Error, SensorErrorLogId_t LogId, const char *Message, int32_t Status);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartSensorTask(void *argument);
void StartWarningTask(void *argument);
void StartCommunicationTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
	/* USER CODE BEGIN Init */

	DebugControl_Init();
	SystemHealth_Init();
	SensorHealth_Init();
	TaskMonitor_Init();

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* creation of SensorEventQueue */
	SensorEventQueueHandle = osMessageQueueNew(8, sizeof(uint8_t), &SensorEventQueue_attributes);

	/* creation of DistanceQueue */
	DistanceQueueHandle = osMessageQueueNew(8, sizeof(uint16_t), &DistanceQueue_attributes);

	/* creation of SensorControlQueue */
	SensorControlQueueHandle = osMessageQueueNew(4, sizeof(uint8_t), &SensorControlQueue_attributes);

	/* creation of BuzzerControlQueue */
	BuzzerControlQueueHandle = osMessageQueueNew(8, sizeof(uint8_t), &BuzzerControlQueue_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */

	QueueStatus_Register(SensorEventQueueHandle, DistanceQueueHandle, SensorControlQueueHandle, BuzzerControlQueueHandle);
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

	/* creation of SensorTask */
	SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);

	/* creation of WarningTask */
	WarningTaskHandle = osThreadNew(StartWarningTask, NULL, &WarningTask_attributes);

	/* creation of CommunicationTa */
	CommunicationTaHandle = osThreadNew(StartCommunicationTask, NULL, &CommunicationTa_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	TaskMonitor_Register(SensorTaskHandle, WarningTaskHandle, CommunicationTaHandle);
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
	/* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	for (;;)
	{
		osDelay(1);
	}
	/* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartSensorTask */

/**	@brief	SensorTask 主迴圈 : 負責 VL53L1X 測距、DataReady event 處理與 reconnect FSM
 *
 * 	@details
 *  - 此 Task 是整個系統中唯一負責操作 VL53L1X runtime I2C API 的 Task
 *  - GPIO EXTI ISR 不直接讀取 Sensor, 只會將 SENSOR_EVENT_DATA_READY 送入 SensorEventQueue
 *  - SensorTask 收到事件後，才執行 CheckForDataReady、GetDistance、ClearInterrupt 等 I2C 操作
 *
 *  FLOW
 *  Step 1: 執行 VL53L1X Hard Reset, 等待 BootState Ready
 *  Step 2: 初始化 VL53L1X, 設定 DistanceMode / TimingBudget / InterMeasurement / InterruptPolarity
 *  Step 3: 啟動 ranging, 並更新 SensorHealth 為 ONLINE
 *  Step 4: 非阻塞檢查 SensorControlQueue, 處理 CLI reconnect 指令
 *  Step 5: 執行 SensorHealth state machine :
 *          - OFFLINE 時週期性 reconnect
 *          - ONLINE/ERROR 時檢查 No-Data Timeout
 *  Step 6: 從 SensorEventQueue 等待 DataReady event
 *  Step 7: 成功讀取距離後, 更新 SystemStatus 並送入 DistanceQueue
 *  Step 8: 若 I2C / Sensor API 發生錯誤, 記錄 SystemHealth / SensorHealth, 並在連續錯誤達門檻後切換為 OFFLINE
 *
 * 	@param	argument	CMSIS-RTOS task argument, 目前未使用
 *
 * 	@return	None
 */

/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void *argument)
{
	/* USER CODE BEGIN StartSensorTask */
	/* Infinite loop */

	int8_t Status = 0;
	uint8_t BootState = 0U;
	uint8_t DataReady = 0U;
	uint8_t SensorEvent = 0U;
	uint8_t SensorControlRaw = 0U;
	uint16_t Device = VL53L1X_DEVICE_ADDR;
	uint16_t Distance = 0U;
	uint32_t CurrentTick = 0U;
	uint32_t BootStartTick = 0U;

	static uint32_t SensorLastRetryTick = 0U;

	SensorControlEvent_t SensorControlEvent = SENSOR_CONTROL_NONE;
	SensorHealthSnapshot_t SensorSnapshot;

	SensorHealth_SetState(SENSOR_STATE_INIT);

	Debug_Print("\r\n[Boot] Debug Layer Ready\r\n");
	Debug_Process();

	VL53L1_HardReset();						/// Step 1: VL53L1X Hard Reset

	BootStartTick = osKernelGetTickCount();	/// Step 2: Wait VL53L1X Boot(如果開機時 Sensor 沒接，必須進入 OFFLINE，讓後面的 retry state machine 接手)

	while (BootState == 0U)
	{
		Status = VL53L1X_BootState(Device, &BootState);

		if (Status != 0)
		{
			SensorHealth_ReportError(SENSOR_ERROR_BOOT_STATE);
			SensorHealth_SetOffline();
			SystemHealth_RecordI2cError();
			SensorTask_LogErrorThrottled(SENSOR_ERROR_LOG_BOOT_STATE, "VL53L1X BootState Failed", Status);

			/*
			 * 即使開機階段 Sensor 初始化失敗，也不結束 SensorTask
			 * SensorTask 會進入主迴圈，交由 OFFLINE reconnect FSM
			 * 週期性嘗試重新初始化 VL53L1X。
			 */
			goto SensorTask_MainLoop;
		}

		if ((osKernelGetTickCount() - BootStartTick) >= VL53L1X_BOOT_TIMEOUT_MS)
		{
			SensorHealth_ReportError(SENSOR_ERROR_BOOT_STATE);
			SensorHealth_SetOffline();
			SystemHealth_RecordI2cTimeout();
			SensorTask_LogErrorThrottled(SENSOR_ERROR_LOG_BOOT_STATE, "VL53L1X BootState Timeout", -1);

			goto SensorTask_MainLoop;
		}

		osDelay(VL53L1X_BOOT_POLL_INTERVAL_MS);

		Debug_Process();
	}

	/// Step 3: Sensor Init
	Status = VL53L1X_SensorInit(Device);

	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();
		SystemHealth_RecordI2cError();
		SensorTask_LogErrorThrottled(SENSOR_ERROR_LOG_SENSOR_INIT, "VL53L1X SensorInit Failed", Status);

		goto SensorTask_MainLoop;
	}

	/// Step 4: Sensor Configuration
	Status = VL53L1X_SetDistanceMode(Device, VL53L1X_DISTANCE_MODE_LONG);

	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();
		SystemHealth_RecordI2cError();
		SensorTask_LogErrorThrottled(SENSOR_ERROR_LOG_SET_DISTANCE_MODE, "VL53L1X SetDistanceMode Failed", Status);

		goto SensorTask_MainLoop;
	}

	Status = VL53L1X_SetTimingBudgetInMs(Device, VL53L1X_TIMING_BUDGET_MS);

	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();
		SystemHealth_RecordI2cError();
		SensorTask_LogErrorThrottled(SENSOR_ERROR_LOG_SET_TIMING_BUDGET, "VL53L1X SetTimingBudgetInMs Failed", Status);

		goto SensorTask_MainLoop;
	}

	Status = VL53L1X_SetInterMeasurementInMs(Device, VL53L1X_INTER_MEASUREMENT_MS);

	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();
		SystemHealth_RecordI2cError();
		SensorTask_LogErrorThrottled(SENSOR_ERROR_LOG_SET_INTER_MEASUREMENT, "VL53L1X SetInterMeasurementInMs Failed", Status);

		goto SensorTask_MainLoop;
	}

	Status = VL53L1X_SetInterruptPolarity(Device, VL53L1X_INTERRUPT_POLARITY_ACTIVE_LOW);

	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();
		SystemHealth_RecordI2cError();
		SensorTask_LogErrorThrottled(SENSOR_ERROR_LOG_SET_INTERRUPT_POLARITY, "VL53L1X SetInterruptPolarity Failed", Status);

		goto SensorTask_MainLoop;
	}

	/// Step 5: Start Ranging
	Status = VL53L1X_StartRanging(Device);

	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_START_RANGING);
		SensorHealth_SetOffline();
		SystemHealth_RecordI2cError();
		SensorTask_LogErrorThrottled(SENSOR_ERROR_LOG_START_RANGING, "VL53L1X StartRanging Failed", Status);

		goto SensorTask_MainLoop;
	}

	SensorHealth_ReportSuccess();

	Debug_Print("\r\n[Boot] VL53L1X Initialization OK\r\n");
	Debug_Print("[Boot] Start Ranging\r\n");
	Debug_Process();

	SensorTask_MainLoop:

	for (;;)
	{
		CurrentTick = osKernelGetTickCount();

		if (SensorControlQueueHandle != NULL)
		{
			if (osMessageQueueGet(SensorControlQueueHandle, &SensorControlRaw, NULL, 0U) == osOK)
			{
				SensorControlEvent = (SensorControlEvent_t) SensorControlRaw;

				switch (SensorControlEvent)
				{

				case SENSOR_CONTROL_RECONNECT:

					Debug_Print("[SensorTask] CLI Reconnect Command Received\r\n");
					Debug_Process();

					if (SensorTask_TryReconnect() == true)
					{
						CurrentTick = osKernelGetTickCount();

						SensorLastRetryTick = CurrentTick;

						Debug_Print("[SensorTask] Reconnect timeout base reset\r\n");
						Debug_Process();
					}

					break;

				case SENSOR_CONTROL_NONE:
				default:
					break;
				}
			}
		}

		/**
		 * Step 6: Sensor health state machine
		 *
		 * @details
		 * 演算法名稱：
		 * - Watchdog Timeout Algorithm
		 * - Periodic Reconnect Algorithm
		 *
		 * Step 1. 取得 SensorHealth snapshot。
		 * Step 2. 若 Sensor 已經 OFFLINE，週期性執行 reconnect。
		 * Step 3. 若 Sensor 尚未 OFFLINE，但太久沒有成功更新距離，
		 *         則判定為 No-Data Timeout，切換為 OFFLINE。
		 */
		if (SensorHealth_GetSnapshot(&SensorSnapshot) == true)
		{
			/// Step 6-1: OFFLINE retry flow
			/// Sensor 已經離線時, 不再等待 EXTI event, 每 SENSOR_RETRY_INTERVAL_MS 嘗試重新初始化 VL53L1X
			if (SensorSnapshot.State == SENSOR_STATE_OFFLINE)
			{
				if ((SensorLastRetryTick == 0U) || ((CurrentTick - SensorLastRetryTick) >= SENSOR_RETRY_INTERVAL_MS))
				{
					SensorLastRetryTick = CurrentTick;

					if (SensorTask_TryReconnect() == true)
					{
						SensorLastRetryTick = 0U;
					}
				}

				Debug_Process();
				osDelay(SENSOR_OFFLINE_LOOP_DELAY_MS);

				continue;
			}

			/**
			 * Step 6-2: No-data timeout detection
			 *
			 * Sensor 拔掉後，EXTI / DataReady 可能不再觸發。
			 * 因此不能只依賴 Queue event 來累積錯誤。
			 *
			 * 若距離上次成功讀取 Sensor 的時間超過 SENSOR_NO_DATA_TIMEOUT_MS，
			 * 則視為 Sensor 已失聯，強制切換為 OFFLINE，讓 retry state machine 接手。
			 */
			if ((SensorSnapshot.State == SENSOR_STATE_ONLINE) || (SensorSnapshot.State == SENSOR_STATE_ERROR))
			{
				if ((SensorSnapshot.LastSuccessTick != 0U) && ((CurrentTick - SensorSnapshot.LastSuccessTick) >= SENSOR_NO_DATA_TIMEOUT_MS))
				{
					SensorHealth_ReportError(SENSOR_ERROR_CHECK_DATA_READY);
					SensorHealth_SetOffline();

					SystemHealth_RecordSensorDataReadyTimeout();

					SensorTask_LogErrorThrottled(SENSOR_ERROR_LOG_CHECK_DATA_READY, "VL53L1X No Data Timeout", -1);

					Debug_Process();
					osDelay(SENSOR_OFFLINE_LOOP_DELAY_MS);
					continue;
				}
			}
		}

		/**
		 * Step 7: 正常 DataReady event-driven flow
		 *
		 * 注意：
		 * 這裡不能用 osWaitForever。
		 * 使用 timeout 才能讓 SensorTask 週期性醒來檢查 OFFLINE retry。
		 */
		if (osMessageQueueGet(SensorEventQueueHandle, &SensorEvent, NULL, SENSOR_EVENT_QUEUE_WAIT_MS) == osOK)
		{
			if (SensorEvent == SENSOR_EVENT_DATA_READY)
			{
				DataReady = 0U;

				Status = VL53L1X_CheckForDataReady(Device, &DataReady);
				if (Status != 0)
				{
					SensorTask_HandleSensorError(SENSOR_ERROR_CHECK_DATA_READY, SENSOR_ERROR_LOG_CHECK_DATA_READY, "VL53L1X CheckForDataReady Failed", Status);
					continue;
				}

				if (DataReady != 0U)
				{
					Status = VL53L1X_GetDistance(Device, &Distance);

					if (Status == 0)
					{
						SensorHealth_ReportSuccess();
						SystemStatus_UpdateDistance(Distance);

						if (DistanceQueueHandle != NULL)
						{
							/// 20260601 Version
							osStatus_t QueueStatus;

							QueueStatus = osMessageQueuePut(DistanceQueueHandle, &Distance, 0U, 0U);

							if (QueueStatus != osOK)
							{
								SystemHealth_RecordQueueOverflow();
							}
						}
					}
					else
					{
						SensorTask_HandleSensorError(SENSOR_ERROR_GET_DISTANCE, SENSOR_ERROR_LOG_GET_DISTANCE, "VL53L1X GetDistance Failed", Status);
					}

					Status = VL53L1X_ClearInterrupt(Device);

					if (Status != 0)
					{
						SensorTask_HandleSensorError(SENSOR_ERROR_CLEAR_INTERRUPT, SENSOR_ERROR_LOG_CLEAR_INTERRUPT, "VL53L1X ClearInterrupt Failed", Status);
					}

					Debug_Process();
				}
			}
		}
	}

	/* USER CODE END StartSensorTask */
}

/* USER CODE BEGIN Header_StartWarningTask */

/**	@brief WarningTask 警示等級變化 Log 節流輸出
 *
 * 	@details
 * 	Step 1: 取得目前 RTOS Tick
 * 	Step 2: 若第一次輸出, 允許立即輸出
 * 	Step 3: 若距離上次輸出超過 WARNING_TASK_LOG_INTERVAL_MS 允許輸出
 * 	Step 4: 輸出目前 WarningLevel 和 Distance
 * 	Step 5: 更新 WarningTaskLastLogTick
 *
 *	@param	WarningLevel	目前警示等級
 *	@param	Distance		目前距離(mm)
 *
 *	@return	None
 */
static void WarningTask_LogThrottled(WarningLevel_t WarningLevel, uint16_t Distance)
{
	uint32_t CurrentTick;

	CurrentTick = osKernelGetTickCount();

	if ((WarningTaskLastLogTick == 0U) || ((CurrentTick - WarningTaskLastLogTick) >= WARNING_TASK_LOG_INTERVAL_MS))
	{
		Debug_Print("[WarningTask] Level = %s, Distance = %u (mm)\r\n", SystemStatus_WarningLevelToString(WarningTask_ConvertToSystemLevel(WarningLevel)), Distance);
		Debug_Process();

		WarningTaskLastLogTick = CurrentTick;
	}
}

/**	@brief	WarningTask 主迴圈 : 負責距離警示等級、LED 輸出與 Buzzer 節奏控制。
 *
 * 	@details
 *  - 此 Task 從 DistanceQueue 接收 SensorTask 送出的距離值, 並依照
 *    WarningConfigService 中設定的 enter / exit threshold 執行 hysteresis
 *    判斷, 避免距離在臨界點附近造成警示等級頻繁抖動
 *
 *  - WarningTask 同時集中管理 LED 與有源蜂鳴器輸出, 其他 Task 不直接控制
 *    Buzzer GPIO, 而是透過 BuzzerControlQueue 傳送控制事件。
 *
 *  FLOW :
 *  Step 1: 初始化 LED / Buzzer 為 SAFE 狀態
 *  Step 2: 非阻塞檢查 BuzzerControlQueue，處理 CLI buzzer 指令
 *  Step 3: 以短 timeout 從 DistanceQueue 取得最新距離
 *  Step 4: 根據 hysteresis threshold 更新 WarningLevel
 *  Step 5: 警示等級變化時, 更新 LED 與 Buzzer ON/OFF pattern
 *  Step 6: 執行 Buzzer output FSM:
 *          - Test mode
 *          - Forced ON
 *          - Muted
 *          - Normal warning pattern
 *  Step 7: 更新 SystemStatus, 供 CLI status / monitor 查詢。
 *
 * 	@param	argument	CMSIS-RTOS task argument，目前未使用。
 *
 * 	@return	None
 */

/* USER CODE END Header_StartWarningTask */
void StartWarningTask(void *argument)
{
	/* USER CODE BEGIN StartWarningTask */

	uint16_t Distance = 0U;
	WarningLevel_t Warning_Level = WARNING_LEVEL_SAFE;			/// 預設警示距離(此次計算距離等級)
	WarningLevel_t PreviousWarning_Level = WARNING_LEVEL_SAFE;	/// 上一次已經輸出過的等級

	uint32_t BuzzerTick = 0U;
	uint32_t BuzzerOnTime = 0U;
	uint32_t BuzzerOffTime = 0U;
	uint8_t BuzzerIsOn = 0U;

	uint8_t BuzzerControlRaw = 0U;
	BuzzerControlEvent_t BuzzerControlEvent = BUZZER_CONTROL_NONE;

	uint8_t BuzzerMuted = 0U;
	uint8_t BuzzerForcedOn = 0U;
	uint32_t BuzzerTestEndTick = 0U;

	WARNING_LED_SAFE();
	Buzzer_Off();

	/// 20260518 Reversion => 新增檢查機制 : 即使尚未收到距離，CLI 查 Status
	SystemStatus_UpdateWarningLevel(WarningTask_ConvertToSystemLevel(Warning_Level));

	SystemStatus_UpdateBuzzerState(0U);

	/* Infinite loop */
	for (;;)
	{
		if (BuzzerControlQueueHandle != NULL)
		{
			if (osMessageQueueGet(BuzzerControlQueueHandle, &BuzzerControlRaw, NULL, 0U) == osOK)
			{
				BuzzerControlEvent = (BuzzerControlEvent_t) BuzzerControlRaw;

				switch (BuzzerControlEvent)
				{
				case BUZZER_CONTROL_ON:

					BuzzerMuted = 0U;
					BuzzerForcedOn = 1U;
					BuzzerTestEndTick = 0U;

					Buzzer_On();
					BuzzerIsOn = 1U;
					SystemStatus_UpdateBuzzerState(1U);

					Debug_Print("[WarningTask] Buzzer Force ON\r\n");
					Debug_Process();

					break;

				case BUZZER_CONTROL_OFF:

					BuzzerForcedOn = 0U;
					BuzzerTestEndTick = 0U;

					Buzzer_Off();
					BuzzerIsOn = 0U;
					BuzzerTick = osKernelGetTickCount();
					SystemStatus_UpdateBuzzerState(0U);

					Debug_Print("[WarningTask] Buzzer Force OFF Released\r\n");
					Debug_Process();

					break;

				case BUZZER_CONTROL_TEST:

					BuzzerMuted = 0U;
					BuzzerForcedOn = 0U;
					BuzzerTestEndTick = osKernelGetTickCount() + BUZZER_TEST_DURATION_MS;

					Buzzer_On();
					BuzzerIsOn = 1U;
					SystemStatus_UpdateBuzzerState(1U);

					Debug_Print("[WarningTask] Buzzer Test %lu ms\r\n", (unsigned long) BUZZER_TEST_DURATION_MS);
					Debug_Process();

					break;

				case BUZZER_CONTROL_MUTE:

					BuzzerMuted = 1U;
					BuzzerForcedOn = 0U;
					BuzzerTestEndTick = 0U;

					Buzzer_Off();
					BuzzerIsOn = 0U;
					SystemStatus_UpdateBuzzerState(0U);

					Debug_Print("[WarningTask] Buzzer Muted\r\n");
					Debug_Process();

					break;

				case BUZZER_CONTROL_UNMUTE:

					BuzzerMuted = 0U;
					BuzzerForcedOn = 0U;
					BuzzerTestEndTick = 0U;
					BuzzerTick = osKernelGetTickCount();

					Debug_Print("[WarningTask] Buzzer Unmuted\r\n");
					Debug_Process();

					break;

				case BUZZER_CONTROL_NONE:
				default:
					break;
				}
			}
		}

		/// 20260512 版本修改
		/// => 蜂鳴器需要[週期性: ON/OFF], 所以 WarningTasK 不能永遠卡在 osWaitForever
		/// => 改成最多等 10 ms, 有距離資料就更新警示等級
		/// => 沒距離資料也繼續往下跑, 維持 Buzzer 節奏
		/// => if (osMessageQueueGet(DistanceQueueHandle, &Distance, NULL, osWaitForever) == osOK)
		if (osMessageQueueGet(DistanceQueueHandle, &Distance, NULL, WARNING_DISTANCE_QUEUE_WAIT_MS) == osOK)
		{
			switch (Warning_Level)
			{

			case WARNING_LEVEL_DANGER:

				if (Distance > WarningConfig_GetDangerExitMm())
				{
					Warning_Level = WARNING_LEVEL_WARNING;
				}

				break;

			case WARNING_LEVEL_WARNING:

				if (Distance <= WarningConfig_GetDangerEnterMm())
				{
					Warning_Level = WARNING_LEVEL_DANGER;
				}
				else if (Distance > WarningConfig_GetWarningExitMm())
				{
					Warning_Level = WARNING_LEVEL_CAUTION;
				}

				break;

			case WARNING_LEVEL_CAUTION:

				if (Distance <= WarningConfig_GetWarningEnterMm())
				{
					Warning_Level = WARNING_LEVEL_WARNING;
				}
				else if (Distance > WarningConfig_GetCautionExitMm())
				{
					Warning_Level = WARNING_LEVEL_SAFE;
				}

				break;

			case WARNING_LEVEL_SAFE:
			default:

				if (Distance <= WarningConfig_GetDangerEnterMm())
				{
					Warning_Level = WARNING_LEVEL_DANGER;
				}
				else if (Distance <= WarningConfig_GetWarningEnterMm())
				{
					Warning_Level = WARNING_LEVEL_WARNING;
				}
				else if (Distance <= WarningConfig_GetCautionEnterMm())
				{
					Warning_Level = WARNING_LEVEL_CAUTION;
				}
				else
				{
					Warning_Level = WARNING_LEVEL_SAFE;
				}

				break;
			}

			/**
			 * Step F-6.5: Buzzer control command handling
			 *
			 * @details
			 * 演算法名稱：Finite State Machine / Event-Driven Control
			 *
			 * Step 1. 非阻塞讀取 BuzzerControlQueue
			 * Step 2. 根據 CLI event 更新 Buzzer control state
			 * Step 3. Buzzer GPIO 實際控制仍集中在 WarningTask
			 */

			/// 即使 Warning_Level 沒有變化, 只要 WarningTask 有處理新距離, WarningUpdateCount 仍會增加
			SystemStatus_UpdateWarningLevel(WarningTask_ConvertToSystemLevel(Warning_Level));

			if (Warning_Level != PreviousWarning_Level)
			{
				switch (Warning_Level)
				{

				case WARNING_LEVEL_DANGER:

					WARNING_LED_DANGER();
					BuzzerOnTime = BUZZER_ON_TIME_DANGER_MS;
					BuzzerOffTime = BUZZER_OFF_TIME_DANGER_MS;

					break;

				case WARNING_LEVEL_WARNING:

					WARNING_LED_WARNING();
					BuzzerOnTime = BUZZER_ON_TIME_WARNING_MS;
					BuzzerOffTime = BUZZER_OFF_TIME_WARNING_MS;

					break;

				case WARNING_LEVEL_CAUTION:

					WARNING_LED_CAUTION();
					BuzzerOnTime = BUZZER_ON_TIME_CAUTION_MS;
					BuzzerOffTime = BUZZER_OFF_TIME_CAUTION_MS;

					break;

				case WARNING_LEVEL_SAFE:
				default:

					WARNING_LED_SAFE();
					Buzzer_Off();
					BuzzerOnTime = 0U;
					BuzzerOffTime = 0U;
					BuzzerIsOn = 0U;
					SystemStatus_UpdateBuzzerState(0U);

					break;
				}

				if (DebugControl_IsLogEnable() != 0U)
				{
					WarningTask_LogThrottled(Warning_Level, Distance);
				}

				PreviousWarning_Level = Warning_Level;	/// 更新前一次警示等級
			}
		}

		/**
		 * Step F-6.5: Buzzer output state machine
		 *
		 * @details
		 * 優先順序：
		 * 1. Test mode
		 * 2. Forced ON
		 * 3. Muted
		 * 4. Normal warning pattern
		 */
		if (BuzzerTestEndTick != 0U)
		{
			if ((int32_t) (osKernelGetTickCount() - BuzzerTestEndTick) >= 0)
			{
				Buzzer_Off();
				BuzzerIsOn = 0U;
				BuzzerTestEndTick = 0U;
				BuzzerTick = osKernelGetTickCount();
				SystemStatus_UpdateBuzzerState(0U);
			}
		}
		else if (BuzzerForcedOn != 0U)
		{
			if (BuzzerIsOn == 0U)
			{
				Buzzer_On();
				BuzzerIsOn = 1U;
				SystemStatus_UpdateBuzzerState(1U);
			}
		}
		else if (BuzzerMuted != 0U)
		{
			if (BuzzerIsOn != 0U)
			{
				Buzzer_Off();
				BuzzerIsOn = 0U;
				SystemStatus_UpdateBuzzerState(0U);
			}
		}
		else
		{
			if (Warning_Level == WARNING_LEVEL_SAFE)
			{
				if (BuzzerIsOn != 0U)
				{
					Buzzer_Off();
					BuzzerIsOn = 0U;
					BuzzerTick = osKernelGetTickCount();
					SystemStatus_UpdateBuzzerState(0U);
				}
			}
			else
			{
				uint32_t CurrentBuzzerTick = osKernelGetTickCount();

				if (BuzzerIsOn != 0U)
				{
					if ((CurrentBuzzerTick - BuzzerTick) >= BuzzerOnTime)
					{
						Buzzer_Off();
						BuzzerIsOn = 0U;
						BuzzerTick = CurrentBuzzerTick;
						SystemStatus_UpdateBuzzerState(0U);
					}
				}
				else
				{
					if ((CurrentBuzzerTick - BuzzerTick) >= BuzzerOffTime)
					{
						Buzzer_On();
						BuzzerIsOn = 1U;
						BuzzerTick = CurrentBuzzerTick;
						SystemStatus_UpdateBuzzerState(1U);
					}
				}
			}
		}
	}

	/* USER CODE END StartWarningTask */
}

/* USER CODE BEGIN Header_StartCommunicationTask */

/**	20260518 版本更新: 新增轉換函式
 * 	@brief 將 WarningTask 內部警示等級轉換成 SystemStatusService 警示等級
 *
 * 	@details
 * 	演算法名稱：Enum-to-Enum Mapping
 *
 * 	Step 1. 依照 WarningTask 的 WarningLevel_t 進行 switch 判斷
 * 	Step 2. 將內部 warning level 轉換成 SystemWarningLevel_t
 * 	Step 3. 若無法識別，回傳 SYSTEM_WARNING_LEVEL_UNKNOWN
 *
 * 	@param Level WarningTask 內部警示等級
 *
 * 	@return SystemWarningLevel_t 給 SystemStatusService 使用的警示等級
 */
static SystemWarningLevel_t WarningTask_ConvertToSystemLevel(WarningLevel_t Level)
{
	SystemWarningLevel_t SystemLevel;

	switch (Level)
	{
	case WARNING_LEVEL_SAFE:
		SystemLevel = SYSTEM_WARNING_LEVEL_SAFE;
		break;

	case WARNING_LEVEL_CAUTION:
		SystemLevel = SYSTEM_WARNING_LEVEL_CAUTION;
		break;

	case WARNING_LEVEL_WARNING:
		SystemLevel = SYSTEM_WARNING_LEVEL_WARNING;
		break;

	case WARNING_LEVEL_DANGER:
		SystemLevel = SYSTEM_WARNING_LEVEL_DANGER;
		break;

	default:
		SystemLevel = SYSTEM_WARNING_LEVEL_UNKNOWN;
		break;
	}

	return SystemLevel;
}

/**	@brief	CommunicationTask 主迴圈:負責 UART RX DMA、CLI command 處理與 Debug TX 輸出
 *
 * 	@details
 *  - USART3 使用 Receive-To-Idle DMA 接收資料。
 *  - 當 UART 收到資料並偵測到 IDLE Line 時, HAL_UARTEx_RxEventCallback() 會將 DMA buffer 中的資料
 *    搬入 UART RX Ring Buffer，並透過 osThreadFlagsSet() 通知 CommunicationTask
 *  - CommunicationTask 被喚醒後,
 *    會從 Ring Buffer 取出完整 command line, 交給 CLI_ProcessLine() 進行 parser 與 command handler 處理
 *  - Task 也會週期性呼叫 Debug_Process(),
 *    讓 debug log 透過 UART TX DMA 非阻塞輸出, 避免 SensorTask / WarningTask 被 UART 傳輸阻塞
 *
 *  FLOW:
 *  Step 1: 初始化 UART RX Ring Buffer
 *  Step 2: 初始化 UART status counter
 *  Step 3: 初始化 CLI parser / command handler
 *  Step 4: 啟動 USART3 Receive-To-Idle DMA
 *  Step 5: 等待 UART RX ready thread flag
 *  Step 6: 從 Ring Buffer 擷取完整 command line
 *  Step 7: 呼叫 CLI_ProcessLine() 處理 CLI 指令
 *  Step 8: 呼叫 Debug_Process() 處理 pending debug output
 *
 * 	@param	argument	CMSIS-RTOS task argument，目前未使用
 *
 * 	@return	None
 */
/* USER CODE END Header_StartCommunicationTask */
void StartCommunicationTask(void *argument)
{
	/* USER CODE BEGIN StartCommunicationTask */

	uint8_t LocalRxBuffer[CLI_LINE_BUFFER_SIZE];
	uint32_t Flags;

	(void) argument;

	Debug_Print("[COM] CommunicationTask Started\r\n");
	Debug_Process();

	RingBuffer_Init(&UartRxRingBuffer);

	UartStatus_Init();

	CLI_Init();

	Communication_UART_RxStart();

	/* Infinite loop */
	for (;;)
	{
		Flags = osThreadFlagsWait(COMM_FLAG_UART_RX_READY, osFlagsWaitAny, 10U);

		if ((Flags & COMM_FLAG_UART_RX_READY) != 0U && (Flags & osFlagsError) == 0U)
		{
			while (Communication_ReadLine((char*) LocalRxBuffer, sizeof(LocalRxBuffer)) == true)
			{
				CLI_ProcessLine((char*) LocalRxBuffer);
			}
		}

		Debug_Process();
	}

	/* USER CODE END StartCommunicationTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * 	@brief	啟動 USART3 Receive-To-Idle DMA 接收
 *
 * 	@details
 * 	演算法 [Receive-To-Idle Framing]
 * 	Step 1: 使用 HAL_UARTEx_ReceiveToIdle_DMA() 啟動 USART3 RX DMA。
 * 	Step 2: UART 收到資料並偵測到 IDLE line 後，HAL 會呼叫 HAL_UARTEx_RxEventCallback()。
 * 	Step 3: 關閉 DMA Half Transfer interrupt，避免半滿時觸發 callback。
 *
 * 	@param	None
 *
 * 	@return	None
 */
static void Communication_UART_RxStart(void)
{
	HAL_StatusTypeDef HalStatus;

	HalStatus = HAL_UARTEx_ReceiveToIdle_DMA(&huart3, UartRxDmaBuffer, UART_RX_DMA_BUFFER_SIZE);

	if (HalStatus == HAL_OK)
	{
		__HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
	}
	else
	{
		Debug_Print("[COM] UART Rx DMA Start Failed, Status = %d\r\n", HalStatus);
		Debug_Process();
	}
}

/**
 * 	@brief	UART Receive-To-Idle callback
 *
 * 	@details
 * 	演算法 / Pattern：
 * 	- Receive-To-Idle Framing
 * 	- Producer-Consumer Pattern
 * 	- Double Buffering / Buffer Handoff
 * 	- Bounds Checking Algorithm
 *
 * 	Step 1: 確認 callback 來源為 USART3。
 * 	Step 2: 開啟 DCache 時，CPU 讀 DMA buffer 前先 invalidate cache。
 * 	Step 3: 使用 Bounds Checking Algorithm 限制處理長度。
 * 	Step 4: 將 DMA buffer 收到的 byte stream 寫入 Ring Buffer。
 * 	Step 5: 使用 osThreadFlagsSet() 通知 CommunicationTask。
 * 	Step 6: 重新啟動 Receive-To-Idle DMA。
 *
 * 	@param	huart	UART handle。
 * 	@param	Size	本次 RX DMA 收到的資料長度。
 *
 * 	@return	None
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	uint16_t Index;
	uint16_t CopyLength;

	if (huart->Instance == USART3)
	{
		UartStatus_OnRxEvent(Size);

		/*
		 * STM32H7 DCache 注意：
		 * DMA 已經將資料寫入 RAM，但 CPU 可能仍讀到舊 cache。
		 * 因此 CPU 讀取 DMA buffer 前需要 invalidate cache。
		 */
		SCB_InvalidateDCache_by_Addr((uint32_t*) UartRxDmaBuffer, UART_RX_DMA_BUFFER_SIZE);

		CopyLength = Size;

		if (CopyLength > UART_RX_DMA_BUFFER_SIZE)
		{
			CopyLength = UART_RX_DMA_BUFFER_SIZE;
		}

		for (Index = 0U; Index < CopyLength; Index++)
		{
			if (RingBuffer_WriteByte(&UartRxRingBuffer, UartRxDmaBuffer[Index]) == false)
			{
				UartStatus_AddOverflow();
				SystemHealth_RecordUartRxOverflow();
				break;
			}
		}

		if (CommunicationTaHandle != NULL)
		{
			(void) osThreadFlagsSet(CommunicationTaHandle, COMM_FLAG_UART_RX_READY);
		}

		Communication_UART_RxStart();
	}
}

/**
 * 	@brief	從 UART RX Ring Buffer 讀取一行 command
 *
 * 	@details
 * 	演算法 [Line Extraction Algorithm / Delimiter-Based Parsing]
 *
 * 	Step 1: 從 Ring Buffer 逐 byte 讀出資料。
 * 	Step 2: 若讀到 '\r' 或 '\n'，代表一行結束。
 * 	Step 3: 若目前 line 長度大於 0，補上 '\0' 並回傳 true。
 * 	Step 4: 若是連續的 '\r' / '\n'，忽略空行。
 * 	Step 5: 若一般字元超過 LineBufferSize，則截斷，避免 overflow。
 *
 * 	@param	LineBuffer		輸出 command line buffer。
 * 	@param	LineBufferSize	line buffer 大小。
 *
 * 	@return bool	true : 成功取得完整一行 ? false: 尚未取得完整一行
 */
static bool Communication_ReadLine(char *LineBuffer, uint16_t LineBufferSize)
{
	static uint16_t LineLength = 0U;
	uint8_t Data;

	if ((LineBuffer == NULL) || (LineBufferSize == 0U))
	{
		return false;
	}

	while (RingBuffer_ReadByte(&UartRxRingBuffer, &Data) == true)
	{
		if ((Data == '\r') || (Data == '\n'))
		{
			if (LineLength > 0U)
			{
				LineBuffer[LineLength] = '\0';
				LineLength = 0U;
				return true;
			}

			/*
			 * Ignore empty line caused by CR/LF sequence.
			 */
			continue;
		}

		if (LineLength < (uint16_t) (LineBufferSize - 1U))
		{
			LineBuffer[LineLength] = (char) Data;
			LineLength++;
		}
		else
		{
			/*
			 * LineBuffer Overflow Protection:
			 * 若 command 太長，直接清空目前 line。
			 */
			LineLength = 0U;
		}
	}

	return false;
}

/**
 * 	@brief	SensorTask error log 節流輸出
 *
 * 	@details
 * 	演算法 [Time-Based Rate Limiting Algorithm]
 *
 * 	Step 1: 檢查 ErrorId 是否有效
 * 	Step 2: 檢查 Message 指標是否有效
 * 	Step 3: 取得目前 RTOS tick
 * 	Step 4: 若距離上次同類錯誤輸出已超過 SENSOR_ERROR_LOG_INTERVAL_MS，允許輸出
 * 	Step 5: 更新該錯誤類型的最後輸出時間
 *
 * 	@param	ErrorId	Sensor error log ID。
 * 	@param	Message	Error message。
 * 	@param	Status	Error status。
 *
 * 	@return	None
 */
static void SensorTask_LogErrorThrottled(SensorErrorLogId_t ErrorId, const char *Message, int8_t Status)
{
	uint32_t CurrentTick;

	if (ErrorId >= SENSOR_ERROR_LOG_COUNT)
	{
		return;
	}

	if (Message == NULL)
	{
		return;
	}

	CurrentTick = osKernelGetTickCount();

	if (((CurrentTick - SensorErrorLastLogTick[ErrorId]) >= SENSOR_ERROR_LOG_INTERVAL_MS) || (SensorErrorLastLogTick[ErrorId] == 0U))
	{
		Debug_Print("[SensorTask][Error] %s = %d\r\n", Message, Status);
		Debug_Process();

		SensorErrorLastLogTick[ErrorId] = CurrentTick;
	}
}

/**	@brief	Sensor Reconnect Try Log 節流輸出
 *
 *	@details
 *	演算法 [Time-Based Rate Limiting Algorithm]
 *
 *	Step 1: 取得目前 RTOS Tick
 *	Step 2: 若第一次輸出, 允許立即輸出
 * 	Step 3: 若距離上次輸出超過 SENSOR_RECONNECT_TRY_LOG_INTERVAL_MS, 允許輸出
 * 	Step 4: 取得 SensorHealth snapshot, 輸出目前 retry count
 * 	Step 5: 更新 SensorReconnectTryLastLogTick
 *
 *	@param	None
 *
 *	@return	None
 */
static void SensorTask_LogReconnectTryThrottled(void)
{
	uint32_t CurrentTick;
	SensorHealthSnapshot_t Snapshot;

	CurrentTick = osKernelGetTickCount();

	if ((SensorReconnectTryLastLogTick == 0U) || (CurrentTick - SensorReconnectTryLastLogTick) >= SENSOR_RECONNECT_TRY_LOG_INTERVAL_MS)
	{
		if (SensorHealth_GetSnapshot(&Snapshot) == true)
		{
			Debug_Print("[SensorTask] VL53L1X Reconnect Try, RetryCount = %lu\r\n", (unsigned long) Snapshot.RetryCount);
		}
		else
		{
			Debug_Print("[SensorTask] VL53L1X Reconnect Try\r\n");
		}

		Debug_Process();

		SensorReconnectTryLastLogTick = CurrentTick;
	}
}

/**	@brief	Sensor reconnect failed log 節流輸出
 *
 * 	@details
 * 	演算法	[Reconnect Failure Summary Log]
 *
 * 	Step 1: 檢查 Message 指標是否有效
 * 	Step 2: 取得目前 RTOS tick
 * 	Step 3: 若第一次輸出, 允許立即輸出
 * 	Step 4: 若距離上次輸出超過 SENSOR_RECONNECT_FAIL_LOG_INTERVAL_MS, 允許輸出
 * 	Step 5: 取得 SensorHealth snapshot, 輸出 retry / error / consecutive error summary
 * 	Step 6: 更新 SensorReconnectFailLastLogTick
 *
 * 	@param	Message	reconnect failed message
 * 	@param	Status	reconnect failed status
 *
 * 	@return	None
 */
static void SensorTask_LogReconnectFailThrottled(const char *Message, int8_t Status)
{
	uint32_t CurrentTick;
	SensorHealthSnapshot_t Snapshot;

	if (Message == NULL)
	{
		return;
	}

	CurrentTick = osKernelGetTickCount();

	if ((SensorReconnectFailLastLogTick == 0U) || (CurrentTick - SensorReconnectFailLastLogTick) >= SENSOR_RECONNECT_FAIL_LOG_INTERVAL_MS)
	{
		if (SensorHealth_GetSnapshot(&Snapshot) == true)
		{
			Debug_Print("[SensorTask][Reconnect] Failed: %s = %d, Retry = %lu, Error = %lu, Consecutive = %lu\r\n", Message, Status, (unsigned long) Snapshot.RetryCount,
			(unsigned long) Snapshot.ErrorCount, (unsigned long) Snapshot.ConsecutiveErrorCount);
		}
		else
		{
			Debug_Print("[SensorTask][Reconnect] Failed: %s = %d\r\n", Message, Status);
		}

		Debug_Process();

		SensorReconnectFailLastLogTick = CurrentTick;
	}
}

/**	@brief	重置 reconnect log 節流狀態
 *
 * 	@details
 * 	Step 1: 清除 reconnect try log last tick
 * 	Step 2: 清除 reconnect fail log last tick
 * 	- reconnect success 後重置節流狀態。
 * 	- 下一次 Sensor 再次離線時，可以立即看到第一次 reconnect try / fail log
 *
 * 	@param	None
 *
 * 	@return	None
 */
static void SensorTask_ResetReconnectLogThrottle(void)
{
	SensorReconnectTryLastLogTick = 0U;
	SensorReconnectFailLastLogTick = 0U;
}

/**
 * 	@brief	嘗試重新初始化 VL53L1X Sensor
 *
 * 	@details
 * 	演算法	[Periodic Reconnect Algorithm]
 *
 * 	Step 1: 回報 Sensor 進入 RETRYING 狀態
 * 	Step 2: 執行 VL53L1X Hard Reset
 * 	Step 3: 等待 VL53L1X BootState ready，並加入 timeout 保護
 * 	Step 4: 執行 VL53L1X_SensorInit()
 * 	Step 5: 重新設定 DistanceMode / TimingBudget / InterMeasurement / InterruptPolarity
 * 	Step 6: 執行 VL53L1X_StartRanging()
 * 	Step 7: 成功則回報 ONLINE，失敗則回報對應錯誤並維持 OFFLINE
 *
 * 	@return bool	true : reconnect success ? false: reconnect failed
 */
static bool SensorTask_TryReconnect(void)
{
	int32_t Status;
	uint8_t BootState = 0U;
	uint32_t BootStartTick = 0U;

	SystemHealth_RecordSensorReconnect();

	SensorHealth_ReportRetry();

	SensorTask_LogReconnectTryThrottled();

	VL53L1_HardReset();

	BootStartTick = osKernelGetTickCount();

	while (BootState == 0U)
	{
		Status = VL53L1X_BootState(VL53L1X_DEVICE_ADDR, &BootState);

		if (Status != 0)
		{
			SensorHealth_ReportError(SENSOR_ERROR_BOOT_STATE);
			SensorHealth_SetOffline();

			SystemHealth_RecordSensorReconnectFail();
			SystemHealth_RecordI2cError();

			SensorTask_LogReconnectFailThrottled("VL53L1X Reconnect BootState Failed", (int8_t) Status);

			return false;
		}

		if ((osKernelGetTickCount() - BootStartTick) >= VL53L1X_BOOT_TIMEOUT_MS)
		{
			SensorHealth_ReportError(SENSOR_ERROR_BOOT_STATE);
			SensorHealth_SetOffline();

			SystemHealth_RecordSensorReconnectFail();
			SystemHealth_RecordI2cTimeout();

			SensorTask_LogReconnectFailThrottled("VL53L1X Reconnect BootState Timeout", -1);

			return false;
		}

		osDelay(VL53L1X_BOOT_POLL_INTERVAL_MS);
	}

	Status = VL53L1X_SensorInit(VL53L1X_DEVICE_ADDR);
	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();

		SystemHealth_RecordSensorReconnectFail();
		SystemHealth_RecordI2cError();

		SensorTask_LogReconnectFailThrottled("VL53L1X Reconnect SensorInit Failed", (int8_t) Status);

		return false;
	}

	Status = VL53L1X_SetDistanceMode(VL53L1X_DEVICE_ADDR, VL53L1X_DISTANCE_MODE_LONG);
	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();

		SystemHealth_RecordSensorReconnectFail();
		SystemHealth_RecordI2cError();

		SensorTask_LogReconnectFailThrottled("VL53L1X Reconnect SetDistanceMode Failed", (int8_t) Status);

		return false;
	}

	Status = VL53L1X_SetTimingBudgetInMs(VL53L1X_DEVICE_ADDR, VL53L1X_TIMING_BUDGET_MS);
	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();

		SystemHealth_RecordSensorReconnectFail();
		SystemHealth_RecordI2cError();

		SensorTask_LogReconnectFailThrottled("VL53L1X Reconnect SetTimingBudget Failed", (int8_t) Status);

		return false;
	}

	Status = VL53L1X_SetInterMeasurementInMs(VL53L1X_DEVICE_ADDR, VL53L1X_INTER_MEASUREMENT_MS);
	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();

		SystemHealth_RecordSensorReconnectFail();
		SystemHealth_RecordI2cError();

		SensorTask_LogReconnectFailThrottled("VL53L1X Reconnect SetInterMeasurement Failed", (int8_t) Status);

		return false;
	}

	Status = VL53L1X_SetInterruptPolarity(VL53L1X_DEVICE_ADDR, VL53L1X_INTERRUPT_POLARITY_ACTIVE_LOW);
	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_SENSOR_INIT);
		SensorHealth_SetOffline();

		SystemHealth_RecordSensorReconnectFail();
		SystemHealth_RecordI2cError();

		SensorTask_LogReconnectFailThrottled("VL53L1X Reconnect SetInterruptPolarity Failed", (int8_t) Status);

		return false;
	}

	Status = VL53L1X_StartRanging(VL53L1X_DEVICE_ADDR);
	if (Status != 0)
	{
		SensorHealth_ReportError(SENSOR_ERROR_START_RANGING);
		SensorHealth_SetOffline();

		SystemHealth_RecordSensorReconnectFail();
		SystemHealth_RecordI2cError();

		SensorTask_LogReconnectFailThrottled("VL53L1X Reconnect StartRanging Failed", (int8_t) Status);

		return false;
	}

	SensorHealth_ReportSuccess();

	SensorTask_ResetReconnectLogThrottle();

	Debug_Print("[SensorTask] VL53L1X Reconnect Success\r\n");
	Debug_Process();

	return true;
}

/**
 * 	@brief	SensorTask runtime error handler
 *
 * 	@details
 * 	演算法 [Consecutive Error Counter Algorithm]
 *
 * 	Step 1: 回報 SensorHealth error
 * 	Step 2: 輸出 throttled error log
 * 	Step 3: 若連續錯誤數達到門檻，切換 Sensor state 為 OFFLINE
 *
 * 	@param Error	SensorHealth error type
 * 	@param LogId	SensorTask log throttle ID
 * 	@param Message	Error message。
 * 	@param Status	Error status。
 *
 * 	@return None
 */
static void SensorTask_HandleSensorError(SensorError_t Error, SensorErrorLogId_t LogId, const char *Message, int32_t Status)
{
	SensorHealth_ReportError(Error);

	switch (Error)
	{
	case SENSOR_ERROR_GET_DISTANCE:
		SystemHealth_RecordSensorReadFail();
		SystemHealth_RecordI2cError();
		break;

	case SENSOR_ERROR_CHECK_DATA_READY:
	case SENSOR_ERROR_CLEAR_INTERRUPT:
	case SENSOR_ERROR_BOOT_STATE:
	case SENSOR_ERROR_SENSOR_INIT:
	case SENSOR_ERROR_START_RANGING:
	default:
		SystemHealth_RecordI2cError();
		break;
	}

	SensorTask_LogErrorThrottled(LogId, Message, (int8_t) Status);

	if (SensorHealth_GetConsecutiveErrorCount() >= SENSOR_OFFLINE_ERROR_THRESHOLD)
	{
		SensorHealth_SetOffline();
	}
}

/* USER CODE END Application */

