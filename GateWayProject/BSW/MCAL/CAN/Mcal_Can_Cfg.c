/*
 * Mcal_Can_Cfg.c
 *
 *  Created on: Aug 5, 2025
 *      Author: Truong Quoc Huy
 */

#include "Mcal_Can_Cfg.h"

/* Cấu hình: CAN1, 500 kbps, standard ID, filter ID 0x100 */
const Can_ConfigType CanConfig[] = {
    {
        .HwUnit = CAN_HW_UNIT_1,
        .Baudrate = CAN_BAUDRATE_500KBPS,
        .IdType = CAN_ID_TYPE_STANDARD,
        .FilterId = 0x100
    }
};
