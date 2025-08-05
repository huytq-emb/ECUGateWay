#ifndef MCAL_DIO_H
#define MCAL_DIO_H

#include "Std_Types.h"

/* Định nghĩa kiểu dữ liệu theo AUTOSAR */
typedef uint32 Dio_ChannelType;
typedef uint32 Dio_PortType;
typedef uint8 Dio_LevelType;
typedef uint16 Dio_PortLevelType;

typedef struct {
    Dio_PortType Port;
    uint8 ChannelOffset;
    uint8 ChannelMask;
} Dio_ChannelGroupType;

/* Định nghĩa kiểu cấu hình */
typedef enum {
    DIO_INPUT,
    DIO_OUTPUT
} Dio_ModeType;

typedef enum {
    DIO_PUSHPULL,
    DIO_OPENDRAIN
} Dio_OutputType;

typedef enum {
    DIO_NO_PULL,
    DIO_PULL_UP,
    DIO_PULL_DOWN
} Dio_PullType;

typedef struct {
    Dio_PortType Port;
    uint8 Pin;
    Dio_ModeType Mode;
    Dio_OutputType OutputType;
    Dio_PullType Pull;
} Dio_ConfigType;

/* Định nghĩa cổng và kênh */
#define DIO_PORTA 0x01
#define DIO_PORTB 0x02
#define DIO_PORTC 0x03
#define DIO_PORTD 0x04
#define DIO_PORTE 0x05
#define DIO_PORTF 0x06
#define DIO_PORTG 0x07
#define DIO_PORTH 0x08
#define DIO_PORTI 0x09
#define DIO_CHANNEL_0 0x00
#define DIO_CHANNEL_1 0x01
#define DIO_CHANNEL_GROUP_A {DIO_PORTA, 0, 0x0F} /* GPIOA Pin 0-3 */
#define DIO_CHANNEL_GROUP_B {DIO_PORTB, 0, 0x0F} /* GPIOB Pin 0-3 */
#define DIO_CHANNEL_GROUP_F {DIO_PORTF, 0, 0x0F} /* GPIOF Pin 0-3 */

/* API theo chuẩn AUTOSAR */
/**
 * @brief Initializes the DIO module with the provided configuration.
 * @param ConfigPtr Pointer to the DIO configuration structure.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_Init(const Dio_ConfigType* ConfigPtr);

/**
 * @brief Writes a level to a specific channel.
 * @param ChannelId Channel identifier (0-15).
 * @param Level Level to set (STD_HIGH or STD_LOW).
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level, Dio_PortType PortId);

/**
 * @brief Reads the level of a specific channel.
 * @param ChannelId Channel identifier (0-15).
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @return Dio_LevelType Level of the channel (STD_HIGH or STD_LOW).
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId, Dio_PortType PortId);

/**
 * @brief Writes a level to a specific port.
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @param Level Level to set for the port.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);

/**
 * @brief Reads the level of a specific port.
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @return Dio_PortLevelType Level of the port.
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId);

/**
 * @brief Writes a level to a specific channel group.
 * @param ChannelGroupIdPtr Pointer to the channel group configuration.
 * @param Level Level to set for the channel group.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr, Dio_PortLevelType Level);

/**
 * @brief Reads the level of a specific channel group.
 * @param ChannelGroupIdPtr Pointer to the channel group configuration.
 * @return Dio_PortLevelType Level of the channel group.
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr);

/**
 * @brief Flips the level of a specific channel.
 * @param ChannelId Channel identifier (0-15).
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_FlipChannel(Dio_ChannelType ChannelId, Dio_PortType PortId);

#endif
