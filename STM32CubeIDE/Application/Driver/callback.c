#include "callback.h"


volatile uint32_t last_irq_time = 0;
extern TIM_HandleTypeDef htim3;
volatile uint8_t button_pressed = 0;
volatile uint8_t long_press_sent = 0;

extern osMessageQueueId_t btnEventQueueHandle;
ButtonEvent_t event = BTN_NONE;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
    	uint32_t now = HAL_GetTick();

    	if(now - last_irq_time < 20)
    	    return;

    	last_irq_time = now;

    	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
    	{
    	    button_pressed = 1;
    	    long_press_sent = 0;

    	    __HAL_TIM_SET_COUNTER(&htim3,0);
    	    HAL_TIM_Base_Start_IT(&htim3);
    	}
    	else
    	{
    	    button_pressed = 0;


    	    HAL_TIM_Base_Stop_IT(&htim3);

    	    if(long_press_sent == 0)
    	    {
    	        event = BTN_SHORT_PRESS;
				HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
    	        osMessageQueuePut(btnEventQueueHandle,
    	                          &event,
    	                          0,
    	                          0);
    	    }

    	    __HAL_TIM_SET_COUNTER(&htim3,0);
    	}
    }
}

void tim3_callback()
{
    event = BTN_LONG_PRESS;

    osMessageQueuePut(btnEventQueueHandle,
                      &event,
                      0,
                      0);

    long_press_sent = 1;

    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);

    HAL_TIM_Base_Stop_IT(&htim3);
}

