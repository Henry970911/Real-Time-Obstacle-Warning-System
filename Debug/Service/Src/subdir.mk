################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Service/Src/CliServices.c \
../Service/Src/ControlCommandService.c \
../Service/Src/DebugControlService.c \
../Service/Src/DebugService.c \
../Service/Src/QueueStatusService.c \
../Service/Src/RingBuffer.c \
../Service/Src/SystemHealthService.c \
../Service/Src/TaskMonitorService.c 

OBJS += \
./Service/Src/CliServices.o \
./Service/Src/ControlCommandService.o \
./Service/Src/DebugControlService.o \
./Service/Src/DebugService.o \
./Service/Src/QueueStatusService.o \
./Service/Src/RingBuffer.o \
./Service/Src/SystemHealthService.o \
./Service/Src/TaskMonitorService.o 

C_DEPS += \
./Service/Src/CliServices.d \
./Service/Src/ControlCommandService.d \
./Service/Src/DebugControlService.d \
./Service/Src/DebugService.d \
./Service/Src/QueueStatusService.d \
./Service/Src/RingBuffer.d \
./Service/Src/SystemHealthService.d \
./Service/Src/TaskMonitorService.d 


# Each subdirectory must supply rules for building sources it contributes
Service/Src/%.o Service/Src/%.su Service/Src/%.cyclo: ../Service/Src/%.c Service/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_HAL_DRIVER -DSTM32H735xx -c -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/VL53L1X ToF/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/App/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/BSP/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Platform/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Service/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Service-2f-Src

clean-Service-2f-Src:
	-$(RM) ./Service/Src/CliServices.cyclo ./Service/Src/CliServices.d ./Service/Src/CliServices.o ./Service/Src/CliServices.su ./Service/Src/ControlCommandService.cyclo ./Service/Src/ControlCommandService.d ./Service/Src/ControlCommandService.o ./Service/Src/ControlCommandService.su ./Service/Src/DebugControlService.cyclo ./Service/Src/DebugControlService.d ./Service/Src/DebugControlService.o ./Service/Src/DebugControlService.su ./Service/Src/DebugService.cyclo ./Service/Src/DebugService.d ./Service/Src/DebugService.o ./Service/Src/DebugService.su ./Service/Src/QueueStatusService.cyclo ./Service/Src/QueueStatusService.d ./Service/Src/QueueStatusService.o ./Service/Src/QueueStatusService.su ./Service/Src/RingBuffer.cyclo ./Service/Src/RingBuffer.d ./Service/Src/RingBuffer.o ./Service/Src/RingBuffer.su ./Service/Src/SystemHealthService.cyclo ./Service/Src/SystemHealthService.d ./Service/Src/SystemHealthService.o ./Service/Src/SystemHealthService.su ./Service/Src/TaskMonitorService.cyclo ./Service/Src/TaskMonitorService.d ./Service/Src/TaskMonitorService.o ./Service/Src/TaskMonitorService.su

.PHONY: clean-Service-2f-Src

