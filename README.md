# STM32H735G-DK RTOS Obstacle Distance Warning System

## Project Overview / 專案概述

This project implements an **RTOS-based embedded firmware system** on the **STM32H735G-DK** development board.

本專案主要目標是使用 **STM32H735G-DK** 開發一套基於 **FreeRTOS / CMSIS-RTOS v2** 的即時障礙物距離警示韌體系統。

系統透過 **VL53L1X Time-of-Flight distance sensor** 取得即時距離資料，並根據距離門檻判斷目前的警示等級，再透過 LED 與蜂鳴器輸出警示狀態。

此外，本專案也實作了 **UART DMA Debug System** 與 **CLI Runtime Diagnostics**，可在系統執行期間即時查詢 sensor、queue、UART、task stack、system health 與 error 狀態。

此專案重點不是單純讀取感測器數值，而是理解並實作 **MCU 韌體系統中的周邊整合、RTOS 任務分工、中斷事件處理、非阻塞 Debug、CLI 診斷介面、錯誤處理與系統穩定性設計**。

---

## Hardware Platform / 硬體平台

| Item | Description |
|---|---|
| MCU Board | STM32H735G-DK |
| MCU Family | STM32H7 Series |
| CPU Core | ARM Cortex-M7 |
| Sensor | VL53L1X Time-of-Flight Distance Sensor |
| Sensor Interface | I2C4 |
| Sensor Interrupt | GPIO EXTI DataReady |
| Sensor Reset Pin | XSHUT GPIO |
| Debug Interface | USART3 |
| UART TX | DMA-based non-blocking transmission |
| UART RX | DMA-based receive + ring buffer |
| Warning Output | GPIO LEDs |
| Audio Output | Active buzzer module |
| RTOS | FreeRTOS / CMSIS-RTOS v2 |
| Development IDE | STM32CubeIDE |
| Firmware Language | C |

---

## Current Implementation Status / 目前實作狀態

### Implemented

- VL53L1X distance measurement through I2C4.
- GPIO EXTI DataReady interrupt handoff to SensorTask.
- FreeRTOS SensorTask, WarningTask and CommunicationTask.
- UART TX DMA non-blocking debug output.
- UART RX DMA CLI input.
- CLI diagnostics for status, queue, uart, task, monitor and error.
- Sensor health tracking and reconnect flow.

### Intentionally Kept Limited

- `threshold` command is currently read-only.
- Task logic is still kept in CubeMX-generated `freertos.c`.
- Validation is mainly board-level manual testing.

---

## Key Features / 專案功能

- STM32H735G-DK firmware with FreeRTOS / CMSIS-RTOS v2
- VL53L1X ToF sensor integration through I2C4
- GPIO EXTI DataReady interrupt handling
- ISR-to-task event delivery through RTOS queue
- SensorTask / WarningTask / CommunicationTask architecture
- UART TX DMA non-blocking debug output
- UART RX DMA Receive-to-Idle CLI input
- RX ring buffer and debug ring buffer
- Four-level warning state with hysteresis
- LED and active buzzer output
- Sensor health monitoring and reconnect flow
- Queue, UART, task stack and error diagnostics

---

## Project Development Phases / 專案開發階段

本專案由基礎硬體 bring-up 開始，逐步擴充成具備 RTOS、UART DMA、CLI、error handling 與 system health monitoring 的完整 MCU firmware system。

```text
Phase A - STM32 Basic Bring-up
Phase B - VL53L1X Distance Sensor Integration
Phase C - Debug Layer and UART Output
Phase D - FreeRTOS Task / Queue Architecture
Phase E - Buzzer and WarningTask Integration
Phase F - UART DMA + CLI Debug System
Phase G - System Stabilization and Error Handling
Phase H - Runtime Monitoring and Health Snapshot
Phase I - Code Freeze / Cleanup / Documentation
```

---

## Phase A - STM32 Basic Bring-up / STM32 基礎建置

此階段主要目標是確認 STM32H735G-DK 可以正常啟動，並建立後續 firmware 開發的基礎環境。

完成內容包含：

- STM32CubeIDE project creation
- Clock configuration
- GPIO initialization
- UART basic debug output
- HAL initialization flow verification
- Basic boot message output
- Initial peripheral configuration

### Phase A Result

```text
MCU Reset
→ HAL Init
→ System Clock Init
→ GPIO Init
→ USART Init
→ Boot log output
```

### Phase A Debug Notes

開發初期曾遇到 UART 輸出異常問題，後續確認與 STM32H735 Cortex-M7 的 D-Cache / DMA coherency 有關。

暫時驗證方式：

```text
Disable D-Cache
→ UART output becomes normal
→ Confirms DMA/cache coherency relationship
```

此問題也成為後續 UART DMA Debug System 設計時的重要除錯經驗。

---

## Phase B - VL53L1X Sensor Integration / VL53L1X 距離感測器整合

