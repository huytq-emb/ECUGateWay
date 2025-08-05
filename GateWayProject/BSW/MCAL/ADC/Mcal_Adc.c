#include "Mcal_Adc.h"
#include "Mcal_Adc_Cfg.h"
#include "Mcal_Adc_Hw.h"

/* Biến toàn cục để lưu cấu hình và trạng thái */
static const Adc_ConfigType* AdcConfigPtr = NULL;
static Adc_StatusType AdcGroupStatus[ADC_GROUP_COUNT] = {ADC_IDLE};
static Adc_ValueGroupType* AdcResultBuffer[ADC_GROUP_COUNT] = {NULL};
static uint8 AdcStreamSampleCount[ADC_GROUP_COUNT] = {0}; /* Đếm số mẫu trong streaming mode */
static uint8 clockEnabled[3] = {0}; /* Hỗ trợ ADC1, ADC2, ADC3 */

/* Hàm hỗ trợ: Lấy con trỏ ADC từ HwUnit */
static ADC_TypeDef* Adc_GetHwUnit(Adc_HwUnitType HwUnit) {
    if (HwUnit >= ADC_HW_UNIT_1 && HwUnit <= ADC_HW_UNIT_3) {
        return adcMap[HwUnit];
    }
    return NULL;
}

/* Hàm hỗ trợ: Kích hoạt clock cho ADC */
static void Adc_EnableClock(Adc_HwUnitType HwUnit) {
    if (HwUnit >= ADC_HW_UNIT_1 && HwUnit <= ADC_HW_UNIT_3 && !clockEnabled[HwUnit - ADC_HW_UNIT_1]) {
        clockEnabled[HwUnit - ADC_HW_UNIT_1] = 1;
        switch (HwUnit) {
            case ADC_HW_UNIT_1: RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; break;
            case ADC_HW_UNIT_2: RCC->APB2ENR |= RCC_APB2ENR_ADC2EN; break;
            case ADC_HW_UNIT_3: RCC->APB2ENR |= RCC_APB2ENR_ADC3EN; break;
            default: break;
        }
    }
}

/* Hàm hỗ trợ: Cấu hình TIM2 cho hardware trigger */
static void Adc_ConfigureTimer2(uint16 TimerPsc, uint16 TimerArr) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; /* Bật clock cho TIM2 */
    TIM2->CR1 = 0; /* Reset CR1 */
    TIM2->CR2 |= (0x2 << 4); /* MMS = 010: Update event làm trigger */
    TIM2->PSC = TimerPsc; /* Prescaler */
    TIM2->ARR = TimerArr; /* Auto-reload */
    TIM2->CR1 |= (1 << 0); /* CEN = 1: Bật timer */
}

/* Hàm hỗ trợ: Cấu hình DMA cho scan mode */
static void Adc_ConfigureDma(Adc_GroupType Group, ADC_TypeDef* adc) {
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN; /* Bật clock cho DMA2 */
    DMA2_Stream0->CR = 0; /* Reset CR */
    DMA2_Stream0->PAR = (uint32_t)&adc->DR; /* Peripheral address */
    DMA2_Stream0->M0AR = (uint32_t)AdcResultBuffer[Group]; /* Memory address */
    DMA2_Stream0->NDTR = AdcConfigPtr[Group].NumChannels * AdcConfigPtr[Group].StreamNumSamples; /* Số lượng dữ liệu */
    DMA2_Stream0->CR |= (0x0 << 6); /* PL = 00: Low priority */
    DMA2_Stream0->CR |= (0x1 << 10); /* MSIZE = 01: 16-bit */
    DMA2_Stream0->CR |= (0x1 << 8); /* PSIZE = 01: 16-bit */
    DMA2_Stream0->CR |= (1 << 11); /* MINC = 1: Increment memory */
    DMA2_Stream0->CR |= (1 << 25); /* CIRC = 1: Circular mode */
    DMA2_Stream0->CR |= (1 << 0); /* EN = 1: Bật DMA */
}

