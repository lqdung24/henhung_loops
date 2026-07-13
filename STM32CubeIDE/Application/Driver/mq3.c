/*
 * mq3.c
 *
 *  Created on: Jul 13, 2026
 *      Author: lqdung
 */

#include "mq3.h"
#include "app_config.h"

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "math.h"

extern osMessageQueueId_t mq3QueueHandle;
extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;
static uint16_t avg;
extern uint16_t ADC_Buffer[ADC_BUF_LEN];

uint16_t MQ3_read_nostart_ADC(){
	if(osMessageQueueGet(mq3QueueHandle, &avg, NULL, osWaitForever) == osOK){
		return avg*ADC_SCALE;
	}
	return 0;
}


uint16_t MQ3_read(){
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Buffer, ADC_BUF_LEN);
	if(osMessageQueueGet(mq3QueueHandle, &avg, NULL, osWaitForever) == osOK){
		HAL_ADC_Stop_DMA(&hadc1);
		return avg;
	}

	HAL_ADC_Stop_DMA(&hadc1);
	return 0;
}

float MQ3_get_ratio(uint16_t raw_adc){
	float v_mq3 = ((float)raw_adc * 3) / 4095.0f;

	if (v_mq3 <= 0.01f)
		return 0;

	if (v_mq3 >= (MQ3_VC - 0.01f))
		v_mq3 = MQ3_VC - 0.01f;

	float ratio = (MQ3_VC/v_mq3 - 1)/(MQ3_VC/(400*3.0/4095.0*1.5)-1);

	return ratio;
}

uint16_t MQ3_ADCToAlcohol_x100(uint16_t raw_adc)
{
    float ratio = MQ3_get_ratio(raw_adc);

    float ppm = powf(ratio / 0.4f, -1.43f);

    float mg100ml = ppm * 0.08f;

    if (mg100ml < 0.0f)
        mg100ml = 0.0f;

    return (uint16_t)(mg100ml * 100.0f + 0.5f);
}

uint16_t MQ3_adc_to_voltage_x1000(uint16_t adc_raw)
{
    return (uint16_t)((adc_raw * 3000UL) / 4095);
}

uint8_t MQ3_check_start(){
	uint16_t res = MQ3_read();

	return (uint8_t) (res < 1500);
}