此階段主要目標是整合 **VL53L1X Time-of-Flight distance sensor**，並透過 I2C4 讀取距離資料。

完成內容包含：

- VL53L1X API integration
- VL53L1 platform layer porting
- STM32 HAL I2C read / write implementation
- Sensor boot and initialization
- Distance mode configuration
- Timing budget configuration
- Start ranging
- Polling-based distance read verification
- XSHUT hardware reset support

### VL53L1X Platform Layer

VL53L1X driver 需要由 platform layer 提供底層 I2C 操作。

主要包含：

- `VL53L1_WriteMulti`
- `VL53L1_ReadMulti`
- `VL53L1_WrByte`
- `VL53L1_WrWord`
- `VL53L1_WrDWord`
- `VL53L1_RdByte`
- `VL53L1_RdWord`
- `VL53L1_RdDWord`
- `VL53L1_WaitMs`
- `VL53L1_HardReset`

### Phase B Sensor Flow

```text
VL53L1_HardReset
→ VL53L1X Boot Check
→ Sensor Init
→ Set Distance Mode
→ Set Timing Budget
→ Start Ranging
→ Read Distance
→ Print Distance Result
```

### Phase B Result

完成 VL53L1X I2C 通訊與距離讀取，確認 sensor 可以透過 STM32H735G-DK 正常取得距離資料。

---

## Phase C - Debug Layer / 除錯輸出層

此階段主要目標是建立可重複使用的 DebugService，避免 application code 直接依賴 blocking UART output。

完成內容包含：

- DebugService module
- Debug_Print interface
- UART output abstraction
- Debug message formatting
- UART output path verification
- Debug output used by multiple modules

### Debug Layer Purpose

```text
Application / Service modules
        ↓
Debug_Print()
        ↓
DebugService
        ↓
UART output
        ↓
Terminal
```

### Design Purpose

- 統一 debug output interface
- 避免各模組直接操作 UART
- 提高後續改成 UART DMA TX 的彈性
- 讓 SensorTask / WarningTask / CommunicationTask 都能共用同一套 debug output

---

## Phase D - FreeRTOS Task / Queue Architecture / RTOS 任務與佇列架構

此階段主要目標是將原本單一流程式邏輯拆分成 RTOS task-based architecture。

完成內容包含：

- FreeRTOS / CMSIS-RTOS v2 enable
- SensorTask creation
- WarningTask creation
- SensorEventQueue creation
- DistanceQueue creation
- ISR-to-task event handoff
- SensorTask-to-WarningTask distance transfer
- Task-based firmware architecture

### RTOS Task Design

```text
SensorTask
└── Responsible for VL53L1X distance acquisition

WarningTask
└── Responsible for warning level decision and output control

CommunicationTask
└── Responsible for UART DMA, CLI command handling, and debug processing
```

### Queue Design

| Queue | Direction | Purpose |
|---|---|---|
| SensorEventQueue | ISR → SensorTask | Notify sensor DataReady event |
| DistanceQueue | SensorTask → WarningTask | Transfer valid distance data |
| SensorControlQueue | CLI / Service → SensorTask | Sensor control or recovery event |
| BuzzerControlQueue | WarningTask / CLI → Buzzer logic | Buzzer control command |

### Main RTOS Data Flow

```text
GPIO EXTI ISR
        ↓
SensorEventQueue
        ↓
SensorTask
        ↓
DistanceQueue
        ↓
WarningTask
        ↓
LED / Buzzer
```

### Phase D Design Principle

ISR 中不直接執行 I2C transaction。

```text
EXTI ISR
→ only send event to queue
→ SensorTask handles I2C read in task context
```

此設計可避免在 interrupt context 中執行可能 blocking 或 timeout 的 HAL I2C API。

---

## Phase E - Buzzer and WarningTask Integration / 蜂鳴器與警示任務整合

此階段主要目標是將距離資料轉換成具體警示輸出。

完成內容包含：

- Buzzer_service implementation
- Active buzzer GPIO output
- WarningTask distance threshold judgment
- Four-level warning state
- LED output integration
- Buzzer output integration
- Hysteresis logic
- Warning level transition verification

### Warning Levels

| Level | Description | Output Behavior |
|---|---|---|
| SAFE | Object is far away | LED / buzzer off or safe indication |
| CAUTION | Object is approaching | Low-level warning |
| WARNING | Object is close | Medium-level warning |
| DANGER | Object is very close | High-level warning |

### WarningTask Flow

```text
Receive distance from DistanceQueue
→ Compare with threshold
→ Apply hysteresis
→ Update warning level
→ Update LED output
→ Update buzzer output
```

### Hysteresis Purpose

距離值在門檻附近可能產生抖動，如果沒有 hysteresis，warning level 可能會頻繁切換。