/**
 * @brief Initializes the ADC module with the provided configuration.
 * @param ConfigPtr Pointer to the ADC configuration structure.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_Init(const Adc_ConfigType* ConfigPtr) {
    if (ConfigPtr == NULL) return E_NOT_OK;

    AdcConfigPtr = ConfigPtr;

    for (uint8 i = 0; i < ADC_GROUP_COUNT; i++) {
        /* Kiểm tra tính hợp lệ của cấu hình */
        if (ConfigPtr[i].NumChannels == 0 || ConfigPtr[i].NumChannels > 16 ||
            ConfigPtr[i].Channels == NULL || ConfigPtr[i].ResultBuffer == NULL ||
            Adc_GetHwUnit(ConfigPtr[i].HwUnit) == NULL ||
            ConfigPtr[i].AccessMode > ADC_ACCESS_MODE_STREAMING ||
            ConfigPtr[i].ConvMode > ADC_CONV_MODE_CONTINUOUS ||
            ConfigPtr[i].TriggerSource > ADC_TRIG_SRC_TIM2 ||
            (ConfigPtr[i].AccessMode == ADC_ACCESS_MODE_STREAMING && ConfigPtr[i].StreamNumSamples == 0)) {
            return E_NOT_OK;
        }

        /* Kiểm tra trùng lặp kênh */
        for (uint8 j = 0; j < ConfigPtr[i].NumChannels; j++) {
            for (uint8 k = j + 1; k < ConfigPtr[i].NumChannels; k++) {
                if (ConfigPtr[i].Channels[j] == ConfigPtr[i].Channels[k]) {
                    return E_NOT_OK; /* Kênh trùng lặp */
                }
            }
        }

        AdcGroupStatus[i] = ADC_IDLE;
        AdcResultBuffer[i] = NULL;
        AdcStreamSampleCount[i] = 0;

        /* Kích hoạt clock và cấu hình ADC */
        Adc_EnableClock(ConfigPtr[i].HwUnit);
        ADC_TypeDef* adc = Adc_GetHwUnit(ConfigPtr[i].HwUnit);

        /* Cấu hình ADC: 12-bit resolution */
        adc->CR1 = 0; /* Reset CR1 */
        adc->CR1 |= (0 << 8); /* RES = 00: 12-bit */
        if (ConfigPtr[i].NumChannels > 1) {
            adc->CR1 |= (1 << 8); /* SCAN = 1: Bật scan mode */
            adc->CR2 |= (1 << 10); /* DMA = 1: Bật DMA */
            Adc_ConfigureDma(i, adc); /* Cấu hình DMA cho scan mode */
        }
        adc->CR2 = 0; /* Reset CR2 */
        adc->CR2 |= (1 << 1); /* ADON = 1: Bật ADC */
        if (ConfigPtr[i].ConvMode == ADC_CONV_MODE_CONTINUOUS) {
            adc->CR2 |= (1 << 0); /* CONT = 1: Continuous mode */
        }
        if (ConfigPtr[i].TriggerSource == ADC_TRIG_SRC_TIM2) {
            adc->CR2 |= (1 << 8); /* EXTEN = 01: Rising edge trigger */
            adc->CR2 |= (0x7 << 24); /* EXTSEL = 0111: TIM2 TRGO */
            Adc_ConfigureTimer2(ConfigPtr[i].TimerPsc, ConfigPtr[i].TimerArr);
        }

        /* Cấu hình kênh cho nhóm */
        for (uint8 j = 0; j < ConfigPtr[i].NumChannels; j++) {
            if (j < 6) {
                adc->SQR3 |= (ConfigPtr[i].Channels[j] << (j * 5)); /* Kênh 0-5 trong SQR3 */
            } else if (j < 12) {
                adc->SQR2 |= (ConfigPtr[i].Channels[j] << ((j - 6) * 5)); /* Kênh 6-11 trong SQR2 */
            } else {
                adc->SQR1 |= (ConfigPtr[i].Channels[j] << ((j - 12) * 5)); /* Kênh 12-15 trong SQR1 */
            }
        }
        adc->SQR1 |= ((ConfigPtr[i].NumChannels - 1) << 20); /* Đặt số lượng kênh */
    }
    return E_OK;
}

/**
 * @brief Sets up the result buffer for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @param DataBufferPtr Pointer to the result buffer.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr) {
    if (Group >= ADC_GROUP_COUNT || DataBufferPtr == NULL || AdcConfigPtr == NULL) {
        return E_NOT_OK;
    }
    if (AdcResultBuffer[Group] != NULL) {
        return E_NOT_OK; /* Bộ đệm đã được thiết lập */
    }
    AdcResultBuffer[Group] = DataBufferPtr;
    return E_OK;
}

/**
 * @brief Starts conversion for a specific ADC group (software trigger only).
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return None
 */
void Adc_StartGroupConversion(Adc_GroupType Group) {
    if (Group >= ADC_GROUP_COUNT || AdcConfigPtr == NULL) return;

    ADC_TypeDef* adc = Adc_GetHwUnit(AdcConfigPtr[Group].HwUnit);
    if (adc == NULL || AdcGroupStatus[Group] != ADC_IDLE || AdcConfigPtr[Group].TriggerSource != ADC_TRIG_SRC_SW) return;

    AdcGroupStatus[Group] = ADC_BUSY;
    AdcStreamSampleCount[Group] = 0;
    adc->CR2 |= (1 << 30); /* SWSTART: Bắt đầu chuyển đổi */
}

/**
 * @brief Stops conversion for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return None
 */
