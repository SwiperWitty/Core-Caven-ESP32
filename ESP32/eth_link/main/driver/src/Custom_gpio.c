#include "Custom_gpio.h"
#include "string.h"
/*
    ESP_LOGE - 错误（最低）0
    ESP_LOGW - 警告
    ESP_LOGI - 信息
    ESP_LOGD - 调试
    ESP_LOGV - 详细（最高）
*/
static const char *TAG = "Custom_gpio";

/*
User_GPIO_config 设置GPIO-PIN的工作模式
gpiox [1-x] 无效
pin [0-15]  一次一个
set [0-1] 0-INPUT,1-OUTPUT 
*/
void User_GPIO_config(int gpiox,int pin,int set)
{
    if (pin > 0)
    {
        gpio_pad_select_gpio(pin);
        if (set)
        {
            gpio_set_direction(pin, GPIO_MODE_OUTPUT);
        }
        else
        {
            gpio_set_direction(pin, GPIO_MODE_INPUT);
        }
    }
}
/*
User_GPIO_set 设置GPIO-PIN的输出电平(输出模式)
gpiox [1-x]
pin [0-15]
set [0-1]
*/
void User_GPIO_set(int gpiox,int pin,int set)
{
    if (pin > 0)
    {
        if (set)
        {
            gpio_set_level(pin, 1);
        }
        else
        {
            gpio_set_level(pin, 0);
        }
    }
}

/*
User_GPIO_get 获取GPIO-PIN的输入电平
gpiox [1-x]
pin [0-15]

返回 0/1
*/
char User_GPIO_get(int gpiox,int pin)
{
	char retval = 0;
    if (pin > 0)
    {
        retval = (gpio_get_level(pin) & 0xff);
    }
	return retval;
}


int custom_gpio_init(int set)
{
    int retval = 0;
#ifdef Exist_GPIO

#if (BOARD_NAME == ESP32_CAVEND)
    if (set)
    {
        User_GPIO_config(0,LED_T_IO,1);
        User_GPIO_config(0,KEY_IO,0);
        User_GPIO_config(0,POW_KILL_IO,1);

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
    User_GPIO_set(0,POW_KILL_IO,set);
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
        
        vTaskDelay(pdMS_TO_TICKS(absolute_Time*10));
        gpio_set_level(LED_T_IO, temp_num%2);
    }
    vTaskDelete(NULL); /*  基本不用退出 */
}
