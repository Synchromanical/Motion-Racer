################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/b_l475e_iot01a2_bus.c \
../Core/Src/main.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c 

OBJS += \
./Core/Src/b_l475e_iot01a2_bus.o \
./Core/Src/main.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o 

C_DEPS += \
./Core/Src/b_l475e_iot01a2_bus.d \
./Core/Src/main.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L475xx -c -I../Core/Inc -IC:/Users/Alex/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.1/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/Alex/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.1/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -IC:/Users/Alex/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.1/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/Alex/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.1/Drivers/CMSIS/Include -I../X-CUBE-MEMS1/Target -IC:/Users/Alex/STM32Cube/Repository//Packs/STMicroelectronics/X-CUBE-MEMS1/11.1.0/Drivers/BSP/Components/lsm6dsl -I"C:/Users/Alex/STM32CubeIDE/workspace_1.16.1/NEHD_Project/Drivers/BSP/B-L475E-IOT01" -I"C:/Users/Alex/STM32CubeIDE/workspace_1.16.1/NEHD_Project/Drivers/BSP/Components/lsm6dsl" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/b_l475e_iot01a2_bus.cyclo ./Core/Src/b_l475e_iot01a2_bus.d ./Core/Src/b_l475e_iot01a2_bus.o ./Core/Src/b_l475e_iot01a2_bus.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su

.PHONY: clean-Core-2f-Src

