#include "ds1307.h"

#include "stdio.h"

static uint8_t DecToBCD(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t BCDToDec(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

HAL_StatusTypeDef SetTime(I2C_HandleTypeDef *hi2c, Time *time)
{
    uint8_t data[7];

    data[0] = DecToBCD(time->sec);
    data[1] = DecToBCD(time->min);
    data[2] = DecToBCD(time->hour);
    data[3] = DecToBCD(time->weekday);
    data[4] = DecToBCD(time->day);
    data[5] = DecToBCD(time->month);
    data[6] = DecToBCD(time->year);

    return HAL_I2C_Mem_Write(hi2c,
                             DS1307_ADDRESS,
                             0x00,
                             I2C_MEMADD_SIZE_8BIT,
                             data,
                             7,
                             HAL_MAX_DELAY);
}

HAL_StatusTypeDef GetTime(I2C_HandleTypeDef *hi2c, Time *time)
{
    uint8_t data[7];

    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(hi2c,
                              DS1307_ADDRESS,
                              0x00,
                              I2C_MEMADD_SIZE_8BIT,
                              data,
                              7,
                              HAL_MAX_DELAY);

    if (status != HAL_OK)
        return status;

    time->sec     = BCDToDec(data[0] & 0x7F);
    time->min     = BCDToDec(data[1]);
    time->hour    = BCDToDec(data[2] & 0x3F);
    time->weekday = BCDToDec(data[3]);
    time->day     = BCDToDec(data[4]);
    time->month   = BCDToDec(data[5]);
    time->year    = BCDToDec(data[6]);

    return HAL_OK;
}

void GetTime_string(I2C_HandleTypeDef *hi2c, char *out)
{
    Time time;

    if (GetTime(hi2c, &time) != HAL_OK)
    {
        sprintf(out, "--:-- --/--/----");
        return;
    }

    sprintf(out,
            "%02d:%02d %02d/%02d/20%02d",
            time.hour,
            time.min,
            time.day,
            time.month,
            time.year);
}
/**
 * @brief Set the current date and time of the DS1307 RTC.
 *
 * @param year     Year (0-99), e.g. 26 for 2026.
 * @param month    Month (1-12).
 * @param day      Day of month (1-31).
 * @param weekday  Day of week (1-7, 1 = Sunday).
 * @param hour     Hour (0-23).
 * @param min      Minute (0-59).
 * @param sec      Second (0-59).
 */
void RTC_SetTime(uint8_t year,
                 uint8_t month,
                 uint8_t day,
                 uint8_t weekday,
                 uint8_t hour,
                 uint8_t min,
                 uint8_t sec, I2C_HandleTypeDef *hi2c)
{
    Time time =
    {
        .sec     = sec,
        .min     = min,
        .hour    = hour,
        .weekday = weekday,
        .day     = day,
        .month   = month,
        .year    = year
    };

    SetTime(hi2c, &time);
}

