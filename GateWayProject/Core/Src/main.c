#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Mcal_Dio.h"
#include "Mcal_Dio_Cfg.h"
#include "Mcal_Adc.h"
#include "Mcal_Adc_Cfg.h"
#include "Mcal_Uart.h"
#include "Mcal_Uart_Cfg.h"
#include "Mcal_Can.h"
#include "Mcal_Can_Cfg.h"

void AdcCanUartTask(void *pvParameters) {
    Adc_ValueGroupType dataBuffer0[2]; /* ADC1 Group 0 */
    Can_PduType canTxPdu, canRxPdu;
    Uart_DataType uartBuffer[10];

    /* Thiết lập bộ đệm ADC */
    Adc_SetupResultBuffer(ADC_GROUP_0, dataBuffer0);

    while (1) {
        /* Bắt đầu chuyển đổi ADC */
        Adc_StartGroupConversion(ADC_GROUP_0);
        while (Adc_GetGroupStatus(ADC_GROUP_0) != ADC_COMPLETED) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        /* Đọc và gửi dữ liệu ADC qua CAN */
        if (Adc_ReadGroup(ADC_GROUP_0, dataBuffer0) == E_OK) {
            canTxPdu.Id = 0x100;
            canTxPdu.Dlc = 2;
            canTxPdu.Data[0] = (dataBuffer0[0] >> 8) & 0xFF; /* Byte cao */
            canTxPdu.Data[1] = dataBuffer0[0] & 0xFF; /* Byte thấp */
            if (Can_Write(CAN_HW_UNIT_1, &canTxPdu) == E_OK) {
                while (Can_GetStatus(CAN_HW_UNIT_1) != CAN_COMPLETED) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }

            /* Điều khiển LED */
            if (dataBuffer0[0] > 2048) {
                Dio_WriteChannel(DIO_CHANNEL_0, STD_HIGH, DIO_PORTA);
            } else {
                Dio_WriteChannel(DIO_CHANNEL_0, STD_LOW, DIO_PORTA);
            }
        }

        /* Đọc dữ liệu từ CAN và gửi qua UART */
        if (Can_GetStatus(CAN_HW_UNIT_1) == CAN_COMPLETED) {
            if (Can_Read(CAN_HW_UNIT_1, &canRxPdu) == E_OK) {
                uartBuffer[0] = 'C'; uartBuffer[1] = 'A'; uartBuffer[2] = 'N';
                uartBuffer[3] = ':';
                uartBuffer[4] = canRxPdu.Data[0]; /* Byte cao */
                uartBuffer[5] = canRxPdu.Data[1]; /* Byte thấp */
                uartBuffer[6] = '\r'; uartBuffer[7] = '\n';
                if (Uart_Write(UART_CHANNEL_USART2, uartBuffer, 8) == E_OK) {
                    while (Uart_GetStatus(UART_CHANNEL_USART2) != UART_COMPLETED) {
                        vTaskDelay(pdMS_TO_TICKS(10));
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void) {
    Dio_Init(DioConfig); /* Khởi tạo DIO */
    Adc_Init(AdcConfig); /* Khởi tạo ADC */
    Uart_Init(UartConfig); /* Khởi tạo UART */
    Can_Init(CanConfig); /* Khởi tạo CAN */

    xTaskCreate(AdcCanUartTask, "AdcCanUartTask", 128, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1) {
    }
}