void Adc_StopGroupConversion(Adc_GroupType Group) {
    if (Group >= ADC_GROUP_COUNT || AdcConfigPtr == NULL) return;

    ADC_TypeDef* adc = Adc_GetHwUnit(AdcConfigPtr[Group].HwUnit);
    if (adc == NULL) return;

    adc->CR2 &= ~(1 << 30); /* Dừng software trigger */
    adc->CR2 &= ~(1 << 0); /* Tắt continuous mode */
    AdcGroupStatus[Group] = ADC_IDLE;
    AdcStreamSampleCount[Group] = 0;
}

/**
 * @brief Reads the conversion result for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @param DataBufferPtr Pointer to the buffer to store results.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr) {
    if (Group >= ADC_GROUP_COUNT || AdcConfigPtr == NULL || DataBufferPtr == NULL) return E_NOT_OK;

    ADC_TypeDef* adc = Adc_GetHwUnit(AdcConfigPtr[Group].HwUnit);
    if (adc == NULL || AdcGroupStatus[Group] != ADC_COMPLETED) return E_NOT_OK;

    if (AdcConfigPtr[Group].NumChannels == 1 || AdcConfigPtr[Group].AccessMode != ADC_ACCESS_MODE_STREAMING) {
        for (uint8 i = 0; i < AdcConfigPtr[Group].NumChannels; i++) {
            DataBufferPtr[i] = adc->DR; /* Đọc dữ liệu từ DR */
        }
    } else {
        /* Dữ liệu đã được DMA truyền vào AdcResultBuffer */
        for (uint8 i = 0; i < AdcConfigPtr[Group].NumChannels; i++) {
            DataBufferPtr[i] = AdcResultBuffer[Group][i];
        }
    }

    AdcStreamSampleCount[Group]++;
    if (AdcConfigPtr[Group].AccessMode == ADC_ACCESS_MODE_STREAMING &&
        AdcStreamSampleCount[Group] >= AdcConfigPtr[Group].StreamNumSamples) {
        AdcGroupStatus[Group] = ADC_STREAM_COMPLETED;
    } else if (AdcConfigPtr[Group].ConvMode != ADC_CONV_MODE_CONTINUOUS &&
               AdcConfigPtr[Group].TriggerSource != ADC_TRIG_SRC_TIM2) {
        AdcGroupStatus[Group] = ADC_IDLE;
    }
    return E_OK;
}

/**
 * @brief Gets the status of a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return Adc_StatusType Current status of the group.
 */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group) {
    if (Group >= ADC_GROUP_COUNT || AdcConfigPtr == NULL) return ADC_IDLE;

    ADC_TypeDef* adc = Adc_GetHwUnit(AdcConfigPtr[Group].HwUnit);
    if (adc == NULL) return ADC_IDLE;

    if (AdcGroupStatus[Group] == ADC_BUSY) {
        if (adc->SR & (1 << 5)) { /* OVR: Overrun */
            AdcGroupStatus[Group] = ADC_IDLE;
            AdcStreamSampleCount[Group] = 0;
            return ADC_IDLE;
        }
        if (adc->SR & (1 << 4)) { /* EOC: End of Conversion */
            AdcGroupStatus[Group] = ADC_COMPLETED;
        }
    }
    return AdcGroupStatus[Group];
}

/**
 * @brief Enables hardware trigger for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_EnableHardwareTrigger(Adc_GroupType Group) {
    if (Group >= ADC_GROUP_COUNT || AdcConfigPtr == NULL) return E_NOT_OK;

    ADC_TypeDef* adc = Adc_GetHwUnit(AdcConfigPtr[Group].HwUnit);
    if (adc == NULL || AdcConfigPtr[Group].TriggerSource != ADC_TRIG_SRC_TIM2) return E_NOT_OK;

    AdcGroupStatus[Group] = ADC_BUSY;
    AdcStreamSampleCount[Group] = 0;
    adc->CR2 |= (1 << 8); /* EXTEN = 01: Rising edge trigger */
    adc->CR2 |= (0x7 << 24); /* EXTSEL = 0111: TIM2 TRGO */
    return E_OK;
}

/**
 * @brief Disables hardware trigger for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_DisableHardwareTrigger(Adc_GroupType Group) {
    if (Group >= ADC_GROUP_COUNT || AdcConfigPtr == NULL) return E_NOT_OK;

    ADC_TypeDef* adc = Adc_GetHwUnit(AdcConfigPtr[Group].HwUnit);
    if (adc == NULL) return E_NOT_OK;

    adc->CR2 &= ~(3 << 8); /* EXTEN = 00: Tắt trigger */
    AdcGroupStatus[Group] = ADC_IDLE;
    AdcStreamSampleCount[Group] = 0;
    return E_OK;
}
