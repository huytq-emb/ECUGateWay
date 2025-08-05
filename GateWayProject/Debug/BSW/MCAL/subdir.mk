################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSW/MCAL/Mcal_Adc.c \
../BSW/MCAL/Mcal_Adc_Cfg.c \
../BSW/MCAL/Mcal_Dio.c \
../BSW/MCAL/Mcal_Dio_Cfg.c 

OBJS += \
./BSW/MCAL/Mcal_Adc.o \
./BSW/MCAL/Mcal_Adc_Cfg.o \
./BSW/MCAL/Mcal_Dio.o \
./BSW/MCAL/Mcal_Dio_Cfg.o 

C_DEPS += \
./BSW/MCAL/Mcal_Adc.d \
./BSW/MCAL/Mcal_Adc_Cfg.d \
./BSW/MCAL/Mcal_Dio.d \
./BSW/MCAL/Mcal_Dio_Cfg.d 


# Each subdirectory must supply rules for building sources it contributes
BSW/MCAL/%.o BSW/MCAL/%.su BSW/MCAL/%.cyclo: ../BSW/MCAL/%.c BSW/MCAL/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"D:/ECU_Sim/GateWayProject/ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F" -I"D:/ECU_Sim/GateWayProject/ThirdParty/SEGGER/Config" -I"D:/ECU_Sim/GateWayProject/ThirdParty/SEGGER/OS" -I"D:/ECU_Sim/GateWayProject/ThirdParty/SEGGER/SEGGER" -I"D:/ECU_Sim/GateWayProject/ThirdParty/FreeRTOS/include" -I"D:/ECU_Sim/GateWayProject/ThirdParty/FreeRTOS" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/ECU_Sim/GateWayProject/BSW/MCAL" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSW-2f-MCAL

clean-BSW-2f-MCAL:
	-$(RM) ./BSW/MCAL/Mcal_Adc.cyclo ./BSW/MCAL/Mcal_Adc.d ./BSW/MCAL/Mcal_Adc.o ./BSW/MCAL/Mcal_Adc.su ./BSW/MCAL/Mcal_Adc_Cfg.cyclo ./BSW/MCAL/Mcal_Adc_Cfg.d ./BSW/MCAL/Mcal_Adc_Cfg.o ./BSW/MCAL/Mcal_Adc_Cfg.su ./BSW/MCAL/Mcal_Dio.cyclo ./BSW/MCAL/Mcal_Dio.d ./BSW/MCAL/Mcal_Dio.o ./BSW/MCAL/Mcal_Dio.su ./BSW/MCAL/Mcal_Dio_Cfg.cyclo ./BSW/MCAL/Mcal_Dio_Cfg.d ./BSW/MCAL/Mcal_Dio_Cfg.o ./BSW/MCAL/Mcal_Dio_Cfg.su

.PHONY: clean-BSW-2f-MCAL

