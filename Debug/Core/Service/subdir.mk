################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Service/BuzzerService.c \
../Core/Service/CliServices.c \
../Core/Service/DebugService.c \
../Core/Service/QueueStatusService.c \
../Core/Service/RingBuffer.c \
../Core/Service/SensorHealthService.c \
../Core/Service/SystemStatusService.c \
../Core/Service/UartDma.c \
../Core/Service/UartStatusService.c \
../Core/Service/WarningConfigService.c 

OBJS += \
./Core/Service/BuzzerService.o \
./Core/Service/CliServices.o \
./Core/Service/DebugService.o \
./Core/Service/QueueStatusService.o \
./Core/Service/RingBuffer.o \
./Core/Service/SensorHealthService.o \
./Core/Service/SystemStatusService.o \
./Core/Service/UartDma.o \
./Core/Service/UartStatusService.o \
./Core/Service/WarningConfigService.o 

C_DEPS += \
./Core/Service/BuzzerService.d \
./Core/Service/CliServices.d \
./Core/Service/DebugService.d \
./Core/Service/QueueStatusService.d \
./Core/Service/RingBuffer.d \
./Core/Service/SensorHealthService.d \
./Core/Service/SystemStatusService.d \
./Core/Service/UartDma.d \
./Core/Service/UartStatusService.d \
./Core/Service/WarningConfigService.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Service/%.o Core/Service/%.su Core/Service/%.cyclo: ../Core/Service/%.c Core/Service/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_HAL_DRIVER -DSTM32H735xx -c -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/Service" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/VL53L1X ToF/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/CLIService" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Phase_G" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Service

clean-Core-2f-Service:
	-$(RM) ./Core/Service/BuzzerService.cyclo ./Core/Service/BuzzerService.d ./Core/Service/BuzzerService.o ./Core/Service/BuzzerService.su ./Core/Service/CliServices.cyclo ./Core/Service/CliServices.d ./Core/Service/CliServices.o ./Core/Service/CliServices.su ./Core/Service/DebugService.cyclo ./Core/Service/DebugService.d ./Core/Service/DebugService.o ./Core/Service/DebugService.su ./Core/Service/QueueStatusService.cyclo ./Core/Service/QueueStatusService.d ./Core/Service/QueueStatusService.o ./Core/Service/QueueStatusService.su ./Core/Service/RingBuffer.cyclo ./Core/Service/RingBuffer.d ./Core/Service/RingBuffer.o ./Core/Service/RingBuffer.su ./Core/Service/SensorHealthService.cyclo ./Core/Service/SensorHealthService.d ./Core/Service/SensorHealthService.o ./Core/Service/SensorHealthService.su ./Core/Service/SystemStatusService.cyclo ./Core/Service/SystemStatusService.d ./Core/Service/SystemStatusService.o ./Core/Service/SystemStatusService.su ./Core/Service/UartDma.cyclo ./Core/Service/UartDma.d ./Core/Service/UartDma.o ./Core/Service/UartDma.su ./Core/Service/UartStatusService.cyclo ./Core/Service/UartStatusService.d ./Core/Service/UartStatusService.o ./Core/Service/UartStatusService.su ./Core/Service/WarningConfigService.cyclo ./Core/Service/WarningConfigService.d ./Core/Service/WarningConfigService.o ./Core/Service/WarningConfigService.su

.PHONY: clean-Core-2f-Service

