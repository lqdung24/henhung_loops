/*
 * maintask.h
 *
 *  Created on: Jul 8, 2026
 *      Author: lqdung
 */

#ifndef MAINTASK_H
#define MAINTASK_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "tm_stm32f4_mfrc522.h"
#include "stdio.h"
#include <string.h>

void StartSensorTask();
void MQ3_Task();

#endif
