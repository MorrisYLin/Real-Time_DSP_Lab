################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/s70kl1281/s70kl1281.c 

OBJS += \
./Drivers/BSP/Components/s70kl1281/s70kl1281.o 

C_DEPS += \
./Drivers/BSP/Components/s70kl1281/s70kl1281.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/s70kl1281/%.o Drivers/BSP/Components/s70kl1281/%.su Drivers/BSP/Components/s70kl1281/%.cyclo: ../Drivers/BSP/Components/s70kl1281/%.c Drivers/BSP/Components/s70kl1281/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32H735G_DISCO -DDEBUG -DSTM32H735xx -D_FPU_PRESENT=1 -DARM_MATH_CM7 -c -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"../Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"../Drivers/CMSIS/Core/Include" -I"../Drivers/BSP/STM32H735G-DK" -I"../Drivers/BSP/Components/Common" -I../Middlewares/ST/STM32_Audio/Addons/PDM/Inc -I../Utilities/lcd -I../Drivers/CMSIS/DSP/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-s70kl1281

clean-Drivers-2f-BSP-2f-Components-2f-s70kl1281:
	-$(RM) ./Drivers/BSP/Components/s70kl1281/s70kl1281.cyclo ./Drivers/BSP/Components/s70kl1281/s70kl1281.d ./Drivers/BSP/Components/s70kl1281/s70kl1281.o ./Drivers/BSP/Components/s70kl1281/s70kl1281.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-s70kl1281

