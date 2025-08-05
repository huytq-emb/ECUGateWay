#include "Mcal_Uart_Cfg.h"
#include "Mcal_Dio.h"

/* Cấu hình:
 * - USART1: 115200 baud, 8-N-1, interrupt mode, PA9 (TX), PA10 (RX), AF7
 * - USART2: 115200 baud, 8-N-1, DMA mode, PA2 (TX), PA3 (RX), AF7
 * - USART3: 115200 baud, 8-N-1, polling mode, PB10 (TX), PB11 (RX), AF7
 */
const Uart_ConfigType UartConfig[] = {
    {
        .ChannelId = UART_CHANNEL_USART1,
        .Baudrate = UART_BAUDRATE_115200,
        .DataBits = 8,
        .Parity = UART_PARITY_NONE,
        .StopBits = UART_STOP_BITS_1,
        .Mode = UART_MODE_INTERRUPT,
        .Port = DIO_PORTA,
        .TxPin = 9,
        .RxPin = 10,
        .AlternateFunction = 7
    },
    {
        .ChannelId = UART_CHANNEL_USART2,
        .Baudrate = UART_BAUDRATE_115200,
        .DataBits = 8,
        .Parity = UART_PARITY_NONE,
        .StopBits = UART_STOP_BITS_1,
        .Mode = UART_MODE_DMA,
        .Port = DIO_PORTA,
        .TxPin = 2,
        .RxPin = 3,
        .AlternateFunction = 7
    },
    {
        .ChannelId = UART_CHANNEL_USART3,
        .Baudrate = UART_BAUDRATE_115200,
        .DataBits = 8,
        .Parity = UART_PARITY_NONE,
        .StopBits = UART_STOP_BITS_1,
        .Mode = UART_MODE_POLLING,
        .Port = DIO_PORTB,
        .TxPin = 10,
        .RxPin = 11,
        .AlternateFunction = 7
    }
};
