#ifndef MCAL_ADC_H
#define MCAL_ADC_H

#include "Std_Types.h"

/* Định nghĩa kiểu dữ liệu theo AUTOSAR */
typedef uint8 Adc_GroupType;
typedef uint8 Adc_ChannelType;
typedef uint16 Adc_ValueGroupType;
typedef uint8 Adc_HwUnitType;
typedef uint8 Adc_StreamNumSampleType;

typedef enum {
    ADC_IDLE,
    ADC_BUSY,
    ADC_COMPLETED,
    ADC_STREAM_COMPLETED
} Adc_StatusType;

typedef enum {
    ADC_ACCESS_MODE_SINGLE,
    ADC_ACCESS_MODE_STREAMING
} Adc_GroupAccessModeType;

typedef enum {
    ADC_CONV_MODE_ONESHOT,
    ADC_CONV_MODE_CONTINUOUS
} Adc_GroupConvModeType;

typedef enum {
    ADC_TRIG_SRC_SW,
    ADC_TRIG_SRC_TIM2
} Adc_TriggerSourceType;

typedef struct {
    Adc_HwUnitType HwUnit;
    Adc_GroupType GroupId;
    Adc_ChannelType* Channels;
    uint8 NumChannels;
    Adc_GroupAccessModeType AccessMode;
    Adc_GroupConvModeType ConvMode;
    Adc_TriggerSourceType TriggerSource;
    Adc_ValueGroupType* ResultBuffer;
    Adc_StreamNumSampleType StreamNumSamples; /* Số mẫu cho streaming mode */
    uint16 TimerPsc; /* Prescaler cho TIM2 */
    uint16 TimerArr; /* Auto-reload cho TIM2 */
} Adc_ConfigType;

/* Định nghĩa đơn vị phần cứng, kênh, và nhóm */
#define ADC_HW_UNIT_1 0x01
#define ADC_HW_UNIT_2 0x02
#define ADC_HW_UNIT_3 0x03
#define ADC_CHANNEL_0 0x00
#define ADC_CHANNEL_1 0x01
#define ADC_CHANNEL_2 0x02
#define ADC_CHANNEL_3 0x03
#define ADC_CHANNEL_4 0x04
#define ADC_CHANNEL_5 0x05
#define ADC_GROUP_0 0x00
#define ADC_GROUP_1 0x01
#define ADC_GROUP_2 0x02

/* API theo chuẩn AUTOSAR */
/**
 * @brief Initializes the ADC module with the provided configuration.
 * @param ConfigPtr Pointer to the ADC configuration structure.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_Init(const Adc_ConfigType* ConfigPtr);

/**
 * @brief Sets up the result buffer for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @param DataBufferPtr Pointer to the result buffer.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);

/**
 * @brief Starts conversion for a specific ADC group (software trigger only).
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return None
 */
void Adc_StartGroupConversion(Adc_GroupType Group);

/**
 * @brief Stops conversion for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return None
 */
void Adc_StopGroupConversion(Adc_GroupType Group);

/**
 * @brief Reads the conversion result for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @param DataBufferPtr Pointer to the buffer to store results.
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);

/**
 * @brief Gets the status of a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return Adc_StatusType Current status of the group.
 */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/**
 * @brief Enables hardware trigger for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_EnableHardwareTrigger(Adc_GroupType Group);

/**
 * @brief Disables hardware trigger for a specific ADC group.
 * @param Group Group identifier (ADC_GROUP_0 to ADC_GROUP_2).
 * @return Std_ReturnType E_OK if successful, E_NOT_OK otherwise.
 */
Std_ReturnType Adc_DisableHardwareTrigger(Adc_GroupType Group);

#endif
