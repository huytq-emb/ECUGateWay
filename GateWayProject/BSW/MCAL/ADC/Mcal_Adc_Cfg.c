#include "Mcal_Adc_Cfg.h"

/* Cấu hình:
 * - Group 0: ADC1, Channel 0, 1 (GPIOA Pin 0, 1), single conversion, software trigger
 * - Group 1: ADC2, Channel 2, 3 (GPIOA Pin 2, 3), continuous mode, software trigger
 * - Group 2: ADC3, Channel 4, 5 (GPIOC Pin 4, 5), scan mode, TIM2 trigger
 */
static Adc_ChannelType AdcGroup0Channels[] = {ADC_CHANNEL_0, ADC_CHANNEL_1};
static Adc_ChannelType AdcGroup1Channels[] = {ADC_CHANNEL_2, ADC_CHANNEL_3};
static Adc_ChannelType AdcGroup2Channels[] = {ADC_CHANNEL_4, ADC_CHANNEL_5};
static Adc_ValueGroupType AdcGroup0Buffer[2];
static Adc_ValueGroupType AdcGroup1Buffer[2];
static Adc_ValueGroupType AdcGroup2Buffer[2];

const Adc_ConfigType AdcConfig[] = {
    {
        .HwUnit = ADC_HW_UNIT_1,
        .GroupId = ADC_GROUP_0,
        .Channels = AdcGroup0Channels,
        .NumChannels = 2,
        .AccessMode = ADC_ACCESS_MODE_SINGLE,
        .ConvMode = ADC_CONV_MODE_ONESHOT,
        .TriggerSource = ADC_TRIG_SRC_SW,
        .ResultBuffer = AdcGroup0Buffer,
        .StreamNumSamples = 1,
        .TimerPsc = 0, /* Không dùng cho software trigger */
        .TimerArr = 0  /* Không dùng cho software trigger */
    },
    {
        .HwUnit = ADC_HW_UNIT_2,
        .GroupId = ADC_GROUP_1,
        .Channels = AdcGroup1Channels,
        .NumChannels = 2,
        .AccessMode = ADC_ACCESS_MODE_SINGLE,
        .ConvMode = ADC_CONV_MODE_CONTINUOUS,
        .TriggerSource = ADC_TRIG_SRC_SW,
        .ResultBuffer = AdcGroup1Buffer,
        .StreamNumSamples = 1,
        .TimerPsc = 0, /* Không dùng cho software trigger */
        .TimerArr = 0  /* Không dùng cho software trigger */
    },
    {
        .HwUnit = ADC_HW_UNIT_3,
        .GroupId = ADC_GROUP_2,
        .Channels = AdcGroup2Channels,
        .NumChannels = 2,
        .AccessMode = ADC_ACCESS_MODE_STREAMING,
        .ConvMode = ADC_CONV_MODE_ONESHOT,
        .TriggerSource = ADC_TRIG_SRC_TIM2,
        .ResultBuffer = AdcGroup2Buffer,
        .StreamNumSamples = 4, /* Streaming mode: 4 mẫu */
        .TimerPsc = 15999, /* Prescaler: 16MHz / (15999 + 1) = 1kHz */
        .TimerArr = 999   /* Auto-reload: 1kHz / (999 + 1) = 1Hz */
    }
};
