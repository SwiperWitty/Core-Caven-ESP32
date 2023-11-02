#ifndef _RTC_TIME__H_
#define _RTC_TIME__H_

/*
    this is rtc_time.h feil    

    2023.9.12   
*/

#include "Precompiling_define.h"
#include"sys_typedef.h"

#define AT8563  1
#define HYM8563 2

#if (Board_Name == ESP32_Cavend)
    #include"Items.h"
    #define RTC_TIME_IC     AT8563
#elif (Board_Name == EY1001)
    #include"Items.h"
    #define RTC_TIME_IC     HYM8563
#endif


int rtc_time_init (int set);    //
int rtc_set_time (int set);
int rtc_get_time (int set);

#endif
