/*
 * Mcal_Can.h
 *
 *  Created on: Aug 5, 2025
 *      Author: Truong Quoc Huy
 */

#ifndef MCAL_CAN_H
#define MCAL_CAN_H

#include "Std_Types.h"

/* Định nghĩa kiểu dữ liệu theo AUTOSAR */
typedef uint8 Can_HwUnitType;
typedef uint32 Can_IdType;
typedef uint8 Can_DataType;

typedef enum {
    CAN_IDLE,
    CAN_BUSY,
    CAN_COMPLETED
} Can_StatusType;

typedef enum {
    CAN_ID_TYPE_STANDARD, /* 11-bit */
    CAN_ID_TYPE_EXTENDED  /* 29-bit */
} Can_IdTypeEnum;

typedef struct {
    Can_HwUnitType HwUnit;
    uint32 Baudrate;
    Can_IdTypeEnum IdType;
    uint16 FilterId; /* ID để lọc message */
} Can_ConfigType;

typedef struct {
    Can_IdType Id;
    uint8 Dlc; /* Data Length Code: 0-8 */
    Can_DataType Data[8];
} Can_PduType;

/* Định nghĩa đơn vị phần cứng và hằng số */
#define CAN_HW_UNIT_1 0x01
#define CAN_BAUDRATE_500KBPS 500000

/* API theo chuẩn AUTOSAR */
void Can_Init(const Can_ConfigType* ConfigPtr);
Std_ReturnType Can_Write(Can_HwUnitType HwUnit, const Can_PduType* PduInfo);
Std_ReturnType Can_Read(Can_HwUnitType HwUnit, Can_PduType* PduInfo);
Can_StatusType Can_GetStatus(Can_HwUnitType HwUnit);
Std_ReturnType Can_SetBaudrate(Can_HwUnitType HwUnit, uint32 Baudrate);

#endif
