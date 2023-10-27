#ifndef _CUSTOM_GPIO__H_
#define _CUSTOM_GPIO__H_

/*
    this is Custom_gpio.h feil    
    这是个模板文件

    2023.9.13   
*/

#include "Precompiling_define.h"
#include"sys_typedef.h"



#if (Board_Name == ESP32_Cavend)
    #include"Items.h"
    #define LED_R_IO    GPIO_NUM_13     // GPIO_NUM_13(13) 
    #define LED_B_IO    GPIO_NUM_14     // GPIO_NUM_14(14) 
    #define LED_R    1
    #define LED_B    2
#elif (Board_Name == EY1001)
    #include"Items.h"
    
#endif


int Custom_gpio_init (int set);    // 示例 
int LED_Set (char n,int set);
void test_led_task(void *pvParam);

#endif
