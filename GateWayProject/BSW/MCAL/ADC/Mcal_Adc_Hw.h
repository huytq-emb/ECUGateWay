#ifndef MCAL_ADC_HW_H
#define MCAL_ADC_HW_H

#include "stm32f4xx.h"

/* Ánh xạ đơn vị ADC */
static ADC_TypeDef* const adcMap[] = {
    [ADC_HW_UNIT_1] = ADC1,
    [ADC_HW_UNIT_2] = ADC2,
    [ADC_HW_UNIT_3] = ADC3
};

/* Kích hoạt clock cho ADC */
static void Adc_EnableClock(Adc_HwUnitType HwUnit);

/* Cấu hình TIM2 cho hardware trigger */
static void Adc_ConfigureTimer2(uint16 TimerPsc, uint16 TimerArr);

#endif