```text
Without hysteresis:
Distance near threshold
→ Warning level toggles frequently
→ LED / buzzer output becomes unstable

With hysteresis:
Separate enter and exit threshold
→ Warning level becomes stable
→ Output behavior becomes smoother
```

---

## Phase F - UART DMA + CLI Debug System / UART DMA 與 CLI 除錯系統

此階段主要目標是建立可互動的 runtime debug system，使系統在執行期間能透過 terminal 輸入指令並查詢內部狀態。

完成內容包含：

- USART3 TX DMA
- USART3 RX DMA
- UART TX busy flag
- UART TX complete callback
- UART RX callback
- RX ring buffer
- Debug ring buffer
- CLI parser
- CLI command handler
- CommunicationTask
- Non-blocking debug output
- Runtime diagnostics command set

### UART TX DMA Debug Flow

```text
Debug_Print()
    ↓
Debug Ring Buffer
    ↓
Debug_Process()
    ↓
UART_DMA_Send()
    ↓
HAL_UART_TxCpltCallback()
    ↓
Clear UART TX busy flag
```

### UART RX DMA CLI Flow

```text
User input from terminal
    ↓
UART RX DMA
    ↓
HAL_UARTEx_RxEventCallback()
    ↓
Copy received bytes
    ↓
RX Ring Buffer
    ↓
CommunicationTask
    ↓
CLI Parser
    ↓
Command Handler
    ↓
DebugService response
```

### CLI Commands

| Command | Purpose |
|---|---|
| `help` | Show available commands |
| `status` | Show basic system status |
| `sensor` | Show sensor status and last distance |
| `queue` | Show RTOS queue usage |
| `uart` | Show UART RX/TX status |
| `threshold` / `threshold show` | Show current warning thresholds |
| `buzzer` | Test or configure buzzer behavior |
| `log` | Configure debug log behavior |
| `task` | Show task stack usage |
| `monitor` | Show integrated system health snapshot |
| `error` | Show error counters and failure information |

### Phase F Result

系統可以透過 UART terminal 即時輸入 CLI command，並在不阻塞 SensorTask 的情況下回傳診斷資訊。

---

## Phase G - System Stabilization and Error Handling / 系統穩定性與錯誤處理

此階段主要目標是讓系統在 sensor 異常、I2C error、queue 壓力、UART 壓力等情況下仍能維持可觀察與可恢復。

完成內容包含：

- SensorHealthService
- SystemHealthService
- I2C error count tracking
- Retry count tracking
- Consecutive error count tracking
- Last data tick tracking
- Last retry tick tracking
- Sensor reconnect mechanism
- I2C error log throttling
- Reconnect log throttling
- Sensor disconnect test
- Sensor reconnect test
- UART / CLI stress test
- Queue stress test

### Sensor Error Handling Flow

```text
Sensor read failure
    ↓
Update I2C error count
    ↓
Update consecutive error count
    ↓
Update sensor health state
    ↓
Check retry / reconnect condition
    ↓
Attempt recovery
    ↓
Resume measurement if sensor is available
```

### Sensor Disconnect / Reconnect Behavior

```text
System running normally
→ Sensor disconnected
→ I2C read fails
→ Error counter increases
→ Sensor state changes
→ Reconnect mechanism attempts recovery
→ Sensor reconnected
→ System resumes normal ranging
```

### Log Throttling

為避免 sensor 拔除時大量 I2C error log 佔滿 UART output，本專案加入 log throttling。

```text
Repeated I2C errors
→ Print error log with interval limit
→ Avoid flooding UART output
→ Keep CLI responsive
```

---

## Phase H - Runtime Monitoring and Health Snapshot / 執行期監控與健康狀態快照

此階段主要目標是讓系統內部狀態可以透過 CLI 被觀察。

完成內容包含：

- QueueStatusService
- UartStatusService
- TaskMonitorService
- SystemStatusService
- SystemHealthService
- SensorHealthService
- CLI monitor command
- CLI task command
- CLI queue command
- CLI uart command
- CLI sensor command
- Integrated health snapshot

### Monitored Items

| Category | Monitored Data |
|---|---|
| Sensor | State, last distance, error count, retry count |
| Queue | Used count, remaining count, capacity |
| UART | RX callback count, overflow count, last RX size |
| Task | Stack high water mark |
| System | Health snapshot, runtime state |
| Error | I2C error, sensor error, reconnect status |

### Monitor Flow

```text
User executes monitor command
→ CommunicationTask parses command
→ Command handler collects service data
→ SystemHealthService builds snapshot
→ DebugService prints result
→ UART TX DMA outputs response
```

---

## Phase I - Code Freeze / Cleanup / Documentation / 程式整理與文件化

此階段主要目標是將已完成的功能整理成可作為 GitHub portfolio 的狀態。

完成內容包含：

- Code freeze checklist
- File structure review
- Service module review
- Header dependency review
- Magic number review
- Naming consistency review
- Chinese / English documentation improvement
- README preparation
- Project architecture summary
- Interview-oriented project explanation

