################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utilities/lcd/stm32_lcd.c 

OBJS += \
./Utilities/lcd/stm32_lcd.o 

C_DEPS += \
./Utilities/lcd/stm32_lcd.d 


# Each subdirectory must supply rules for building sources it contributes
Utilities/lcd/%.o Utilities/lcd/%.su Utilities/lcd/%.cyclo: ../Utilities/lcd/%.c Utilities/lcd/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32H735G_DISCO -DDEBUG -DSTM32H735xx -D_FPU_PRESENT=1 -DARM_MATH_CM7 -c -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"../Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"../Drivers/CMSIS/Core/Include" -I"../Drivers/BSP/STM32H735G-DK" -I"../Drivers/BSP/Components/Common" -I../Middlewares/ST/STM32_Audio/Addons/PDM/Inc -I../Utilities/lcd -I../Drivers/CMSIS/DSP/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Utilities-2f-lcd

clean-Utilities-2f-lcd:
	-$(RM) ./Utilities/lcd/stm32_lcd.cyclo ./Utilities/lcd/stm32_lcd.d ./Utilities/lcd/stm32_lcd.o ./Utilities/lcd/stm32_lcd.su

.PHONY: clean-Utilities-2f-lcd

