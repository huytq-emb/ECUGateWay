#include "Mcal_Uart.h"
#include "Mcal_Uart_Cfg.h"
#include "Mcal_Uart_Hw.h"
#include "Mcal_Dio.h"

/* Biến toàn cục để lưu cấu hình và trạng thái */
static const Uart_ConfigType* UartConfigPtr = NULL;
static Uart_StatusType UartStatus[UART_CHANNEL_COUNT] = {UART_IDLE};
static Uart_DataType* UartTxBuffer[UART_CHANNEL_COUNT] = {NULL};
static Uart_DataType* UartRxBuffer[UART_CHANNEL_COUNT] = {NULL};
static uint8 UartTxLength[UART_CHANNEL_COUNT] = {0};
static uint8 UartRxLength[UART_CHANNEL_COUNT] = {0};
static uint8 UartTxIndex[UART_CHANNEL_COUNT] = {0};
static uint8 UartRxIndex[UART_CHANNEL_COUNT] = {0};
static uint8 clockEnabled[3] = {0}; /* Hỗ trợ USART1, USART2, USART3 */

/* Hàm hỗ trợ: Lấy con trỏ USART từ ChannelId */
static USART_TypeDef* Uart_GetChannel(Uart_ChannelType ChannelId) {
    if (ChannelId >= UART_CHANNEL_USART1 && ChannelId <= UART_CHANNEL_USART3) {
        return uartMap[ChannelId];
    }
    return NULL;
}

/* Hàm hỗ trợ: Kích hoạt clock cho USART */
static void Uart_EnableClock(Uart_ChannelType ChannelId) {
    if (ChannelId >= UART_CHANNEL_USART1 && ChannelId <= UART_CHANNEL_USART3 && !clockEnabled[ChannelId - UART_CHANNEL_USART1]) {
        clockEnabled[ChannelId - UART_CHANNEL_USART1] = 1;
        switch (ChannelId) {
            case UART_CHANNEL_USART1: RCC->APB2ENR |= RCC_APB2ENR_USART1EN; break;
            case UART_CHANNEL_USART2: RCC->APB1ENR |= RCC_APB1ENR_USART2EN; break;
            case UART_CHANNEL_USART3: RCC->APB1ENR |= RCC_APB1ENR_USART3EN; break;
            default: break;
        }
    }
}

/* Hàm hỗ trợ: Cấu hình GPIO cho UART */
static void Uart_ConfigureGpio(Uart_ChannelType ChannelId, Dio_PortType Port, uint8 TxPin, uint8 RxPin, uint8 AlternateFunction) {
    GPIO_TypeDef* gpio = NULL;
    switch (Port) {
        case DIO_PORTA: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; gpio = GPIOA; break;
        case DIO_PORTB: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; gpio = GPIOB; break;
        default: return;
    }
    if (TxPin >= 16 || RxPin >= 16 || AlternateFunction > 15) return;

    /* Cấu hình TX và RX thành alternate function mode */
    gpio->MODER &= ~(0xF << (TxPin * 2)); /* Xóa cấu hình cũ */
    gpio->MODER |= (0x2 << (TxPin * 2)); /* AF mode cho TX */
    gpio->MODER &= ~(0xF << (RxPin * 2));
    gpio->MODER |= (0x2 << (RxPin * 2)); /* AF mode cho RX */

    /* Cấu hình alternate function */
    if (TxPin < 8) {
        gpio->AFR[0] &= ~(0xF << (TxPin * 4));
        gpio->AFR[0] |= (AlternateFunction << (TxPin * 4));
    } else {
        gpio->AFR[1] &= ~(0xF << ((TxPin - 8) * 4));
        gpio->AFR[1] |= (AlternateFunction << ((TxPin - 8) * 4));
    }
    if (RxPin < 8) {
        gpio->AFR[0] &= ~(0xF << (RxPin * 4));
        gpio->AFR[0] |= (AlternateFunction << (RxPin * 4));
    } else {
        gpio->AFR[1] &= ~(0xF << ((RxPin - 8) * 4));
        gpio->AFR[1] |= (AlternateFunction << ((RxPin - 8) * 4));
    }
}

