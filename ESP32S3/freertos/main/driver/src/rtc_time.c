#include"rtc_time.h"

#include "driver/i2c.h"

/*
    ESP_LOGE - 错误（最低）0
    ESP_LOGW - 警告
    ESP_LOGI - 信息
    ESP_LOGD - 调试
    ESP_LOGV - 详细（最高）
*/
static const char *TAG = "Driver";


int rtc_time_init (int set)
{
    int retval = 0;
#ifdef Exist_RTC_Clock
    if (set)
    {
        

    }
    else
    {

    }
    ESP_LOGI(TAG,"rtc_time_init %d ",retval);
#endif 
    return retval;
}

int rtc_set_time (int set)
{
    int retval = 0;
#ifdef Exist_RTC_Clock
    retval = 1;
#endif 
    return retval;

}

int rtc_get_time (int set)
{
    int retval = 0;
#ifdef Exist_RTC_Clock
    retval = 1;
#endif 
    return retval;

}

