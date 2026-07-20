#include "callback.h"
#include "app_config.h"

volatile uint32_t last_irq_time = 0;
extern TIM_HandleTypeDef htim3;


extern osMessageQueueId_t btnEventQueueHandle;
extern osMessageQueueId_t mq3QueueHandle;
extern uint16_t ADC_Buffer[ADC_BUF_LEN];
ButtonEvent_t event = BTN_NONE;


static uint32_t press_tick = 0;
static uint8_t button_pressed = 0;
static uint8_t long_long_press_sent = 0;
volatile uint8_t long_press_sent = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin != GPIO_PIN_0)
        return;

    uint32_t now = HAL_GetTick();

    if (now - last_irq_time < 50)
        return;

    last_irq_time = now;

    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
    {
        // Button pressed
        button_pressed = 1;
        long_long_press_sent = 0;
        press_tick = now;

        __HAL_TIM_SET_COUNTER(&htim3, 0);
        HAL_TIM_Base_Start_IT(&htim3);
    }
    else
    {
        // Button released
        button_pressed = 0;

        HAL_TIM_Base_Stop_IT(&htim3);
        __HAL_TIM_SET_COUNTER(&htim3, 0);

        if (long_long_press_sent)
            return;

        uint32_t press_time = now - press_tick;

        if (press_time >= 3000)
        {
            event = BTN_LONG_PRESS;
            HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);
        }
        else
        {
            event = BTN_SHORT_PRESS;
            HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
        }

        osMessageQueuePut(btnEventQueueHandle,
                          &event,
                          0,
                          0);
    }
}

void tim3_callback(void)
{
    if (!button_pressed)
        return;

    event = BTN_LONG_LONG_PRESS;

    osMessageQueuePut(btnEventQueueHandle,
                      &event,
                      0,
                      0);

    long_long_press_sent = 1;

    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);

    HAL_TIM_Base_Stop_IT(&htim3);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
	if (hadc->Instance == ADC1) {

	}
	else {
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if(hadc->Instance != ADC1)
        return;

    uint32_t sum = 0;

    for(int i = 0; i < ADC_BUF_LEN; i++)
        sum += ADC_Buffer[i];

    uint16_t avg = sum / ADC_BUF_LEN;

    osMessageQueuePut(mq3QueueHandle,
                      &avg,
                      0, 0);
    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);
}