/* Hàm hỗ trợ: Cấu hình DMA cho UART */
static void Uart_ConfigureDma(Uart_ChannelType ChannelId, Uart_DataType* TxBuffer, Uart_DataType* RxBuffer, uint8 TxLength, uint8 RxLength) {
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMA2EN; /* Bật clock DMA1 và DMA2 */
    DMA_Stream_TypeDef* txStream = NULL;
    DMA_Stream_TypeDef* rxStream = NULL;
    uint32 txChannel = 4, rxChannel = 4;
    uint32 txIrq = 0, rxIrq = 0;

    switch (ChannelId) {
        case UART_CHANNEL_USART1:
            txStream = DMA2_Stream7; rxStream = DMA2_Stream5;
            txIrq = DMA2_Stream7_IRQn; rxIrq = DMA2_Stream5_IRQn;
            break;
        case UART_CHANNEL_USART2:
            txStream = DMA1_Stream6; rxStream = DMA1_Stream5;
            txIrq = DMA1_Stream6_IRQn; rxIrq = DMA1_Stream5_IRQn;
            break;
        case UART_CHANNEL_USART3:
            txStream = DMA1_Stream3; rxStream = DMA1_Stream1;
            txIrq = DMA1_Stream3_IRQn; rxIrq = DMA1_Stream1_IRQn;
            break;
        default: return;
    }

    /* Cấu hình DMA TX */
    if (TxBuffer != NULL && TxLength > 0) {
        txStream->CR = 0; /* Reset CR */
        txStream->CR |= (txChannel << 25); /* Channel */
        txStream->CR |= (1 << 10); /* Memory increment */
        txStream->CR |= (1 << 6); /* Memory-to-peripheral */
        txStream->CR |= (1 << 8); /* Enable TC interrupt */
        txStream->PAR = (uint32_t)&Uart_GetChannel(ChannelId)->DR;
        txStream->M0AR = (uint32_t)TxBuffer;
        txStream->NDTR = TxLength;
        NVIC_EnableIRQ(txIrq);
    }

    /* Cấu hình DMA RX */
    if (RxBuffer != NULL && RxLength > 0) {
        rxStream->CR = 0; /* Reset CR */
        rxStream->CR |= (rxChannel << 25); /* Channel */
        rxStream->CR |= (1 << 10); /* Memory increment */
        rxStream->CR |= (0 << 6); /* Peripheral-to-memory */
        rxStream->CR |= (1 << 8); /* Enable TC interrupt */
        rxStream->PAR = (uint32_t)&Uart_GetChannel(ChannelId)->DR;
        rxStream->M0AR = (uint32_t)RxBuffer;
        rxStream->NDTR = RxLength;
        NVIC_EnableIRQ(rxIrq);
    }
}

