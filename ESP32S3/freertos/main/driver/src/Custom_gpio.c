#include "Custom_gpio.h"

#include "driver/gpio.h"

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

#if (Board_Name == ESP32_Cavend)
    if (set)
    {
        #if (LED_T_IO != -1)
        gpio_pad_select_gpio(LED_T_IO);
        gpio_set_direction(LED_T_IO, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_T_IO, 1);
        #endif

        #if (LED_R_IO != -1)
        gpio_pad_select_gpio(LED_R_IO);
        gpio_set_direction(LED_R_IO, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_R_IO, 1);
        #endif

        #if (LED_B_IO != -1)
        gpio_pad_select_gpio(LED_B_IO);
        gpio_set_direction(LED_B_IO, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_B_IO, 1);
        #endif
    }
    else
    {
        #if (LED_T_IO != -1)
        gpio_pad_select_gpio(LED_T_IO);
        gpio_set_direction(LED_T_IO, GPIO_MODE_INPUT_OUTPUT_OD);
        #endif
        #if (LED_R_IO != -1)
        gpio_pad_select_gpio(LED_R_IO);
        gpio_set_direction(LED_R_IO, GPIO_MODE_INPUT_OUTPUT_OD);
        #endif
        #if (LED_B_IO != -1)
        gpio_pad_select_gpio(LED_B_IO);
        gpio_set_direction(LED_B_IO, GPIO_MODE_INPUT_OUTPUT_OD);
        #endif
    }
    retval = ESP_OK;
#elif (Board_Name == EY1001)
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

#if (Board_Name == ESP32_Cavend)
    switch (n)
    {
    case 0:
        if (set)
        {
            gpio_set_level(LED_T_IO, 0);
        }
        else
        {
            gpio_set_level(LED_T_IO, 1);
        }
        break;
    case 1:
        if (set)
        {
            gpio_set_level(LED_R_IO, 0);
        }
        else
        {
            gpio_set_level(LED_R_IO, 1);
        }
        break;
    case 2:
        if (set)
        {
            gpio_set_level(LED_B_IO, 0);
        }
        else
        {
            gpio_set_level(LED_B_IO, 1);
        }
        break;
    default:
        retval = -1;
        break;
    }
    
#elif (Board_Name == EY1001)

#endif

    // ESP_LOGI(TAG, "led (%d) set : (%d) retval (%d) ", n, set, retval);
#endif
    return retval;
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
        if (num % 2)
        {
            LED_Set(LED_T, TURE);
            LED_Set(LED_R, TURE);
            LED_Set(LED_B, TURE);
        }
        else
        {
            LED_Set(LED_T, 0);
            LED_Set(LED_R, 0);
            LED_Set(LED_B, 0);
        }
        
        ESP_LOGI("test_led_task FUN", "\n ");
        if (num > 5)
        {
            num = 0;
            // while(1)
            // {
            //     vTaskDelay(600 / portTICK_PERIOD_MS);
            // }
        }
        vTaskDelay(absolute_Time);
    }
    vTaskDelete(NULL); /*  基本不用退出 */
}
