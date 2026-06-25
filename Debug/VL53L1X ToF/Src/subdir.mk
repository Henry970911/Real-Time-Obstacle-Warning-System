################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../VL53L1X\ ToF/Src/VL53L1X_api.c \
../VL53L1X\ ToF/Src/VL53L1X_calibration.c \
../VL53L1X\ ToF/Src/VL53L1_platform.c 

OBJS += \
./VL53L1X\ ToF/Src/VL53L1X_api.o \
./VL53L1X\ ToF/Src/VL53L1X_calibration.o \
./VL53L1X\ ToF/Src/VL53L1_platform.o 

C_DEPS += \
./VL53L1X\ ToF/Src/VL53L1X_api.d \
./VL53L1X\ ToF/Src/VL53L1X_calibration.d \
./VL53L1X\ ToF/Src/VL53L1_platform.d 


# Each subdirectory must supply rules for building sources it contributes
VL53L1X\ ToF/Src/VL53L1X_api.o: ../VL53L1X\ ToF/Src/VL53L1X_api.c VL53L1X\ ToF/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_HAL_DRIVER -DSTM32H735xx -c -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/VL53L1X ToF/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/App/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/BSP/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Platform/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Service/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"VL53L1X ToF/Src/VL53L1X_api.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
VL53L1X\ ToF/Src/VL53L1X_calibration.o: ../VL53L1X\ ToF/Src/VL53L1X_calibration.c VL53L1X\ ToF/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_HAL_DRIVER -DSTM32H735xx -c -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/VL53L1X ToF/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/App/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/BSP/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Platform/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Service/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"VL53L1X ToF/Src/VL53L1X_calibration.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
VL53L1X\ ToF/Src/VL53L1_platform.o: ../VL53L1X\ ToF/Src/VL53L1_platform.c VL53L1X\ ToF/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_HAL_DRIVER -DSTM32H735xx -c -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/VL53L1X ToF/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Core/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/App/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/BSP/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Platform/Inc" -I"C:/Users/CPC/STM32CubeIDE/workspace 1.20.0/STM32H735_Real-Time Warning System/Service/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"VL53L1X ToF/Src/VL53L1_platform.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-VL53L1X-20-ToF-2f-Src

clean-VL53L1X-20-ToF-2f-Src:
	-$(RM) ./VL53L1X\ ToF/Src/VL53L1X_api.cyclo ./VL53L1X\ ToF/Src/VL53L1X_api.d ./VL53L1X\ ToF/Src/VL53L1X_api.o ./VL53L1X\ ToF/Src/VL53L1X_api.su ./VL53L1X\ ToF/Src/VL53L1X_calibration.cyclo ./VL53L1X\ ToF/Src/VL53L1X_calibration.d ./VL53L1X\ ToF/Src/VL53L1X_calibration.o ./VL53L1X\ ToF/Src/VL53L1X_calibration.su ./VL53L1X\ ToF/Src/VL53L1_platform.cyclo ./VL53L1X\ ToF/Src/VL53L1_platform.d ./VL53L1X\ ToF/Src/VL53L1_platform.o ./VL53L1X\ ToF/Src/VL53L1_platform.su

.PHONY: clean-VL53L1X-20-ToF-2f-Src