/**
 * @brief Initializes the UART module with the provided configuration.
 * @param ConfigPtr Pointer to the UART configuration structure.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Uart_Init(const Uart_ConfigType* ConfigPtr) {
    if (ConfigPtr == NULL) return E_NOT_OK;

    UartConfigPtr = ConfigPtr;

    for (uint8 i = 0; i < UART_CHANNEL_COUNT; i++) {
        /* Kiểm tra tính hợp lệ của cấu hình */
        if (Uart_GetChannel(ConfigPtr[i].ChannelId) == NULL ||
            ConfigPtr[i].Baudrate == 0 ||
            (ConfigPtr[i].DataBits != 8 && ConfigPtr[i].DataBits != 9) ||
            ConfigPtr[i].Parity > UART_PARITY_ODD ||
            ConfigPtr[i].StopBits > UART_STOP_BITS_2 ||
            ConfigPtr[i].Mode > UART_MODE_DMA ||
            ConfigPtr[i].Port > DIO_PORTB ||
            ConfigPtr[i].TxPin >= 16 || ConfigPtr[i].RxPin >= 16 ||
            ConfigPtr[i].AlternateFunction > 15) {
            return E_NOT_OK;
        }

        UartStatus[i] = UART_IDLE;
        UartTxBuffer[i] = NULL;
        UartRxBuffer[i] = NULL;
        UartTxLength[i] = 0;
        UartRxLength[i] = 0;
        UartTxIndex[i] = 0;
        UartRxIndex[i] = 0;

        /* Kích hoạt clock và cấu hình GPIO */
        Uart_EnableClock(ConfigPtr[i].ChannelId);
        Uart_ConfigureGpio(ConfigPtr[i].ChannelId, ConfigPtr[i].Port, ConfigPtr[i].TxPin, ConfigPtr[i].RxPin, ConfigPtr[i].AlternateFunction);

        USART_TypeDef* usart = Uart_GetChannel(ConfigPtr[i].ChannelId);

        /* Tính baud rate */
        uint32 clock = (ConfigPtr[i].ChannelId == UART_CHANNEL_USART1) ? 84000000UL : 42000000UL; /* APB2: 84MHz, APB1: 42MHz */
        uint32 divider = (clock * 25) / (4 * ConfigPtr[i].Baudrate);
        uint16 mantissa = divider / 100;
        uint8 fraction = ((divider % 100) * 16 + 50) / 100;
        usart->BRR = (mantissa << 4) | (fraction & 0x0F);

        /* Cấu hình CR1: Data bits, Parity, Enable USART */
        usart->CR1 = 0; /* Reset CR1 */
        if (ConfigPtr[i].DataBits == 9) {
            usart->CR1 |= (1 << 12); /* M = 1: 9 data bits */
        } else {
            usart->CR1 &= ~(1 << 12); /* M = 0: 8 data bits */
        }
        if (ConfigPtr[i].Parity != UART_PARITY_NONE) {
            usart->CR1 |= (1 << 10); /* PCE = 1: Enable parity */
            if (ConfigPtr[i].Parity == UART_PARITY_ODD) {
                usart->CR1 |= (1 << 9); /* PS = 1: Odd parity */
            } else {
                usart->CR1 &= ~(1 << 9); /* PS = 0: Even parity */
            }
        }
        usart->CR1 |= (1 << 3) | (1 << 2); /* TE = 1, RE = 1: Enable TX, RX */
        if (ConfigPtr[i].Mode == UART_MODE_INTERRUPT) {
            usart->CR1 |= (1 << 5) | (1 << 7); /* RXNEIE, TXEIE: Enable interrupts */
            if (ConfigPtr[i].ChannelId == UART_CHANNEL_USART1) {
                NVIC_EnableIRQ(USART1_IRQn);
            } else if (ConfigPtr[i].ChannelId == UART_CHANNEL_USART2) {
                NVIC_EnableIRQ(USART2_IRQn);
            } else if (ConfigPtr[i].ChannelId == UART_CHANNEL_USART3) {
                NVIC_EnableIRQ(USART3_IRQn);
            }
        } else if (ConfigPtr[i].Mode == UART_MODE_DMA) {
            usart->CR3 |= (1 << 7) | (1 << 6); /* DMAT, DMAR: Enable DMA */
        }
        usart->CR1 |= (1 << 13); /* UE = 1: Enable USART */

        /* Cấu hình CR2: Stop bits */
        usart->CR2 = 0; /* Reset CR2 */
        if (ConfigPtr[i].StopBits == UART_STOP_BITS_2) {
            usart->CR2 |= (2 << 12); /* STOP = 10: 2 stop bits */
        } else {
            usart->CR2 &= ~(3 << 12); /* STOP = 00: 1 stop bit */
        }
    }
    return E_OK;
}

/**
 * @brief Writes data to a specific UART channel.
 * @param Channel Channel identifier (UART_CHANNEL_USART1 to UART_CHANNEL_USART3).
 * @param Data Pointer to the data buffer to transmit.
 * @param Length Length of the data to transmit.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Uart_Write(Uart_ChannelType Channel, const Uart_DataType* Data, uint8 Length) {
    if (Channel >= UART_CHANNEL_COUNT || Data == NULL || Length == 0 || UartConfigPtr == NULL) return E_NOT_OK;
    if (UartStatus[Channel] != UART_IDLE) return E_NOT_OK;

    UartStatus[Channel] = UART_BUSY;
    UartTxBuffer[Channel] = (Uart_DataType*)Data;
    UartTxLength[Channel] = Length;
    UartTxIndex[Channel] = 0;

    USART_TypeDef* usart = Uart_GetChannel(Channel);
    if (usart == NULL) return E_NOT_OK;

    if (UartConfigPtr[Channel].Mode == UART_MODE_POLLING) {
        for (uint8 i = 0; i < Length; i++) {
            while (!(usart->SR & (1 << 7))) {} /* Chờ TXE = 1 */
            usart->DR = Data[i];
        }
        while (!(usart->SR & (1 << 6))) {} /* Chờ TC = 1 */
        UartStatus[Channel] = UART_COMPLETED;
    } else if (UartConfigPtr[Channel].Mode == UART_MODE_INTERRUPT) {
        usart->DR = UartTxBuffer[Channel][0]; /* Gửi byte đầu tiên */
        UartTxIndex[Channel]++;
    } else if (UartConfigPtr[Channel].Mode == UART_MODE_DMA) {
        Uart_ConfigureDma(Channel, (Uart_DataType*)Data, NULL, Length, 0);
        DMA_Stream_TypeDef* txStream = NULL;
        switch (Channel) {
            case UART_CHANNEL_USART1: txStream = DMA2_Stream7; break;
            case UART_CHANNEL_USART2: txStream = DMA1_Stream6; break;
            case UART_CHANNEL_USART3: txStream = DMA1_Stream3; break;
        }
        txStream->CR |= (1 << 0); /* EN = 1: Bật DMA */
    }
    return E_OK;
}

