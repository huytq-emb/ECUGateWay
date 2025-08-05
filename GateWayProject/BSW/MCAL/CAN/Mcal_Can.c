/*
 * Mcal_Can.c
 *
 *  Created on: Aug 5, 2025
 *      Author: Truong Quoc Huy
 */

#include "Mcal_Can.h"
#include "Mcal_Can_Cfg.h"
#include "stm32f4xx.h"

/* Biến toàn cục để lưu cấu hình và trạng thái */
static const Can_ConfigType* CanConfigPtr = NULL;
static Can_StatusType CanStatus[CAN_HW_UNIT_COUNT] = {CAN_IDLE};
static Can_PduType CanRxPdu[CAN_HW_UNIT_COUNT];

/* Hàm hỗ trợ: Lấy con trỏ CAN từ HwUnit */
static CAN_TypeDef* Can_GetHwUnit(Can_HwUnitType HwUnit) {
    if (HwUnit == CAN_HW_UNIT_1) {
        return CAN1;
    }
    return NULL;
}

/* Hàm hỗ trợ: Kích hoạt clock cho CAN */
static void Can_EnableClock(Can_HwUnitType HwUnit) {
    if (HwUnit == CAN_HW_UNIT_1) {
        RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    }
}

/* Hàm hỗ trợ: Cấu hình GPIO cho CAN */
static void Can_ConfigureGpio(Can_HwUnitType HwUnit) {
    if (HwUnit == CAN_HW_UNIT_1) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
        GPIOB->MODER &= ~(0xF << (8 * 2)); /* PB8, PB9 */
        GPIOB->MODER |= (0xA << (8 * 2)); /* AF mode */
        GPIOB->AFR[1] &= ~(0xFF << ((8 - 8) * 4)); /* AF9 cho PB8, PB9 */
        GPIOB->AFR[1] |= (0x99 << ((8 - 8) * 4));
    }
}

/* Khởi tạo CAN */
void Can_Init(const Can_ConfigType* ConfigPtr) {
    if (ConfigPtr == NULL) return;

    CanConfigPtr = ConfigPtr;

    for (uint8 i = 0; i < CAN_HW_UNIT_COUNT; i++) {
        CanStatus[i] = CAN_IDLE;

        /* Kích hoạt clock và cấu hình GPIO */
        Can_EnableClock(ConfigPtr[i].HwUnit);
        Can_ConfigureGpio(ConfigPtr[i].HwUnit);

        CAN_TypeDef* can = Can_GetHwUnit(ConfigPtr[i].HwUnit);
        if (can == NULL) continue;

        /* Thoát sleep mode và vào initialization mode */
        can->MCR &= ~(CAN_MCR_SLEEP); /* Exit sleep mode */
        can->MCR |= CAN_MCR_INRQ; /* Request initialization */
        while (!(can->MSR & CAN_MSR_INAK)) {} /* Chờ INAK = 1 */

        /* Cấu hình baud rate: f_APB1 = 16 MHz */
        uint32 prescaler = 16000000UL / (ConfigPtr[i].Baudrate * 8); /* TQ = 8 */
        can->BTR = (1 << 24) | (5 << 20) | (1 << 16) | (prescaler - 1); /* BS1=5, BS2=1 */

        /* Cấu hình bộ lọc: Nhận message với ID = FilterId */
        can->FMR |= CAN_FMR_FINIT; /* Initialization mode cho bộ lọc */
        can->FA1R &= ~(1 << 0); /* Tắt filter 0 */
        can->FS1R |= (1 << 0); /* Filter 0: 32-bit */
        if (ConfigPtr[i].IdType == CAN_ID_TYPE_STANDARD) {
            can->sFilterRegister[0].FR1 = (ConfigPtr[i].FilterId << 21); /* Standard ID */
        } else {
            can->sFilterRegister[0].FR1 = (ConfigPtr[i].FilterId << 3); /* Extended ID */
        }
        can->sFilterRegister[0].FR2 = 0; /* Mask = 0: Chỉ nhận đúng ID */
        can->FM1R &= ~(1 << 0); /* Identifier mode */
        can->FA1R |= (1 << 0); /* Kích hoạt filter 0 */
        can->FMR &= ~(CAN_FMR_FINIT); /* Thoát initialization mode */

        /* Thoát initialization mode và bật interrupt */
        can->MCR &= ~(CAN_MCR_INRQ); /* Normal mode */
        while (can->MSR & CAN_MSR_INAK) {} /* Chờ INAK = 0 */
        can->IER |= CAN_IER_FMPIE0 | CAN_IER_TMEIE; /* Bật interrupt RX FIFO0 và TX */
        NVIC_EnableIRQ(CAN1_RX0_IRQn);
        NVIC_EnableIRQ(CAN1_TX_IRQn);
    }
}

