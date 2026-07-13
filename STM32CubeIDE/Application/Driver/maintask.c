/*
 * maintask.c
 *
 *  Created on: Jul 8, 2026
 *      Author: lqdung
 */

#include "maintask.h"
#include "flash.h"
#include "ds1307.h"
#include "app_config.h"
#include "mq3.h"
#include "log.h"

RFID_Packet_t packet;

char buff[128];
char name[32];

extern osMessageQueueId_t btnEventQueueHandle;
extern osMessageQueueId_t rfidDataQueueHandle;
extern osMessageQueueId_t guiCommandQueueHandle;
extern osMessageQueueId_t adcDataQueueHandleHandle;
extern osMessageQueueId_t mq3QueueHandle;
extern osMessageQueueId_t mq3ResHandle;

extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c3;
extern ADC_HandleTypeDef hadc1;

extern uint16_t ADC_Buffer[ADC_BUF_LEN];
static int len;
static MQ3_Command mq3_cmd;
static MQ3_result_t mq3;
void StartSensorTask()
{
  /* USER CODE BEGIN StartSensorTask */
	ButtonEvent_t btn_evt = BTN_NONE;

	SensorState_t current_state = STATE_IDLE;
	ScreenState_t screen = SCREEN_MAIN;
	uint32_t messuring_time = 0;
	uint8_t blow_count = 0;
	UI_cmd_t ui_cmd;
	ui_cmd.cmd = CMD_NONE;

	static char voltage[16];
	static char alcohol[20];
	static char fine[20];
	static const char *measureTexts[4];

	/* Infinite loop */
	for(;;)
	{

		if (osMessageQueueGet(btnEventQueueHandle, &btn_evt, NULL, 0) != osOK) {
			btn_evt = BTN_NONE;
		}

		switch(current_state)
		{
			case STATE_IDLE:
				if (btn_evt == BTN_SHORT_PRESS && screen == SCREEN_MAIN)
				{
					if(MQ3_check_start()){
						ui_cmd.cmd = CMD_PROMPT_SCAN_CARD;
						ui_cmd.count = 0;
						osMessageQueuePut(guiCommandQueueHandle, &ui_cmd, 0, 0);
						current_state = STATE_WAIT_RFID;
					}else{
						ui_cmd.cmd = CMD_SHOW_HIGH_ALCOHOL_DIALOG;
						ui_cmd.count = 0;
						osMessageQueuePut(guiCommandQueueHandle, &ui_cmd, 0, 0);
					}
				}else if(btn_evt == BTN_LONG_PRESS){
					if(screen == SCREEN_MAIN){
						ui_cmd.cmd = CMD_GOTO_HISTORY;
						ui_cmd.count = 0;
						screen = SCREEN_HISTORY;
					}else{
						ui_cmd.cmd = CMD_GOTO_MAIN;
						ui_cmd.count = 0;
						screen = SCREEN_MAIN;
					}

					osMessageQueuePut(guiCommandQueueHandle, &ui_cmd, 0, 0);
				}
				break;

			case STATE_WAIT_RFID:
				if(TM_MFRC522_Check((uint8_t *) packet.id) == MI_OK){

					User *user = DB_Find((uint8_t *) packet.id);

					GetTime_string(&hi2c3, packet.time);

			        len = snprintf(buff, sizeof(buff),
			                       "%s %-32s  UID: %02X %02X %02X %02X %02X\r\n",
								   packet.time,
			                       user->name,
			                       user->cardID[0],
			                       user->cardID[1],
			                       user->cardID[2],
			                       user->cardID[3],
			                       user->cardID[4]);
			        memcpy(packet.name, user->name, sizeof(user->name));

			        HAL_UART_Transmit(&huart1, (uint8_t *)buff, len, HAL_MAX_DELAY);

					osMessageQueuePut(rfidDataQueueHandle, &packet, 0, 0);
					current_state = STATE_MEASURING;

					mq3_cmd = MQ3_CMD_MEASURE;
					osMessageQueuePut(mq3QueueHandle, &mq3_cmd, 0, 0);

					ui_cmd.cmd = CMD_START_MEASURE_OK;
					blow_count = 6;
					messuring_time = osKernelGetTickCount();
				}
				break;

			case STATE_MEASURING:

				if(osMessageQueueGet(mq3ResHandle, &mq3, 0, 0) == osOK){

					current_state = STATE_SHOW_RESULT;
				}else{
					uint32_t elapsed = - messuring_time + osKernelGetTickCount();
					if(elapsed > 1000 && blow_count >= 0){
						ui_cmd.cmd = CMD_START_MEASURE_OK;
						snprintf(fine, sizeof(fine), "%u sec", --blow_count);
						measureTexts[0] = fine;
						ui_cmd.texts = measureTexts;
						ui_cmd.count = 1;
						osMessageQueuePut(guiCommandQueueHandle, &ui_cmd, 0, 0);

						messuring_time = osKernelGetTickCount();
					}
				}

				break;

			case STATE_SHOW_RESULT:

				snprintf(voltage, sizeof(voltage),
				         "%u.%03u V",
				         mq3.raw_voltage_x1000 / 1000,
				         mq3.raw_voltage_x1000 % 1000);

				snprintf(alcohol, sizeof(alcohol),
				         "%u.%02u mg/100mL",
				         mq3.alcohol_x100 / 100,
				         mq3.alcohol_x100 % 100);


				snprintf(fine, sizeof(fine), "1.000.000VND");


				measureTexts[0] = voltage;
				measureTexts[1] = alcohol;
				measureTexts[2] = fine;

				ui_cmd.cmd = CMD_MEASURE_DONE;
				ui_cmd.texts = measureTexts;
				ui_cmd.count = 3;

				osMessageQueuePut(guiCommandQueueHandle, &ui_cmd, 0, 0);

				len = snprintf(name, sizeof(name),
				   "%02X %02X %02X %02X %02X",
				   packet.id[0],
				   packet.id[1],
				   packet.id[2],
				   packet.id[3],
				   packet.id[4]);

				Log_save_record(name, packet.name, packet.time, alcohol);

				len = snprintf(buff, sizeof(buff),
						"%s %s\r\n",
						voltage, ui_cmd.texts[1]);
				HAL_UART_Transmit(&huart1, (uint8_t *)buff, len, HAL_MAX_DELAY);

				current_state = STATE_IDLE;
				break;
		}

		btn_evt = BTN_NONE;

		osDelay(50);
	}
  /* USER CODE END StartSensorTask */
}


