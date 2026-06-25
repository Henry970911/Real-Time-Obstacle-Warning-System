################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/CLIService/ControlCommandService.c \
../Core/CLIService/DebugControlService.c \
../Core/CLIService/TaskMonitorService.c 

OBJS += \
./Core/CLIService/ControlCommandService.o \
./Core/CLIService/DebugControlService.o \
./Core/CLIService/TaskMonitorService.o 

C_DEPS += \
./Core/CLIService/ControlCommandService.d \
./Core/CLIService/DebugControlService.d \
./Core/CLIService/TaskMonitorService.d 


# Each subdirectory must supply rules for building sources it contributes
Core/CLIService/%.o Core/CLIService/%.su Core/CLIService/%.cyclo: ../Core/CLIService/%.c Core/CLIService/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_HAL_DRIVER -DSTM32H735xx -c -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/Service" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/VL53L1X ToF/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/CLIService" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Phase_G" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-CLIService

clean-Core-2f-CLIService:
	-$(RM) ./Core/CLIService/ControlCommandService.cyclo ./Core/CLIService/ControlCommandService.d ./Core/CLIService/ControlCommandService.o ./Core/CLIService/ControlCommandService.su ./Core/CLIService/DebugControlService.cyclo ./Core/CLIService/DebugControlService.d ./Core/CLIService/DebugControlService.o ./Core/CLIService/DebugControlService.su ./Core/CLIService/TaskMonitorService.cyclo ./Core/CLIService/TaskMonitorService.d ./Core/CLIService/TaskMonitorService.o ./Core/CLIService/TaskMonitorService.su

.PHONY: clean-Core-2f-CLIService

