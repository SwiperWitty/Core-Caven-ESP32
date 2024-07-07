#ifndef _RTC_TIME__H_
#define _RTC_TIME__H_

/*
    this is rtc_time.h feil    

    2023.9.12   
*/
#include "Items.h"
#include "sys_typedef.h"
#include "Caven_Type.h"

#define AT8563  1
#define HYM8563 2

#if (BOARD_NAME == ESP32_CAVEND)
    #define RTC_TIME_IC     AT8563
#elif (BOARD_NAME == EY1001)
    #define RTC_TIME_IC     HYM8563
#endif


int rtc_time_init (int set);    //
int rtc_set_time (int set);
int rtc_get_time (int set);

#endif
