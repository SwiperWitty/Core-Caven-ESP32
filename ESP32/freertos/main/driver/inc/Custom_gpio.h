#ifndef _CUSTOM_GPIO__H_
#define _CUSTOM_GPIO__H_

/*
    this is Custom_gpio.h feil    
    这是个模板文件

    2023.9.13   
*/
#include "Items.h"
#include "sys_typedef.h"
#include "Caven_Type.h"

#include "driver/gpio.h"
#ifdef Exist_GPIO
    #if (BOARD_NAME == ESP32_CAVEND)
        #define LED_T_IO    GPIO_NUM_2      // GPIO_NUM_2(2) 
        #define LED_T    1
    #elif (BOARD_NAME == EY1001)
        
        
    #endif
#endif

int custom_gpio_init (int set);    // 示例 
int LED_Set (char n,int set);
void LED_task_run_enable (int set);
void test_led_task (void *pvParam);

#endif
