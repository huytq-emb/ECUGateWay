/*
 * Mcal_Uart_Hw.h
 *
 *  Created on: Aug 5, 2025
 *      Author: Truong Quoc Huy
 */
#ifndef MCAL_UART_HW_H
#define MCAL_UART_HW_H

#include "stm32f4xx.h"

/* Ánh xạ kênh UART */
static USART_TypeDef* const uartMap[] = {
    [UART_CHANNEL_USART1] = USART1,
    [UART_CHANNEL_USART2] = USART2,
    [UART_CHANNEL_USART3] = USART3
};

/* Kích hoạt clock cho UART */
static void Uart_EnableClock(Uart_ChannelType ChannelId);

/* Cấu hình GPIO cho UART */
static void Uart_ConfigureGpio(Uart_ChannelType ChannelId, Dio_PortType Port, uint8 TxPin, uint8 RxPin, uint8 AlternateFunction);

/* Cấu hình DMA cho UART */
static void Uart_ConfigureDma(Uart_ChannelType ChannelId, Uart_DataType* TxBuffer, Uart_DataType* RxBuffer, uint8 TxLength, uint8 RxLength);

#endif
