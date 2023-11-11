#include "Custom_gpio.h"

/*
    ESP_LOGE - 错误（最低）0
    ESP_LOGW - 警告
    ESP_LOGI - 信息
    ESP_LOGD - 调试
    ESP_LOGV - 详细（最高）
*/
static const char *TAG = "Custom_gpio";

int Custom_gpio_init(int set)
{
    int retval = 0;
#ifdef Exist_GPIO

#if (BOARD_NAME == ESP32_CAVEND)
    if (set)
    {
        gpio_pad_select_gpio(LED_T_IO);
        gpio_set_direction(LED_T_IO, GPIO_MODE_OUTPUT);

        gpio_set_level(LED_T_IO, 0);
    }
    else
    {
        gpio_pad_select_gpio(LED_T_IO);
        gpio_set_direction(LED_T_IO, GPIO_MODE_INPUT_OUTPUT_OD);
    }
    retval = ESP_OK;
#elif (BOARD_NAME == EY1001)
    if (set)
    {
    }
    else
    {
    }
#endif

    ESP_LOGI(TAG, "init (%d) ", retval);
#endif
    return retval;
}

int LED_Set(char n, int set)
{
    int retval = ESP_OK;
#ifdef Exist_GPIO

#if (BOARD_NAME == ESP32_CAVEND)
    switch (n)
    {
    case 1:
        if (set)
        {
            gpio_set_level(LED_T_IO, 1);        // high open 
        }
        else
        {
            gpio_set_level(LED_T_IO, 0);
        }
        break;
    case 2:
        if (set)
        {
        }
        else
        {
        }
        break;

    default:
        retval = -1;
        break;
    }
    
#elif (BOARD_NAME == EY1001)

#endif

    // ESP_LOGI(TAG, "led (%d) set : (%d) retval (%d) ", n, set, retval);
#endif
    return retval;
}

int LED_Flag_run = 0;
void LED_task_run_enable (int set)
{
    if (set)
    {
        LED_Flag_run = 1;
    }
    else
    {
        LED_Flag_run = 0;
    }
}

void test_led_task(void *pvParam)
{
    int num = 0;
    Custom_gpio_init(TURE); /* 简单的外设GPIO */
    TickType_t xLast_Time = xTaskGetTickCount();
    TickType_t absolute_Time = 200;
    while (1)
    {
        num++;
        if ((num % 2) && (LED_Flag_run == 1))
        {
            LED_Set(LED_T, TURE);
        }
        else
        {
            LED_Set(LED_T, DISABLE);
        }
        
        // ESP_LOGI("test_led_task FUN", "\n ");
        vTaskDelay(pdMS_TO_TICKS(absolute_Time));
    }
    vTaskDelete(NULL); /*  基本不用退出 */
}
