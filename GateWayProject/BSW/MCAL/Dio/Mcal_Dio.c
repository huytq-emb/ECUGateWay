#include "Mcal_Dio.h"
#include "Mcal_Dio_Cfg.h"
#include "Mcal_Dio_Hw.h"

/* Lưu trữ con trỏ cấu hình */
static const Dio_ConfigType* dioConfigPtr = NULL;

/* Trạng thái kích hoạt clock */
static uint8 clockEnabled[9] = {0}; /* Hỗ trợ DIO_PORTA đến DIO_PORTI */

/* Hàm hỗ trợ: Lấy con trỏ GPIO từ PortId */
static GPIO_TypeDef* Dio_GetPort(Dio_PortType PortId) {
    if (PortId >= DIO_PORTA && PortId <= DIO_PORTI) {
        return portMap[PortId];
    }
    return NULL;
}

/* Hàm hỗ trợ: Kích hoạt clock cho cổng */
static void Dio_EnableClock(Dio_PortType PortId) {
    if (PortId >= DIO_PORTA && PortId <= DIO_PORTI && !clockEnabled[PortId - DIO_PORTA]) {
        clockEnabled[PortId - DIO_PORTA] = 1;
        switch (PortId) {
            case DIO_PORTA: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; break;
            case DIO_PORTB: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; break;
            case DIO_PORTC: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; break;
            case DIO_PORTD: RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; break;
            case DIO_PORTE: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; break;
            case DIO_PORTF: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN; break;
            case DIO_PORTG: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN; break;
            case DIO_PORTH: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN; break;
            case DIO_PORTI: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN; break;
            default: break;
        }
    }
}

/**
 * @brief Initializes the DIO module with the provided configuration.
 * @param ConfigPtr Pointer to the DIO configuration structure.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_Init(const Dio_ConfigType* ConfigPtr) {
    if (ConfigPtr == NULL) {
        return E_NOT_OK;
    }

    dioConfigPtr = ConfigPtr; /* Lưu con trỏ cấu hình */

    /* Kiểm tra và cấu hình các kênh */
    for (uint8 i = 0; i < DIO_CHANNEL_COUNT; i++) {
        /* Kiểm tra tính hợp lệ của cấu hình */
        if (ConfigPtr[i].Pin >= 16 || Dio_GetPort(ConfigPtr[i].Port) == NULL ||
            ConfigPtr[i].Mode > DIO_OUTPUT || ConfigPtr[i].OutputType > DIO_OPENDRAIN ||
            ConfigPtr[i].Pull > DIO_PULL_DOWN) {
            return E_NOT_OK;
        }

        /* Kiểm tra trùng lặp cấu hình */
        for (uint8 j = 0; j < i; j++) {
            if (ConfigPtr[i].Port == ConfigPtr[j].Port && ConfigPtr[i].Pin == ConfigPtr[j].Pin) {
                return E_NOT_OK; /* Trùng lặp chân */
            }
        }

        Dio_EnableClock(ConfigPtr[i].Port);
        GPIO_TypeDef* port = Dio_GetPort(ConfigPtr[i].Port);

        /* Cấu hình mode: 01 = Output, 00 = Input */
        if (ConfigPtr[i].Mode == DIO_OUTPUT) {
            port->MODER |= (0x01 << (ConfigPtr[i].Pin * 2));
        } else {
            port->MODER &= ~(0x03 << (ConfigPtr[i].Pin * 2));
        }
        /* Cấu hình output type: 0 = Push-pull, 1 = Open-drain */
        if (ConfigPtr[i].OutputType == DIO_OPENDRAIN) {
            port->OTYPER |= (0x01 << ConfigPtr[i].Pin);
        } else {
            port->OTYPER &= ~(0x01 << ConfigPtr[i].Pin);
        }
        /* Cấu hình pull-up/pull-down: 00 = No pull, 01 = Pull-up, 10 = Pull-down */
        port->PUPDR &= ~(0x03 << (ConfigPtr[i].Pin * 2));
        port->PUPDR |= (ConfigPtr[i].Pull << (ConfigPtr[i].Pin * 2));
    }
    return E_OK;
}