void MQ3_Task()
{
    MQ3_result_t mq3;
    uint16_t res, peak = 0;

    while (1)
    {
        osMessageQueueGet(mq3QueueHandle,
                          &mq3_cmd,
                          NULL,
                          osWaitForever);

        switch (mq3_cmd)
        {
			case MQ3_CMD_MEASURE:
				uint32_t start = osKernelGetTickCount();
				peak = 0;

				HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Buffer, ADC_BUF_LEN);

				while(osKernelGetTickCount() - start < 6000)
				{
					res = MQ3_read_nostart_ADC();
					peak = Max(res, peak);

					len = snprintf(buff, sizeof(buff), "%u ", res);
					HAL_UART_Transmit(&huart1, (uint8_t *)buff, len, HAL_MAX_DELAY);

					osDelay(200);
				}
				HAL_ADC_Stop_DMA(&hadc1);

				len = snprintf(buff, sizeof(buff), "\r\npeak: %u\r\n", peak);
				HAL_UART_Transmit(&huart1, (uint8_t *)buff, len, HAL_MAX_DELAY);

				mq3.raw_voltage_x1000 = MQ3_adc_to_voltage_x1000(peak);
				mq3.alcohol_x100 = MQ3_ADCToAlcohol_x100(peak);
				osMessageQueuePut(mq3ResHandle, &mq3, 0, 0);
				break;

			case MQ3_CMD_WAIT_READY:
				HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Buffer, ADC_BUF_LEN);

			    while(1)
			    {
			    	res = MQ3_read_nostart_ADC();
			    	float ratio = MQ3_get_ratio(res);
		        	len = snprintf(buff, sizeof(buff), "%u %u\r\n", res, (uint16_t) (ratio*1000));
					HAL_UART_Transmit(&huart1, (uint8_t *)buff, len, HAL_MAX_DELAY);

			        if(ratio > 0.9)
			        {
			        	HAL_ADC_Stop_DMA(&hadc1);
			        	mq3_cmd = MQ3_CMD_MEASURE;
						HAL_UART_Transmit(&huart1, (uint8_t *)buff, len, HAL_MAX_DELAY);
			            break;
			        }

			        osDelay(200);
			    }


			    break;
        }
    }
}
