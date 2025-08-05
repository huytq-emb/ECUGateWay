################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSW/MCAL/Dio/Mcal_Dio.c \
../BSW/MCAL/Dio/Mcal_Dio_Cfg.c 

OBJS += \
./BSW/MCAL/Dio/Mcal_Dio.o \
./BSW/MCAL/Dio/Mcal_Dio_Cfg.o 

C_DEPS += \
./BSW/MCAL/Dio/Mcal_Dio.d \
./BSW/MCAL/Dio/Mcal_Dio_Cfg.d 


# Each subdirectory must supply rules for building sources it contributes
BSW/MCAL/Dio/%.o BSW/MCAL/Dio/%.su BSW/MCAL/Dio/%.cyclo: ../BSW/MCAL/Dio/%.c BSW/MCAL/Dio/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"D:/ECU_Sim/GateWayProject/ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F" -I"D:/ECU_Sim/GateWayProject/ThirdParty/FreeRTOS/include" -I"D:/ECU_Sim/GateWayProject/ThirdParty/FreeRTOS" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/ECU_Sim/GateWayProject/BSW/MCAL" -I"D:/ECU_Sim/GateWayProject/BSW/MCAL/Dio" -I"D:/ECU_Sim/GateWayProject/BSW/MCAL/ADC" -I"D:/ECU_Sim/GateWayProject/BSW/MCAL/UART" -I"D:/ECU_Sim/GateWayProject/BSW/MCAL/CAN" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSW-2f-MCAL-2f-Dio

clean-BSW-2f-MCAL-2f-Dio:
	-$(RM) ./BSW/MCAL/Dio/Mcal_Dio.cyclo ./BSW/MCAL/Dio/Mcal_Dio.d ./BSW/MCAL/Dio/Mcal_Dio.o ./BSW/MCAL/Dio/Mcal_Dio.su ./BSW/MCAL/Dio/Mcal_Dio_Cfg.cyclo ./BSW/MCAL/Dio/Mcal_Dio_Cfg.d ./BSW/MCAL/Dio/Mcal_Dio_Cfg.o ./BSW/MCAL/Dio/Mcal_Dio_Cfg.su

.PHONY: clean-BSW-2f-MCAL-2f-Dio