### Phase I Notes

此階段重點不是新增大型功能，而是讓專案進入可展示、可維護、可說明的狀態。

```text
Core function complete
→ Stabilization verified
→ Runtime diagnostics available
→ Documentation prepared
→ Ready for GitHub / resume / interview presentation
```

---

## Firmware Architecture / 韌體架構

本專案採用分層設計，將硬體驅動、服務模組、RTOS 任務與應用邏輯分離。

```text
Application Layer
└── Obstacle Distance Warning Application
    ├── Distance threshold judgment
    ├── Warning level decision
    ├── LED warning output
    ├── Buzzer warning output
    └── Runtime status reporting

RTOS Task Layer
├── SensorTask
│   ├── Wait for DataReady event
│   ├── Check VL53L1X data ready state
│   ├── Read distance through I2C
│   ├── Clear VL53L1X interrupt
│   ├── Update sensor health status
│   └── Send distance data to DistanceQueue
│
├── WarningTask
│   ├── Receive distance data
│   ├── Apply threshold judgment
│   ├── Apply hysteresis logic
│   ├── Update warning level
│   ├── Control LED output
│   └── Control buzzer output
│
└── CommunicationTask
    ├── Process UART RX DMA data
    ├── Store input into RX ring buffer
    ├── Parse CLI commands
    ├── Execute command handlers
    ├── Process debug TX ring buffer
    └── Output diagnostics through UART TX DMA

Service Layer
├── DebugService
├── UartDma
├── CliServices
├── UartStatusService
├── QueueStatusService
├── SystemStatusService
├── SensorHealthService
├── SystemHealthService
├── WarningConfigService
├── TaskMonitorService
└── BuzzerService

Driver / Platform Layer
├── VL53L1X API
├── VL53L1 Platform Layer
├── STM32 HAL I2C
├── STM32 HAL UART
├── STM32 HAL DMA
├── STM32 HAL GPIO
└── STM32 HAL EXTI

Hardware Layer
├── STM32H735G-DK
├── VL53L1X ToF Sensor
├── LEDs
├── Active Buzzer
└── UART Terminal
```

### 設計重點

RTOS Task Layer 負責系統主要行為：

- `SensorTask` 負責感測器事件與距離讀取
- `WarningTask` 負責警示狀態判斷與輸出控制
- `CommunicationTask` 負責 UART DMA、CLI 與 Debug 輸出

Service Layer 則將共用功能模組化，例如：

```text
Debug Output
UART DMA Control
CLI Command Handling
Sensor Health Tracking
Queue Status Tracking
Task Stack Monitoring
System Health Snapshot
Warning Threshold Configuration
Buzzer Control
```

這樣可以避免 Application Layer 直接操作底層 HAL 或全域狀態，使程式架構更清楚，也更接近實務 Firmware System 的設計方式。

---

## Main System Flow / 主要系統流程

### System Boot Flow

```text
MCU Reset
→ HAL_Init
→ SystemClock_Config
→ GPIO Init
→ DMA Init
→ I2C4 Init
→ USART3 Init
→ FreeRTOS Kernel Init
→ Create Queues
→ Create Tasks
→ Start RTOS Scheduler
```

### Sensor Initialization Flow

```text
VL53L1_HardReset
→ VL53L1X Boot Check
→ VL53L1X Sensor Init
→ Configure Distance Mode
→ Configure Timing Budget
→ Start Ranging
→ Enable DataReady EXTI
```

### Runtime Distance Measurement Flow

```text
VL53L1X DataReady
→ GPIO EXTI Callback
→ Send event to SensorEventQueue
→ SensorTask receives event
→ CheckForDataReady
→ GetDistance
→ ClearInterrupt
→ Send distance to DistanceQueue
→ WarningTask receives distance
→ Update warning level
→ Control LED and buzzer
```

### UART CLI Runtime Flow

```text
User input from terminal
→ USART3 RX DMA
→ RX callback
→ RX ring buffer
→ CommunicationTask
→ CLI parser
→ Command handler
→ DebugService
→ UART TX DMA
→ Terminal output
```

---

## Interrupt Handling / 中斷處理

VL53L1X 的 DataReady 腳位連接到 STM32 GPIO EXTI。

本專案的中斷設計原則：

```text
Do not perform I2C transaction inside ISR.
```

實際設計如下：

```text
GPIO EXTI ISR
    ↓
HAL_GPIO_EXTI_Callback()
    ↓
Check VL53L1X_INT_Pin
    ↓
osMessageQueuePut(SensorEventQueue)
    ↓
Return from ISR
```

SensorTask 之後在 task context 中執行 I2C 操作：

```text
SensorTask
    ↓
Receive SensorEventQueue
    ↓
VL53L1X_CheckForDataReady
    ↓
VL53L1X_GetDistance
    ↓
VL53L1X_ClearInterrupt
```