/**
 * @brief Reads data from a specific UART channel.
 * @param Channel Channel identifier (UART_CHANNEL_USART1 to UART_CHANNEL_USART3).
 * @param Data Pointer to the buffer to store received data.
 * @param Length Length of the data to receive.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Uart_Read(Uart_ChannelType Channel, Uart_DataType* Data, uint8 Length) {
    if (Channel >= UART_CHANNEL_COUNT || Data == NULL || Length == 0 || UartConfigPtr == NULL) return E_NOT_OK;
    if (UartStatus[Channel] != UART_IDLE) return E_NOT_OK;

    UartStatus[Channel] = UART_BUSY;
    UartRxBuffer[Channel] = Data;
    UartRxLength[Channel] = Length;
    UartRxIndex[Channel] = 0;

    USART_TypeDef* usart = Uart_GetChannel(Channel);
    if (usart == NULL) return E_NOT_OK;

    if (UartConfigPtr[Channel].Mode == UART_MODE_POLLING) {
        for (uint8 i = 0; i < Length; i++) {
            while (!(usart->SR & (1 << 5))) {} /* Chờ RXNE = 1 */
            Data[i] = (Uart_DataType)(usart->DR & 0xFF);
        }
        UartStatus[Channel] = UART_COMPLETED;
    } else if (UartConfigPtr[Channel].Mode == UART_MODE_DMA) {
        Uart_ConfigureDma(Channel, NULL, Data, 0, Length);
        DMA_Stream_TypeDef* rxStream = NULL;
        switch (Channel) {
            case UART_CHANNEL_USART1: rxStream = DMA2_Stream5; break;
            case UART_CHANNEL_USART2: rxStream = DMA1_Stream5; break;
            case UART_CHANNEL_USART3: rxStream = DMA1_Stream1; break;
        }
        rxStream->CR |= (1 << 0); /* EN = 1: Bật DMA */
    }
    return E_OK;
}

/**
 * @brief Gets the status of a specific UART channel.
 * @param Channel Channel identifier (UART_CHANNEL_USART1 to UART_CHANNEL_USART3).
 * @return Uart_StatusType Current status of the channel.
 */
Uart_StatusType Uart_GetStatus(Uart_ChannelType Channel) {
    if (Channel >= UART_CHANNEL_COUNT || UartConfigPtr == NULL) return UART_IDLE;
    return UartStatus[Channel];
}

/**
 * @brief Sets the baud rate for a specific UART channel.
 * @param Channel Channel identifier (UART_CHANNEL_USART1 to UART_CHANNEL_USART3).
 * @param Baudrate Desired baud rate.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Uart_SetBaudrate(Uart_ChannelType Channel, Uart_BaudrateType Baudrate) {
    if (Channel >= UART_CHANNEL_COUNT || Baudrate == 0 || UartConfigPtr == NULL) return E_NOT_OK;

    USART_TypeDef* usart = Uart_GetChannel(Channel);
    if (usart == NULL) return E_NOT_OK;

    usart->CR1 &= ~(1 << 13); /* UE = 0 */
    uint32 clock = (Channel == UART_CHANNEL_USART1) ? 84000000UL : 42000000UL; /* APB2: 84MHz, APB1: 42MHz */
    uint32 divider = (clock * 25) / (4 * Baudrate);
    uint16 mantissa = divider / 100;
    uint8 fraction = ((divider % 100) * 16 + 50) / 100;
    usart->BRR = (mantissa << 4) | (fraction & 0x0F);
    usart->CR1 |= (1 << 13); /* UE = 1 */
    return E_OK;
}

