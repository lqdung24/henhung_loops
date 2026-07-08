#ifndef CALLBACK_H
#define CALLBACK_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

typedef enum {
    BTN_NONE = 0,
    BTN_SHORT_PRESS,
    BTN_LONG_PRESS
} ButtonEvent_t;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void tim3_callback();
#endif /* DS1307_H */
