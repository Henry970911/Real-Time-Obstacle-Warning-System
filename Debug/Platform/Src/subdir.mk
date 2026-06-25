################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Platform/Src/UartDma.c \
../Platform/Src/UartStatusService.c 

OBJS += \
./Platform/Src/UartDma.o \
./Platform/Src/UartStatusService.o 

C_DEPS += \
./Platform/Src/UartDma.d \
./Platform/Src/UartStatusService.d 


# Each subdirectory must supply rules for building sources it contributes
Platform/Src/%.o Platform/Src/%.su Platform/Src/%.cyclo: ../Platform/Src/%.c Platform/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_HAL_DRIVER -DSTM32H735xx -c -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/VL53L1X ToF/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/App/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/BSP/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Platform/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Service/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Platform-2f-Src

clean-Platform-2f-Src:
	-$(RM) ./Platform/Src/UartDma.cyclo ./Platform/Src/UartDma.d ./Platform/Src/UartDma.o ./Platform/Src/UartDma.su ./Platform/Src/UartStatusService.cyclo ./Platform/Src/UartStatusService.d ./Platform/Src/UartStatusService.o ./Platform/Src/UartStatusService.su

.PHONY: clean-Platform-2f-Src