### 為什麼 ISR 不直接讀取 I2C？

| Reason | Description |
|---|---|
| I2C operation may block | HAL I2C API may wait for hardware flag or timeout |
| ISR should be short | Long ISR affects system interrupt latency |
| RTOS API has ISR-safe rules | ISR should use proper queue/event API |
| Sensor read belongs to task context | Easier to debug and recover from error |

---

## UART DMA Debug System / UART DMA 除錯系統

本專案使用 UART DMA 建立非阻塞 Debug 輸出機制。

傳統 `printf()` 可能造成 task blocking，尤其在 sensor task 或 warning task 中大量輸出 log 時，可能影響即時性。

本專案改用：

```text
Debug_Print
→ Debug ring buffer
→ Debug_Process
→ UART TX DMA
→ DMA complete callback
```

### UART TX Flow

```text
Debug_Print()
    ↓
Write message into debug ring buffer
    ↓
Debug_Process()
    ↓
Check UART TX busy flag
    ↓
Start UART TX DMA transfer
    ↓
HAL_UART_TxCpltCallback()
    ↓
Clear UART TX busy flag
```

### UART RX Flow

```text
USART3 RX DMA
    ↓
HAL_UARTEx_RxEventCallback()
    ↓
Copy received data
    ↓
Push bytes into RX ring buffer
    ↓
CommunicationTask reads command line
    ↓
CLI parser handles command
```

### UART DMA Design Purpose

- Avoid blocking UART output
- Keep SensorTask responsive
- Keep WarningTask responsive
- Allow CLI command input during runtime
- Decouple debug producer and UART transmitter
- Support runtime diagnostics

---

## CLI Command System / CLI 指令系統

本專案提供 UART-based CLI，可在系統執行期間查詢與控制內部狀態。

### Supported CLI Commands

| Command | Purpose |
|---|---|
| `help` | Show command list |
| `status` | Show basic system status |
| `sensor` | Show sensor state, distance, error count |
| `queue` | Show RTOS queue usage |
| `uart` | Show UART RX/TX statistics |
| `threshold` / `threshold show` | Show current warning thresholds |
| `buzzer` | Test or configure buzzer behavior |
| `log` | Configure debug log behavior |
| `task` | Show task stack high water mark |
| `monitor` | Show integrated system health snapshot |
| `error` | Show error counters and failure status |

### CLI Handling Flow

```text
Terminal input
→ UART RX DMA
→ RX ring buffer
→ CommunicationTask
→ CLI parser
→ Command handler
→ Service module query
→ Debug output
→ UART TX DMA
→ Terminal response
```

### CLI 設計目的

CLI 的目的不是單純輸入指令，而是讓 firmware system 具備 runtime observability。

透過 CLI 可以在不重新燒錄 firmware 的情況下查看：

```text
Sensor 狀態
Queue 使用量
UART callback 統計
Task stack 剩餘量
Warning threshold
Buzzer 狀態
System health snapshot
Error counters
```

---

## Warning Logic / 警示邏輯

系統根據 VL53L1X 讀取到的距離值進行四級警示判斷。

```text
Distance input
    ↓
Threshold comparison
    ↓
Hysteresis check
    ↓
Warning level update
    ↓
LED / buzzer output
```

### Warning Levels

| Warning Level | Meaning | Output |
|---|---|---|
| SAFE | Distance is safe | No warning or safe LED |
| CAUTION | Object is approaching | Low-level warning |
| WARNING | Object is close | Medium-level warning |
| DANGER | Object is too close | High-level warning |

### Hysteresis Logic

Hysteresis 用來避免距離值在門檻附近跳動時造成 warning level 頻繁切換。

```text
Enter threshold
→ change to higher warning level

Exit threshold
→ return to lower warning level only after distance is sufficiently safe
```

### Example

```text
Without hysteresis:
490 mm ↔ 510 mm
→ WARNING ↔ CAUTION toggles repeatedly

With hysteresis:
Enter WARNING below 500 mm
Exit WARNING above 550 mm
→ Warning level remains stable
```

---

## Sensor Health Monitoring / 感測器健康狀態監控

本專案透過 SensorHealthService 記錄 sensor 狀態與錯誤資訊。

### Tracked Sensor Data

| Item | Source |
|---|---|
| Sensor state | SensorHealthService |
| Last error | SensorHealthService |
| Error count | SensorHealthService |
| Retry count | SensorHealthService |
| Consecutive error count | SensorHealthService |
| Last success tick | SensorHealthService |
| Last error tick | SensorHealthService |
| Last distance | SystemStatusService |
| Reconnect counters | SystemHealthService |

### Sensor Health Flow

```text
Sensor read success
→ Update last distance
→ Update last data tick
→ Clear consecutive error count
→ Mark sensor as healthy

Sensor read failure
→ Increase I2C error count
→ Increase consecutive error count
→ Update sensor health state
→ Trigger retry / reconnect logic if needed
```

