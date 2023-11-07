#ifndef _CUSTOM_GPIO__H_
#define _CUSTOM_GPIO__H_

/*
    this is Custom_gpio.h feil    
    这是个模板文件

    2023.9.13   
*/

#include "Precompiling_define.h"
#include "sys_typedef.h"



#if (Board_Name == ESP32_Cavend)
    #include"Items.h"
    #define LED_T_IO    GPIO_NUM_2      // GPIO_NUM_2(2) 
    #define LED_T    1
#elif (Board_Name == EY1001)
    #include"Items.h"
    
#endif


int Custom_gpio_init (int set);    // 示例 
int LED_Set (char n,int set);
void test_led_task (void *pvParam);

#endif
