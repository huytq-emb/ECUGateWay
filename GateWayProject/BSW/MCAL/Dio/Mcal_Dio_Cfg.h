#ifndef MCAL_DIO_CFG_H
#define MCAL_DIO_CFG_H

#include "Mcal_Dio.h"

/* Cấu hình số lượng kênh */
#define DIO_CHANNEL_COUNT 5u
#define DIO_CHANNEL_GROUP_COUNT 3u

extern const Dio_ConfigType DioConfig[];
extern const Dio_ChannelGroupType DioChannelGroupConfig[];

#endif