---

## Sensor Reconnect Mechanism / 感測器重新連線機制

當 sensor 被拔除或 I2C 通訊異常時，系統不應直接卡死，而是需要記錄錯誤並嘗試恢復。

### Reconnect Flow

```text
I2C read failure
    ↓
Update error counter
    ↓
Check retry interval
    ↓
Attempt VL53L1_HardReset
    ↓
Re-initialize VL53L1X
    ↓
Restart ranging
    ↓
Resume normal operation if successful
```

### Sensor Disconnect Test Flow

```text
System running normally
→ Disconnect VL53L1X sensor
→ I2C read error occurs
→ SensorHealthService updates error state
→ Reconnect mechanism starts retry
→ CLI remains responsive
→ Reconnect sensor
→ System resumes measurement
```

---

## Queue and Runtime Monitoring / Queue 與執行期監控

本專案提供 QueueStatusService，用來觀察 FreeRTOS queue 的使用狀態。

### Queue Monitoring Items

| Queue | Monitored Data |
|---|---|
| SensorEventQueue | Used count, remaining count, capacity |
| DistanceQueue | Used count, remaining count, capacity |
| SensorControlQueue | Used count, remaining count, capacity |
| BuzzerControlQueue | Used count, remaining count, capacity |

### Queue Monitoring Purpose

```text
Check queue usage
→ Confirm event flow is normal
→ Detect possible queue full condition
→ Verify ISR-to-task communication
→ Verify task-to-task data transfer
```

---

## Task Stack Monitoring / Task Stack 監控

本專案透過 TaskMonitorService 記錄各 RTOS task 的 stack high water mark。

### Monitored Tasks

| Task | Purpose |
|---|---|
| SensorTask | Sensor acquisition and error handling |
| WarningTask | Warning logic and output control |
| CommunicationTask | UART DMA, CLI, debug output |

### Stack Monitoring Purpose

```text
Read stack high water mark
→ Check remaining stack margin
→ Detect stack size risk
→ Adjust task stack size if needed
```

---

## UART Status Monitoring / UART 狀態監控

本專案透過 UartStatusService 記錄 UART RX/TX 狀態。

### UART Status Monitoring

Currently monitored through CLI:

- RX DMA callback count
- Last RX size
- RX ring buffer overflow count

UART TX DMA busy state is handled internally by UartDma.

---

## Validation Tests / 驗證測試

本專案包含多種功能驗證與壓力測試。

| Test Item | Purpose |
|---|---|
| UART Boot Log Test | Verify boot message and UART output |
| VL53L1X I2C Communication Test | Verify sensor I2C communication |
| Sensor Distance Read Test | Verify distance acquisition |
| DataReady EXTI Test | Verify GPIO interrupt event |
| SensorEventQueue Test | Verify ISR-to-task event transfer |
| DistanceQueue Test | Verify SensorTask-to-WarningTask data transfer |
| Warning Level Test | Verify threshold and warning state |
| Hysteresis Test | Verify stable warning transition |
| LED Output Test | Verify GPIO LED warning output |
| Buzzer Output Test | Verify active buzzer control |
| UART TX DMA Test | Verify non-blocking debug output |
| UART RX DMA Test | Verify command input reception |
| RX Ring Buffer Test | Verify input buffering |
| CLI Command Test | Verify parser and command handler |
| Queue Status Test | Verify queue monitoring output |
| UART Status Test | Verify UART statistics |
| Task Monitor Test | Verify stack high water mark reporting |
| Sensor Disconnect Test | Verify error handling |
| Sensor Reconnect Test | Verify recovery mechanism |
| UART / CLI Stress Test | Verify runtime command stability |
| Queue Stress Test | Verify queue behavior under load |
| Long-run Stability Test | Verify system behavior over time |

---

## Test Flow Summary / 測試流程摘要

### Basic Boot Test

```text
Power on board
→ MCU reset
→ HAL init
→ Peripheral init
→ RTOS scheduler starts
→ Boot log appears on UART terminal
```

### Sensor Measurement Test

```text
Power on board
→ Initialize VL53L1X
→ Start ranging
→ Trigger DataReady interrupt
→ SensorTask reads distance
→ WarningTask receives distance
→ LED / buzzer output changes
→ Terminal shows sensor status
```

### UART DMA CLI Test

```text
Open UART terminal
→ Input CLI command
→ UART RX DMA receives data
→ RX ring buffer stores bytes
→ CLI parser detects command
→ Command handler executes
→ DebugService prints response
→ UART TX DMA sends response
```

### Warning Logic Test

```text
Move object closer to sensor
→ Distance decreases
→ Warning level changes
→ LED state changes
→ Buzzer pattern changes

Move object away from sensor
→ Distance increases
→ Hysteresis condition satisfied
→ Warning level returns to lower state
```

