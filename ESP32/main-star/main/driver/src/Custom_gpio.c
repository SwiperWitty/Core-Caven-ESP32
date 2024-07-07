#include "Custom_gpio.h"

/*
    ESP_LOGE - 错误（最低）0
    ESP_LOGW - 警告
    ESP_LOGI - 信息
    ESP_LOGD - 调试
    ESP_LOGV - 详细（最高）
*/
static const char *TAG = "Custom_gpio";

int custom_gpio_init(int set)
{
    int retval = 0;
#ifdef Exist_GPIO

#if (BOARD_NAME == ESP32_CAVEND)
    if (set)
    {
        gpio_pad_select_gpio(LED_T_IO);
        gpio_set_direction(LED_T_IO, GPIO_MODE_OUTPUT);
        gpio_pad_select_gpio(POW_KILL_IO);
        gpio_set_direction(POW_KILL_IO, GPIO_MODE_OUTPUT);
        gpio_pad_select_gpio(KEY_IO);
        gpio_set_direction(KEY_IO, GPIO_MODE_INPUT);

        LCD_POW_Set (1);
        POW_KILL_Set (0);
    }
    else
    {
        gpio_pad_select_gpio(LED_T_IO);
        gpio_set_direction(LED_T_IO, GPIO_MODE_INPUT_OUTPUT_OD);
    }
    retval = ESP_OK;
#elif (BOARD_NAME == EY1001)

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

#endif
    return retval;
}

int KEY_Get (void)
{
    int retval = 0;
    retval = gpio_get_level(KEY_IO);
    return retval;
}

void LCD_POW_Set (int set)
{
    if (set)
    {
        gpio_set_level(LCD_POW_IO, 0);
    }
    else
    {
        gpio_set_level(LCD_POW_IO, 1);
    }
}

void POW_KILL_Set (int set)
{
    if (set)
    {
        gpio_set_level(POW_KILL_IO, 1);
    }
    else
    {
        gpio_set_level(POW_KILL_IO, 0);
    }
}

void test_led_task(void *pvParam)
{
    int temp_num = 0;
    custom_gpio_init(TURE); /* 简单的外设GPIO */
    TickType_t xLast_Time = xTaskGetTickCount();
    TickType_t absolute_Time = 100;
    while (1)
    {
        temp_num++;
        if (KEY_Get () == 0)
        {
            temp_num = 0;
            xLast_Time = xTaskGetTickCount();
            ESP_LOGI(TAG, "key get low");
            do{
                if ((xTaskGetTickCount() - xLast_Time) > 2000 && temp_num == 0)
                {
                    ESP_LOGI(TAG, "get off");
                    temp_num = 1;
                }
                vTaskDelay(pdMS_TO_TICKS(absolute_Time));
            }while (KEY_Get () == 0);
            if (temp_num)
            {
                vTaskDelay(pdMS_TO_TICKS(absolute_Time));
                POW_KILL_Set (1);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(absolute_Time));
    }
    vTaskDelete(NULL); /*  基本不用退出 */
}