/* Xử lý ngắt UART */
void USART1_IRQHandler(void) {
    USART_TypeDef* usart = USART1;
    if (usart->SR & (1 << 3)) { /* ORE: Overrun error */
        usart->DR; /* Đọc DR để xóa cờ ORE */
        UartStatus[UART_CHANNEL_USART1] = UART_IDLE;
        return;
    }
    if (usart->SR & (1 << 2)) { /* PE: Parity error */
        usart->DR; /* Đọc DR để xóa cờ PE */
        UartStatus[UART_CHANNEL_USART1] = UART_IDLE;
        return;
    }
    if (usart->SR & (1 << 1)) { /* FE: Framing error */
        usart->DR; /* Đọc DR để xóa cờ FE */
        UartStatus[UART_CHANNEL_USART1] = UART_IDLE;
        return;
    }
    if (usart->SR & (1 << 5) && UartRxIndex[UART_CHANNEL_USART1] < UartRxLength[UART_CHANNEL_USART1]) {
        UartRxBuffer[UART_CHANNEL_USART1][UartRxIndex[UART_CHANNEL_USART1]++] = usart->DR;
        if (UartRxIndex[UART_CHANNEL_USART1] >= UartRxLength[UART_CHANNEL_USART1]) {
            UartStatus[UART_CHANNEL_USART1] = UART_COMPLETED;
        }
    }
    if (usart->SR & (1 << 7) && UartTxIndex[UART_CHANNEL_USART1] < UartTxLength[UART_CHANNEL_USART1]) {
        usart->DR = UartTxBuffer[UART_CHANNEL_USART1][UartTxIndex[UART_CHANNEL_USART1]++];
        if (UartTxIndex[UART_CHANNEL_USART1] >= UartTxLength[UART_CHANNEL_USART1]) {
            UartStatus[UART_CHANNEL_USART1] = UART_COMPLETED;
        }
    }
}

void USART2_IRQHandler(void) {
    USART_TypeDef* usart = USART2;
    if (usart->SR & (1 << 3)) { /* ORE: Overrun error */
        usart->DR; /* Đọc DR để xóa cờ ORE */
        UartStatus[UART_CHANNEL_USART2] = UART_IDLE;
        return;
    }
    if (usart->SR & (1 << 2)) { /* PE: Parity error */
        usart->DR; /* Đọc DR để xóa cờ PE */
        UartStatus[UART_CHANNEL_USART2] = UART_IDLE;
        return;
    }
    if (usart->SR & (1 << 1)) { /* FE: Framing error */
        usart->DR; /* Đọc DR để xóa cờ FE */
        UartStatus[UART_CHANNEL_USART2] = UART_IDLE;
        return;
    }
    if (usart->SR & (1 << 5) && UartRxIndex[UART_CHANNEL_USART2] < UartRxLength[UART_CHANNEL_USART2]) {
        UartRxBuffer[UART_CHANNEL_USART2][UartRxIndex[UART_CHANNEL_USART2]++] = usart->DR;
        if (UartRxIndex[UART_CHANNEL_USART2] >= UartRxLength[UART_CHANNEL_USART2]) {
            UartStatus[UART_CHANNEL_USART2] = UART_COMPLETED;
        }
    }
    if (usart->SR & (1 << 7) && UartTxIndex[UART_CHANNEL_USART2] < UartTxLength[UART_CHANNEL_USART2]) {
        usart->DR = UartTxBuffer[UART_CHANNEL_USART2][UartTxIndex[UART_CHANNEL_USART2]++];
        if (UartTxIndex[UART_CHANNEL_USART2] >= UartTxLength[UART_CHANNEL_USART2]) {
            UartStatus[UART_CHANNEL_USART2] = UART_COMPLETED;
        }
    }
}

void USART3_IRQHandler(void) {
    USART_TypeDef* usart = USART3;
    if (usart->SR & (1 << 3)) { /* ORE: Overrun error */
        usart->DR; /* Đọc DR để xóa cờ ORE */
        UartStatus[UART_CHANNEL_USART3] = UART_IDLE;
        return;
    }
    if (usart->SR & (1 << 2)) { /* PE: Parity error */
        usart->DR; /* Đọc DR để xóa cờ PE */
        UartStatus[UART_CHANNEL_USART3] = UART_IDLE;
        return;
    }
    if (usart->SR & (1 << 1)) { /* FE: Framing error */
        usart->DR; /* Đọc DR để xóa cờ FE */
        UartStatus[UART_CHANNEL_USART3] = UART_IDLE;
        return;
    }
    if (usart->SR & (1 << 5) && UartRxIndex[UART_CHANNEL_USART3] < UartRxLength[UART_CHANNEL_USART3]) {
        UartRxBuffer[UART_CHANNEL_USART3][UartRxIndex[UART_CHANNEL_USART3]++] = usart->DR;
        if (UartRxIndex[UART_CHANNEL_USART3] >= UartRxLength[UART_CHANNEL_USART3]) {
            UartStatus[UART_CHANNEL_USART3] = UART_COMPLETED;
        }
    }
    if (usart->SR & (1 << 7) && UartTxIndex[UART_CHANNEL_USART3] < UartTxLength[UART_CHANNEL_USART3]) {
        usart->DR = UartTxBuffer[UART_CHANNEL_USART3][UartTxIndex[UART_CHANNEL_USART3]++];
        if (UartTxIndex[UART_CHANNEL_USART3] >= UartTxLength[UART_CHANNEL_USART3]) {
            UartStatus[UART_CHANNEL_USART3] = UART_COMPLETED;
        }
    }
}