/**
 * @brief Writes a level to a specific channel.
 * @param ChannelId Channel identifier (0-15).
 * @param Level Level to set (STD_HIGH or STD_LOW).
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level, Dio_PortType PortId) {
    if (dioConfigPtr == NULL || ChannelId >= 16 || Dio_GetPort(PortId) == NULL) {
        return E_NOT_OK;
    }

    /* Kiểm tra xem kênh có được cấu hình là output không */
    for (uint8 i = 0; i < DIO_CHANNEL_COUNT; i++) {
        if (dioConfigPtr[i].Port == PortId && dioConfigPtr[i].Pin == ChannelId) {
            if (dioConfigPtr[i].Mode != DIO_OUTPUT) {
                return E_NOT_OK;
            }
            break;
        }
    }

    GPIO_TypeDef* port = Dio_GetPort(PortId);
    if (Level == STD_HIGH) {
        port->BSRR = (1U << ChannelId);
    } else {
        port->BSRR = (1U << (ChannelId + 16));
    }
    return E_OK;
}

/**
 * @brief Reads the level of a specific channel.
 * @param ChannelId Channel identifier (0-15).
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @return Dio_LevelType Level of the channel (STD_HIGH or STD_LOW).
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId, Dio_PortType PortId) {
    if (dioConfigPtr == NULL || ChannelId >= 16 || Dio_GetPort(PortId) == NULL) {
        return STD_LOW;
    }

    /* Kiểm tra xem kênh có được cấu hình là input không */
    for (uint8 i = 0; i < DIO_CHANNEL_COUNT; i++) {
        if (dioConfigPtr[i].Port == PortId && dioConfigPtr[i].Pin == ChannelId) {
            if (dioConfigPtr[i].Mode != DIO_INPUT) {
                return STD_LOW;
            }
            break;
        }
    }

    GPIO_TypeDef* port = Dio_GetPort(PortId);
    return (port->IDR & (1U << ChannelId)) ? STD_HIGH : STD_LOW;
}

/**
 * @brief Writes a level to a specific port.
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @param Level Level to set for the port.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level) {
    GPIO_TypeDef* port = Dio_GetPort(PortId);
    if (port == NULL) {
        return E_NOT_OK;
    }

    port->ODR = Level;
    return E_OK;
}

/**
 * @brief Reads the level of a specific port.
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @return Dio_PortLevelType Level of the port.
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId) {
    GPIO_TypeDef* port = Dio_GetPort(PortId);
    if (port == NULL) {
        return 0;
    }

    return (Dio_PortLevelType)(port->IDR & 0xFFFF);
}

/**
 * @brief Writes a level to a specific channel group.
 * @param ChannelGroupIdPtr Pointer to the channel group configuration.
 * @param Level Level to set for the channel group.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr, Dio_PortLevelType Level) {
    if (ChannelGroupIdPtr == NULL || Dio_GetPort(ChannelGroupIdPtr->Port) == NULL) {
        return E_NOT_OK;
    }

    GPIO_TypeDef* port = Dio_GetPort(ChannelGroupIdPtr->Port);
    uint16 current = port->ODR;
    uint16 mask = ChannelGroupIdPtr->ChannelMask << ChannelGroupIdPtr->ChannelOffset;
    uint16 value = (Level << ChannelGroupIdPtr->ChannelOffset) & mask;
    port->ODR = (current & ~mask) | value;
    return E_OK;
}

/**
 * @brief Reads the level of a specific channel group.
 * @param ChannelGroupIdPtr Pointer to the channel group configuration.
 * @return Dio_PortLevelType Level of the channel group.
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr) {
    if (ChannelGroupIdPtr == NULL || Dio_GetPort(ChannelGroupIdPtr->Port) == NULL) {
        return 0;
    }

    GPIO_TypeDef* port = Dio_GetPort(ChannelGroupIdPtr->Port);
    return (Dio_PortLevelType)((port->IDR >> ChannelGroupIdPtr->ChannelOffset) & ChannelGroupIdPtr->ChannelMask);
}

/**
 * @brief Flips the level of a specific channel.
 * @param ChannelId Channel identifier (0-15).
 * @param PortId Port identifier (DIO_PORTA to DIO_PORTI).
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Dio_FlipChannel(Dio_ChannelType ChannelId, Dio_PortType PortId) {
    if (dioConfigPtr == NULL || ChannelId >= 16 || Dio_GetPort(PortId) == NULL) {
        return E_NOT_OK;
    }

    /* Kiểm tra xem kênh có được cấu hình là output không */
    for (uint8 i = 0; i < DIO_CHANNEL_COUNT; i++) {
        if (dioConfigPtr[i].Port == PortId && dioConfigPtr[i].Pin == ChannelId) {
            if (dioConfigPtr[i].Mode != DIO_OUTPUT) {
                return E_NOT_OK;
            }
            break;
        }
    }

    Dio_LevelType current = Dio_ReadChannel(ChannelId, PortId);
    return Dio_WriteChannel(ChannelId, current == STD_HIGH ? STD_LOW : STD_HIGH, PortId);
}
