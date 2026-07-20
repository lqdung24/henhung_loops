#ifndef DS1307_H
#define DS1307_H

#include "stm32f4xx_hal.h"

#define DS1307_ADDRESS    (0x68 << 1)

typedef struct
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} Time;

HAL_StatusTypeDef SetTime(I2C_HandleTypeDef *hi2c, Time *time);
HAL_StatusTypeDef GetTime(I2C_HandleTypeDef *hi2c, Time *time);

void GetTime_string(I2C_HandleTypeDef *hi2c, char *out);

void RTC_SetTime(uint8_t year,
                 uint8_t month,
                 uint8_t day,
                 uint8_t weekday,
                 uint8_t hour,
                 uint8_t min,
                 uint8_t sec, I2C_HandleTypeDef *hi2c);

#endif /* DS1307_H */
