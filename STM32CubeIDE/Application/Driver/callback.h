#ifndef CALLBACK_H
#define CALLBACK_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void tim3_callback();
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
#endif /* DS1307_H */