### Sensor Disconnect / Reconnect Test

```text
System running normally
→ Disconnect sensor
→ I2C read fails
→ Error counter increases
→ Sensor health state changes
→ Reconnect logic attempts recovery
→ Reconnect sensor
→ System resumes normal measurement
```

### Queue Monitoring Test

```text
Run system
→ Execute queue command
→ Read queue usage
→ Verify SensorEventQueue status
→ Verify DistanceQueue status
→ Confirm no queue overflow
```

### Task Stack Monitoring Test

```text
Run system
→ Execute task or monitor command
→ Read stack high water mark
→ Verify each task has enough stack margin
```

---

## Repository Structure / 專案結構

```text
.
├── Core
│   ├── Inc
│   └── Src
│       ├── main.c
│       ├── freertos.c
│       ├── gpio.c
│       ├── i2c.c
│       ├── usart.c
│       ├── dma.c
│       └── stm32h7xx_it.c
│
├── Service
│   ├── DebugService
│   ├── UartDma
│   ├── CliServices
│   ├── UartStatusService
│   ├── QueueStatusService
│   ├── SystemStatusService
│   ├── SensorHealthService
│   ├── SystemHealthService
│   ├── WarningConfigService
│   ├── TaskMonitorService
│   └── BuzzerService
│
├── VL53L1X
│   ├── Platform
│   └── Driver
│
├── Drivers
│   ├── CMSIS
│   └── STM32H7xx_HAL_Driver
│
├── Middlewares
│   └── FreeRTOS
│
├── STM32H735IGKX_FLASH.ld
├── STM32H735IGKX_RAM.ld
└── README.md
```
---

## Key Firmware Concepts / 對應的韌體觀念

本專案涵蓋以下 Firmware / Embedded System 觀念：

```text
MCU Firmware Development
STM32 HAL Peripheral Configuration
FreeRTOS Task Design
CMSIS-RTOS v2 API Usage
RTOS Queue Communication
ISR-to-Task Event Handling
I2C Sensor Integration
GPIO EXTI Interrupt
UART DMA TX/RX
Ring Buffer Design
CLI Parser Design
Non-blocking Debug Output
Runtime Diagnostics
Sensor Health Monitoring
Error Handling
Reconnect Mechanism
Task Stack Monitoring
Queue Status Monitoring
Hardware Bring-up
Embedded System Debugging
```

這些內容對應到實務 Firmware 開發中常見的周邊整合、RTOS 架構、非阻塞通訊、錯誤處理與系統觀察能力。

---

## Problems Solved / 問題與處理方向

| Problem | Cause | Solution |
|---|---|---|
| UART output was abnormal | DMA and Cortex-M7 D-Cache coherency issue | Disabled D-Cache during verification and identified cache/DMA relationship |
| Direct debug output could block system behavior | Blocking UART output may affect SensorTask timing | Implemented DebugService with ring buffer and UART TX DMA |
| I2C operation should not run inside ISR | HAL I2C may block or timeout | ISR only sends event to queue; SensorTask performs I2C operation |
| Sensor disconnect caused repeated I2C errors | VL53L1X unavailable or I2C timeout | Added SensorHealthService, retry count, and reconnect handling |
| Error logs could flood UART output | Repeated I2C failures generated too many logs | Added error log throttling |
| CLI response could be delayed under error condition | Sensor error flow and debug output competed for runtime handling | Separated CommunicationTask and improved UART status tracking |
| Queue status was hard to observe | RTOS queue state is not visible externally | Added QueueStatusService and CLI queue command |
| Task stack usage was unknown | RTOS task stack margin must be monitored | Added TaskMonitorService and task high water mark reporting |
| Warning level could oscillate near threshold | Distance value fluctuates near threshold boundary | Added hysteresis-based warning level control |

---

## Debugging Experience / 除錯經驗

### UART DMA and D-Cache Issue

在 STM32H735 Cortex-M7 上，開發初期曾遇到 UART 輸出異常問題。

後續確認此問題與 D-Cache / DMA coherency 有關。

驗證方式：

```text
Disable D-Cache
→ UART output becomes normal.
→ Confirms DMA/cache coherency relationship.
→ In the current UART DMA path, TX buffers are cleaned before DMA transmission and RX DMA buffers are invalidated before CPU reads. 
```

後續可改善方向：

```text
Use cache clean / invalidate operation
or place DMA buffers in non-cacheable memory region.
```

---

### ISR Design Correction

初期 sensor event handling 經過修正，最終設計避免在 ISR 中執行 I2C operation。

最終設計：

```text
GPIO EXTI ISR
→ Send event to SensorEventQueue only
→ SensorTask handles I2C read
```

此設計符合 RTOS firmware 中 ISR 應保持短小、避免 blocking operation 的原則。

---

### Sensor Disconnect Handling

