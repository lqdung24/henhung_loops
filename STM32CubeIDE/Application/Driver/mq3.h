/*
 * mq3.h
 *
 *  Created on: Jul 13, 2026
 *      Author: lqdung
 */

#ifndef MQ3_H
#define MQ3_h
#include "stm32f4xx_hal.h"

uint16_t MQ3_read_nostart_ADC();
uint16_t MQ3_read();
uint16_t MQ3_ADCToAlcohol_x100(uint16_t raw_adc);
uint16_t MQ3_adc_to_voltage_x1000(uint16_t adc_raw);
float MQ3_get_ratio(uint16_t raw_adc);
uint8_t MQ3_check_start();

#endif