/* Xử lý ngắt DMA */
void DMA2_Stream7_IRQHandler(void) { /* USART1 TX */
    if (DMA2->HISR & DMA_HISR_TEIF7) { /* Transfer error */
        DMA2->HIFCR |= DMA_HIFCR_CTEIF7; /* Xóa cờ TE */
        DMA2_Stream7->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART1] = UART_IDLE;
        return;
    }
    if (DMA2->HISR & DMA_HISR_TCIF7) {
        DMA2->HIFCR |= DMA_HIFCR_CTCIF7; /* Xóa cờ TC */
        DMA2_Stream7->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART1] = UART_COMPLETED;
    }
}

void DMA2_Stream5_IRQHandler(void) { /* USART1 RX */
    if (DMA2->HISR & DMA_HISR_TEIF5) { /* Transfer error */
        DMA2->HIFCR |= DMA_HIFCR_CTEIF5; /* Xóa cờ TE */
        DMA2_Stream5->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART1] = UART_IDLE;
        return;
    }
    if (DMA2->HISR & DMA_HISR_TCIF5) {
        DMA2->HIFCR |= DMA_HIFCR_CTCIF5; /* Xóa cờ TC */
        DMA2_Stream5->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART1] = UART_COMPLETED;
    }
}

void DMA1_Stream6_IRQHandler(void) { /* USART2 TX */
    if (DMA1->HISR & DMA_HISR_TEIF6) { /* Transfer error */
        DMA1->HIFCR |= DMA_HIFCR_CTEIF6; /* Xóa cờ TE */
        DMA1_Stream6->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART2] = UART_IDLE;
        return;
    }
    if (DMA1->HISR & DMA_HISR_TCIF6) {
        DMA1->HIFCR |= DMA_HIFCR_CTCIF6; /* Xóa cờ TC */
        DMA1_Stream6->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART2] = UART_COMPLETED;
    }
}

void DMA1_Stream5_IRQHandler(void) { /* USART2 RX */
    if (DMA1->HISR & DMA_HISR_TEIF5) { /* Transfer error */
        DMA1->HIFCR |= DMA_HIFCR_CTEIF5; /* Xóa cờ TE */
        DMA1_Stream5->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART2] = UART_IDLE;
        return;
    }
    if (DMA1->HISR & DMA_HISR_TCIF5) {
        DMA1->HIFCR |= DMA_HIFCR_CTCIF5; /* Xóa cờ TC */
        DMA1_Stream5->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART2] = UART_COMPLETED;
    }
}

void DMA1_Stream3_IRQHandler(void) { /* USART3 TX */
    if (DMA1->LISR & DMA_LISR_TEIF3) { /* Transfer error */
        DMA1->LIFCR |= DMA_LIFCR_CTEIF3; /* Xóa cờ TE */
        DMA1_Stream3->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART3] = UART_IDLE;
        return;
    }
    if (DMA1->LISR & DMA_LISR_TCIF3) {
        DMA1->LIFCR |= DMA_LIFCR_CTCIF3; /* Xóa cờ TC */
        DMA1_Stream3->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART3] = UART_COMPLETED;
    }
}

void DMA1_Stream1_IRQHandler(void) { /* USART3 RX */
    if (DMA1->LISR & DMA_LISR_TEIF1) { /* Transfer error */
        DMA1->LIFCR |= DMA_LIFCR_CTEIF1; /* Xóa cờ TE */
        DMA1_Stream1->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART3] = UART_IDLE;
        return;
    }
    if (DMA1->LISR & DMA_LISR_TCIF1) {
        DMA1->LIFCR |= DMA_LIFCR_CTCIF1; /* Xóa cờ TC */
        DMA1_Stream1->CR &= ~(1 << 0); /* Tắt DMA */
        UartStatus[UART_CHANNEL_USART3] = UART_COMPLETED;
    }
}
