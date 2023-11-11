#ifndef _MAIN__H_
#define _MAIN__H_


/*
    this is main.h feil    

    2023.9.12   
*/
#include"sys_typedef.h"

#include "nvs_flash.h"
#include "driver/gpio.h"

// #include "esp_bt.h"
// #include "esp_gap_ble_api.h"
// #include "esp_gatts_api.h"
// #include "esp_bt_defs.h"
// #include "esp_bt_main.h"
// #include "ble50_sec_gatts_demo.h"

#include "information.h"

#if (BOARD_NAME == ESP32_CAVEND)
    #include"Items.h"
#elif (BOARD_NAME == EY1001)
    #include"Items.h"
#endif


#ifdef Exist_RTC_Clock
    #include"rtc_time.h"
#endif 
#ifdef Exist_GPIO
    #include"Custom_gpio.h"
#endif 
#ifdef Exist_UART 
    #include"Custom_uart.h" 
#endif 
#ifdef Exist_LCD
    #include"lcd_st7789.h"
#endif 

#include"draw.h"

#define CORE_ZERO   0
#define CORE_ONE    1

#define GATTS_TABLE_TAG "SEC_GATTS_DEMO"
#define HEART_PROFILE_NUM                         1
#define HEART_PROFILE_APP_IDX                     0
#define ESP_HEART_RATE_APP_ID                     0x55
#define HEART_RATE_SVC_INST_ID                    0
#define EXT_ADV_HANDLE                            0
#define NUM_EXT_ADV_SET                           1
#define EXT_ADV_DURATION                          0
#define EXT_ADV_MAX_EVENTS                        0

#define GATTS_DEMO_CHAR_VAL_LEN_MAX               0x40
/* Attributes State Machine */
enum
{
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,
    IDX_CHAR_CFG_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,

    IDX_CHAR_C,
    IDX_CHAR_VAL_C,

    HRS_IDX_NB,
};

#endif // !_MAIN__H_
