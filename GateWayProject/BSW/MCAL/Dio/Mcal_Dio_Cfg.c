#include "Mcal_Dio_Cfg.h"

/* Cấu hình:
 * - GPIOA Pin 0: Output, push-pull (LED)
 * - GPIOC Pin 1: Input, pull-up (Button)
 * - GPIOB Pin 0: Output, push-pull (LED)
 * - GPIOD Pin 0: Output, push-pull (LED)
 * - GPIOE Pin 1: Input, pull-up (Button)
 */
const Dio_ConfigType DioConfig[] = {
    {DIO_PORTA, 0, DIO_OUTPUT, DIO_PUSHPULL, DIO_NO_PULL},
    {DIO_PORTC, 1, DIO_INPUT, DIO_PUSHPULL, DIO_PULL_UP},
    {DIO_PORTB, 0, DIO_OUTPUT, DIO_PUSHPULL, DIO_NO_PULL},
    {DIO_PORTD, 0, DIO_OUTPUT, DIO_PUSHPULL, DIO_NO_PULL},
    {DIO_PORTE, 1, DIO_INPUT, DIO_PUSHPULL, DIO_PULL_UP}
};

/* Cấu hình nhóm kênh */
const Dio_ChannelGroupType DioChannelGroupConfig[] = {
    {DIO_PORTA, 0, 0x0F}, /* GPIOA Pin 0-3 */
    {DIO_PORTB, 0, 0x0F}, /* GPIOB Pin 0-3 */
    {DIO_PORTF, 0, 0x0F}  /* GPIOF Pin 0-3 */
};
