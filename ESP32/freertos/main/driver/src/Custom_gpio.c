#include"Custom_gpio.h"

#include "driver/gpio.h"

/*
    ESP_LOGE - 错误（最低）0
    ESP_LOGW - 警告
    ESP_LOGI - 信息
    ESP_LOGD - 调试
    ESP_LOGV - 详细（最高）
*/
static const char *TAG = "Custom_gpio";


int Custom_gpio_init (int set)
{
    int retval = 0;
#ifdef Exist_GPIO

    #if (Board_Name == ESP32_Cavend)
    if (set)
    {
        gpio_pad_select_gpio(LED_R_IO);
        gpio_set_direction(LED_R_IO, GPIO_MODE_OUTPUT); 
        gpio_pad_select_gpio(LED_B_IO);
        gpio_set_direction(LED_B_IO, GPIO_MODE_OUTPUT); 

        gpio_set_level(LED_R_IO, 1);
        gpio_set_level(LED_B_IO, 1);
        
    }
    else
    {
        gpio_pad_select_gpio(LED_R_IO);
        gpio_set_direction(LED_R_IO, GPIO_MODE_INPUT_OUTPUT_OD); 
        gpio_pad_select_gpio(LED_B_IO);
        gpio_set_direction(LED_B_IO, GPIO_MODE_INPUT_OUTPUT_OD); 
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

    ESP_LOGI(TAG,"init (%d) ",retval);
#endif 
    return retval;
}

int LED_Set (char n,int set)
{
    int retval = 0;
#ifdef Exist_GPIO

    #if (Board_Name == ESP32_Cavend)
    switch (n)
    {
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
        break;
    }
    retval = ESP_OK;
    #elif (Board_Name == EY1001)

    #endif

    ESP_LOGI(TAG,"led set %d : %d  (%d) ",n,set,retval);
#endif 
    return retval;
}


