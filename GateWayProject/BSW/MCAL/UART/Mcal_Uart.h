#ifndef MCAL_UART_H
#define MCAL_UART_H

#include "Std_Types.h"
#include "Mcal_Dio.h"

/* Định nghĩa kiểu dữ liệu theo AUTOSAR */
typedef uint8 Uart_ChannelType;
typedef uint32 Uart_BaudrateType;
typedef uint8 Uart_DataType;

typedef enum {
    UART_IDLE,
    UART_BUSY,
    UART_COMPLETED
} Uart_StatusType;

typedef enum {
    UART_PARITY_NONE,
    UART_PARITY_EVEN,
    UART_PARITY_ODD
} Uart_ParityType;

typedef enum {
    UART_STOP_BITS_1,
    UART_STOP_BITS_2
} Uart_StopBitsType;

typedef enum {
    UART_MODE_POLLING,
    UART_MODE_INTERRUPT,
    UART_MODE_DMA
} Uart_ModeType;

typedef struct {
    Uart_ChannelType ChannelId;
    Uart_BaudrateType Baudrate;
    uint8 DataBits; /* 8 hoặc 9 */
    Uart_ParityType Parity;
    Uart_StopBitsType StopBits;
    Uart_ModeType Mode;
    Dio_PortType Port; /* Port GPIO: DIO_PORTA, DIO_PORTB, v.v. */
    uint8 TxPin; /* Chân TX */
    uint8 RxPin; /* Chân RX */
    uint8 AlternateFunction; /* Alternate function: AF7, AF8, v.v. */
} Uart_ConfigType;

/* Định nghĩa kênh và hằng số */
#define UART_CHANNEL_USART1 0x01
#define UART_CHANNEL_USART2 0x02
#define UART_CHANNEL_USART3 0x03
#define UART_BAUDRATE_115200 115200

/* API theo chuẩn AUTOSAR */
/**
 * @brief Initializes the UART module with the provided configuration.
 * @param ConfigPtr Pointer to the UART configuration structure.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Uart_Init(const Uart_ConfigType* ConfigPtr);

/**
 * @brief Writes data to a specific UART channel.
 * @param Channel Channel identifier (UART_CHANNEL_USART1 to UART_CHANNEL_USART3).
 * @param Data Pointer to the data buffer to transmit.
 * @param Length Length of the data to transmit.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Uart_Write(Uart_ChannelType Channel, const Uart_DataType* Data, uint8 Length);

/**
 * @brief Reads data from a specific UART channel.
 * @param Channel Channel identifier (UART_CHANNEL_USART1 to UART_CHANNEL_USART3).
 * @param Data Pointer to the buffer to store received data.
 * @param Length Length of the data to receive.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Uart_Read(Uart_ChannelType Channel, Uart_DataType* Data, uint8 Length);

/**
 * @brief Gets the status of a specific UART channel.
 * @param Channel Channel identifier (UART_CHANNEL_USART1 to UART_CHANNEL_USART3).
 * @return Uart_StatusType Current status of the channel.
 */
Uart_StatusType Uart_GetStatus(Uart_ChannelType Channel);

/**
 * @brief Sets the baud rate for a specific UART channel.
 * @param Channel Channel identifier (UART_CHANNEL_USART1 to UART_CHANNEL_USART3).
 * @param Baudrate Desired baud rate.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Uart_SetBaudrate(Uart_ChannelType Channel, Uart_BaudrateType Baudrate);

#endif
