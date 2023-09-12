#ifndef _RTC_TIME__H_
#define _RTC_TIME__H_

#include"Items.h"
#include"sys_typedef.h"

#include"rtc_time.h"

#define AT8563  1
#define HYM8563 2

#define RTC_TIME_IC     AT8563


int rtc_time_init (int set);    //
int rtc_set_time (int set);
int rtc_get_time (int set);

#endif
