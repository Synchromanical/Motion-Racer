################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/wm8994/wm8994.c 

OBJS += \
./Drivers/BSP/Components/wm8994/wm8994.o 

C_DEPS += \
./Drivers/BSP/Components/wm8994/wm8994.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/wm8994/%.o Drivers/BSP/Components/wm8994/%.su Drivers/BSP/Components/wm8994/%.cyclo: ../Drivers/BSP/Components/wm8994/%.c Drivers/BSP/Components/wm8994/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L475xx -c -I../Core/Inc -IC:/Users/Alex/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.1/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/Alex/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.1/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -IC:/Users/Alex/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.1/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/Alex/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.1/Drivers/CMSIS/Include -I../X-CUBE-MEMS1/Target -IC:/Users/Alex/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-MEMS1/11.1.0/Drivers/BSP/Components/lsm6dsl -I"C:/Users/Alex/STM32CubeIDE/workspace_1.16.1/NEHD_Project/Drivers/BSP/B-L475E-IOT01" -I"C:/Users/Alex/STM32CubeIDE/workspace_1.16.1/NEHD_Project/Drivers/BSP/Components/lsm6dsl" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-wm8994

clean-Drivers-2f-BSP-2f-Components-2f-wm8994:
	-$(RM) ./Drivers/BSP/Components/wm8994/wm8994.cyclo ./Drivers/BSP/Components/wm8994/wm8994.d ./Drivers/BSP/Components/wm8994/wm8994.o ./Drivers/BSP/Components/wm8994/wm8994.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-wm8994

