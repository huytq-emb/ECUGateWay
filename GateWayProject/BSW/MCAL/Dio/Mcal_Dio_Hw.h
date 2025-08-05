/*
 * Mcal_Dio_Hw.h
 *
 *  Created on: Aug 5, 2025
 *      Author: Truong Quoc Huy
 */

#ifndef MCAL_DIO_HW_H
#define MCAL_DIO_HW_H

#include "stm32f4xx.h"

/* Ánh xạ cổng GPIO */
static GPIO_TypeDef* const portMap[] = {
    [DIO_PORTA] = GPIOA,
    [DIO_PORTB] = GPIOB,
    [DIO_PORTC] = GPIOC,
    [DIO_PORTD] = GPIOD,
    [DIO_PORTE] = GPIOE,
    [DIO_PORTF] = GPIOF,
    [DIO_PORTG] = GPIOG,
    [DIO_PORTH] = GPIOH,
    [DIO_PORTI] = GPIOI
};

/* Kích hoạt clock cho cổng */
static void Dio_EnableClock(Dio_PortType PortId);

#endif