/* Gửi CAN message */
Std_ReturnType Can_Write(Can_HwUnitType HwUnit, const Can_PduType* PduInfo) {
    if (HwUnit >= CAN_HW_UNIT_COUNT || PduInfo == NULL || PduInfo->Dlc > 8) return E_NOT_OK;
    if (CanStatus[HwUnit] != CAN_IDLE) return E_NOT_OK;

    CAN_TypeDef* can = Can_GetHwUnit(HwUnit);
    if (can == NULL) return E_NOT_OK;

    CanStatus[HwUnit] = CAN_BUSY;

    /* Chọn mailbox trống */
    if (can->TSR & CAN_TSR_TME0) {
        can->sTxMailBox[0].TIR = 0; /* Xóa TIR */
        if (CanConfigPtr[HwUnit].IdType == CAN_ID_TYPE_STANDARD) {
            can->sTxMailBox[0].TIR |= (PduInfo->Id << 21); /* Standard ID */
        } else {
            can->sTxMailBox[0].TIR |= (PduInfo->Id << 3) | (1 << 2); /* Extended ID */
        }
        can->sTxMailBox[0].TDTR = PduInfo->Dlc; /* DLC */
        can->sTxMailBox[0].TDLR = (PduInfo->Data[3] << 24) | (PduInfo->Data[2] << 16) |
                                  (PduInfo->Data[1] << 8) | PduInfo->Data[0];
        can->sTxMailBox[0].TDHR = (PduInfo->Data[7] << 24) | (PduInfo->Data[6] << 16) |
                                  (PduInfo->Data[5] << 8) | PduInfo->Data[4];
        can->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ; /* Yêu cầu gửi */
        return E_OK;
    }
    CanStatus[HwUnit] = CAN_IDLE;
    return E_NOT_OK;
}

/* Đọc CAN message */
Std_ReturnType Can_Read(Can_HwUnitType HwUnit, Can_PduType* PduInfo) {
    if (HwUnit >= CAN_HW_UNIT_COUNT || PduInfo == NULL) return E_NOT_OK;
    if (CanStatus[HwUnit] != CAN_COMPLETED) return E_NOT_OK;

    *PduInfo = CanRxPdu[HwUnit];
    CanStatus[HwUnit] = CAN_IDLE;
    return E_OK;
}

/* Lấy trạng thái CAN */
Can_StatusType Can_GetStatus(Can_HwUnitType HwUnit) {
    if (HwUnit >= CAN_HW_UNIT_COUNT) return CAN_IDLE;
    return CanStatus[HwUnit];
}

/* Thay đổi baud rate */
Std_ReturnType Can_SetBaudrate(Can_HwUnitType HwUnit, uint32 Baudrate) {
    CAN_TypeDef* can = Can_GetHwUnit(HwUnit);
    if (can == NULL) return E_NOT_OK;

    can->MCR |= CAN_MCR_INRQ; /* Vào initialization mode */
    while (!(can->MSR & CAN_MSR_INAK)) {} /* Chờ INAK = 1 */

    uint32 prescaler = 16000000UL / (Baudrate * 8); /* TQ = 8 */
    can->BTR = (1 << 24) | (5 << 20) | (1 << 16) | (prescaler - 1); /* BS1=5, BS2=1 */

    can->MCR &= ~(CAN_MCR_INRQ); /* Thoát initialization mode */
    while (can->MSR & CAN_MSR_INAK) {} /* Chờ INAK = 0 */
    return E_OK;
}

/* Xử lý ngắt CAN RX */
void CAN1_RX0_IRQHandler(void) {
    CAN_TypeDef* can = CAN1;
    if (can->RF0R & CAN_RF0R_FMP0) { /* Có message trong FIFO0 */
        Can_PduType* pdu = &CanRxPdu[CAN_HW_UNIT_1];
        if (can->sFIFOMailBox[0].RIR & CAN_RI0R_IDE) {
            pdu->Id = can->sFIFOMailBox[0].RIR >> 3; /* Extended ID */
        } else {
            pdu->Id = can->sFIFOMailBox[0].RIR >> 21; /* Standard ID */
        }
        pdu->Dlc = can->sFIFOMailBox[0].RDTR & 0xF;
        pdu->Data[0] = (can->sFIFOMailBox[0].RDLR >> 0) & 0xFF;
        pdu->Data[1] = (can->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
        pdu->Data[2] = (can->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
        pdu->Data[3] = (can->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
        pdu->Data[4] = (can->sFIFOMailBox[0].RDHR >> 0) & 0xFF;
        pdu->Data[5] = (can->sFIFOMailBox[0].RDHR >> 8) & 0xFF;
        pdu->Data[6] = (can->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
        pdu->Data[7] = (can->sFIFOMailBox[0].RDHR >> 24) & 0xFF;
        CanStatus[CAN_HW_UNIT_1] = CAN_COMPLETED;
        can->RF0R |= CAN_RF0R_RFOM0; /* Giải phóng FIFO0 */
    }
}

/* Xử lý ngắt CAN TX */
void CAN1_TX_IRQHandler(void) {
    CAN_TypeDef* can = CAN1;
    if (can->TSR & CAN_TSR_RQCP0) { /* Mailbox 0 hoàn tất */
        can->TSR |= CAN_TSR_RQCP0; /* Xóa cờ */
        CanStatus[CAN_HW_UNIT_1] = CAN_COMPLETED;
    }
}