當 VL53L1X sensor 被拔除時，系統會遇到 I2C timeout 或 read failure。

後續加入：

- I2C error count
- Consecutive error count
- Retry count
- Last retry tick
- Sensor health state
- Reconnect mechanism
- Error log throttling

最終行為：

```text
Sensor unavailable
→ I2C error detected
→ Health state updated
→ Retry / reconnect flow executed
→ CLI remains responsive
→ Sensor reconnects successfully
```

---

### CLI Runtime Debugging

透過 CLI 指令，可以在 firmware 執行期間即時觀察系統狀態。

常用 debug commands：

```text
sensor
queue
uart
task
monitor
error
```

這使系統不需要重新燒錄 firmware，也能直接從 terminal 觀察內部狀態。

---

## Known Limitations / 已知限制

- Hardware validation is currently manual and board-based.
- `threshold` CLI command is read-only.
- Task logic is still located in CubeMX-managed `freertos.c`.
- Generated build output such as `Debug/` should be excluded from the repository.
- Future work could move DMA buffers to a non-cacheable memory region.

---

## What I Learned / 學習重點

Through this project, I learned and practiced:

- STM32H735G-DK firmware development
- STM32CubeIDE project configuration
- STM32 HAL GPIO / I2C / UART / DMA usage
- FreeRTOS task and queue design
- CMSIS-RTOS v2 API usage
- Interrupt-driven firmware design
- ISR-to-task event handoff
- I2C sensor driver integration
- VL53L1X platform layer porting
- XSHUT hardware reset control
- UART DMA TX/RX implementation
- Ring buffer implementation
- CLI command parser design
- Non-blocking debug system design
- Runtime diagnostics design
- Sensor error handling
- Sensor reconnect mechanism
- Task stack high water mark monitoring
- Queue status monitoring
- UART status monitoring
- Cortex-M7 D-Cache and DMA coherency issue
- Embedded system debugging and validation process
- Firmware architecture documentation

---

## Project Purpose / 專案用途

This project is intended for firmware learning, RTOS practice, MCU peripheral integration, and embedded system portfolio demonstration.

本專案主要作為 Firmware / Embedded System 轉職作品集，展示以下能力：

```text
Embedded C
STM32 MCU Firmware
RTOS-based Firmware Architecture
FreeRTOS Task / Queue Design
I2C Sensor Integration
GPIO EXTI Interrupt Handling
UART DMA Communication
Ring Buffer Design
CLI Debug System
Runtime Diagnostics
Error Handling
Sensor Reconnect Mechanism
System Health Monitoring
Embedded Debugging
```

---


## Interview Talking Points / 面試可說明重點

This project can be introduced in an interview as follows:

```text
I developed an RTOS-based embedded firmware project on STM32H735G-DK.
The system integrates a VL53L1X ToF distance sensor through I2C,
uses GPIO EXTI interrupt for DataReady event handling, and uses
FreeRTOS queues to transfer events and distance data between tasks.

I also implemented a UART DMA-based non-blocking debug interface,
RX ring buffer, CLI command parser, runtime diagnostics, sensor health
monitoring, and reconnect handling.

The project focuses not only on reading sensor data, but also on
firmware architecture, runtime observability, error handling,
and system robustness.
```

中文版本：

```text
我完成了一個基於 STM32H735G-DK 的 RTOS 韌體專題。
系統透過 I2C 整合 VL53L1X ToF 距離感測器，並使用 GPIO EXTI
處理 DataReady 中斷，再透過 FreeRTOS Queue 將事件與距離資料
傳遞給不同 Task。

另外我也實作 UART DMA 非阻塞除錯輸出、RX Ring Buffer、CLI 指令解析、
runtime diagnostics、sensor health monitoring 與 reconnect 機制。

這個專題不只是讀取感測器，而是完整練習 MCU 周邊整合、RTOS 架構、
錯誤處理與韌體系統可觀察性設計。
```

---

## Future Improvements / 後續改善方向

Possible future improvements include:

- Refactor header file dependencies
- Improve service folder organization
- Add formal state machine diagram
- Add unit-testable service logic
- Add structured error code definition
- Add cache-safe DMA buffer design
- Use MPU or non-cacheable SRAM region for DMA buffers
- Add more complete fault injection tests
- Add long-run stability report
- Add README images and architecture diagrams
- Add Git commit history by development phase
- Add coding style and module interface documentation
- Add automated static analysis if applicable

---

## Keywords

`STM32` `STM32H735G-DK` `STM32H7` `Firmware` `Embedded C` `FreeRTOS` `CMSIS-RTOS v2` `RTOS` `I2C` `UART DMA` `GPIO EXTI` `VL53L1X` `ToF Sensor` `Ring Buffer` `CLI` `Debug System` `Queue` `Task` `Sensor Health` `Reconnect` `Runtime Diagnostics` `Embedded System`
