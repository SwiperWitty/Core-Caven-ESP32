#ifndef _ITEMS__H_
#define _ITEMS__H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_task_wdt.h" 
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"


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
#define Exist_UART 
#define EXIST_BLUETOOTH 

                        /*  只需要加上逻辑才能的功能    */
#define Exist_LCD
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
/*-----------------------------------*/

/*  进一步的逻辑关系    */

/*-----------------------------------*/

/*  [priority]    */
#define OTA_TASK_PRIORITY   1
#define GPIO_TASK_PRIORITY  2 
#define SHOW_TASK_PRIORITY  3 

#define UART1_TASK_PRIORITY 5 
#define UART2_TASK_PRIORITY 6 
#define BLUE_TOOCH_TASK_PRIORITY  7
#define TCP_SERVER_TASK_PRIORITY  7 
#define TCP_SERVER_RECV_TASK_PRIORITY  8
#define TCP_CLIENT_TASK_PRIORITY  9 
#define UDP_TASK_PRIORITY   10 
#define HTTP_TASK_PRIORITY  11
#define MQTT_TASK_PRIORITY  11


#define MESSAGE_INFO_TASK_PRIORITY  15 

/*-----------------------------------*/

/*  [timerID]     */
#define TEST_TIMERID        (void *)0 
#define CHECK_TIMERID       (void *)1 
/*-----------------------------------*/
#endif
