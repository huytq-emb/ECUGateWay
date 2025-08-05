################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSW/MCAL/UART/Mcal_Uart.c \
../BSW/MCAL/UART/Mcal_Uart_Cfg.c 

OBJS += \
./BSW/MCAL/UART/Mcal_Uart.o \
./BSW/MCAL/UART/Mcal_Uart_Cfg.o 

C_DEPS += \
./BSW/MCAL/UART/Mcal_Uart.d \
./BSW/MCAL/UART/Mcal_Uart_Cfg.d 


# Each subdirectory must supply rules for building sources it contributes
BSW/MCAL/UART/%.o BSW/MCAL/UART/%.su BSW/MCAL/UART/%.cyclo: ../BSW/MCAL/UART/%.c BSW/MCAL/UART/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"D:/ECU_Sim/GateWayProject/ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F" -I"D:/ECU_Sim/GateWayProject/ThirdParty/FreeRTOS/include" -I"D:/ECU_Sim/GateWayProject/ThirdParty/FreeRTOS" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/ECU_Sim/GateWayProject/BSW/MCAL" -I"D:/ECU_Sim/GateWayProject/BSW/MCAL/Dio" -I"D:/ECU_Sim/GateWayProject/BSW/MCAL/ADC" -I"D:/ECU_Sim/GateWayProject/BSW/MCAL/UART" -I"D:/ECU_Sim/GateWayProject/BSW/MCAL/CAN" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSW-2f-MCAL-2f-UART

clean-BSW-2f-MCAL-2f-UART:
	-$(RM) ./BSW/MCAL/UART/Mcal_Uart.cyclo ./BSW/MCAL/UART/Mcal_Uart.d ./BSW/MCAL/UART/Mcal_Uart.o ./BSW/MCAL/UART/Mcal_Uart.su ./BSW/MCAL/UART/Mcal_Uart_Cfg.cyclo ./BSW/MCAL/UART/Mcal_Uart_Cfg.d ./BSW/MCAL/UART/Mcal_Uart_Cfg.o ./BSW/MCAL/UART/Mcal_Uart_Cfg.su

.PHONY: clean-BSW-2f-MCAL-2f-UART

