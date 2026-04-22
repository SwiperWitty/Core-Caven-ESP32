#include "Base_GPIO.h"
#include "driver/gpio.h"


/*
User_GPIO_config 设置GPIO-PIN的工作模式
gpiox [1-x] 无效
pin [1-38]  一次一个
set [0-1] 0-INPUT,1-OUTPUT 
*/
void User_GPIO_config(int gpiox,int pin,int set)
{
    if (pin > 0)
    {
        esp_rom_gpio_pad_select_gpio(pin);
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
User_GPIO_config 设置GPIO-PIN的输出电平(输出模式)
gpiox [1-x] 无效
pin [1-38]  一次一个
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
User_GPIO_config 设置GPIO-PIN的输出电平(输出模式)
gpiox [1-x] 无效
pin [1-38]  一次一个
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
