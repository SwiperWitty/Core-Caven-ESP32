#ifndef _ITEMS__H_
#define _ITEMS__H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_task_wdt.h" 
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
    this is Items.h feil    
    最底层的存在

    2023.9.12
    The feil to inc & src
*/

#define TURE   1
#define DEBUG_OUT   1           //Debug 通道(串口1)->MODE


                        /*  基本外设就能实现的功能    */
// nc
#define Exist_GPIO 
#define Exist_LCD 
#define Exist_OLED 

                        /*  只需要加上逻辑才能的功能    */
// #define Exist_LCD
// #define Exist_OLED            //一般这两个是二选一（占用的都是SPI）

// #define Exist_HC138
// #define Exist_HC595
// #define Exist_DS18B20

// #define Exist_Ultrasonic             //超声波测距

// #define Exist_Voice                  //语音播报(MP3)

// #define Exist_Motor                  //电机
// #define Exist_STEP_Motor
// #define Exist_Steering_Engine        //舵机

#define Exist_RTC_Clock              //外部硬件RTC时钟


/*  进一步的逻辑关系    */


#endif
