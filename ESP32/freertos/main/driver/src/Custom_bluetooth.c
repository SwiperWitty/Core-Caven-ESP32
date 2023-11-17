#include"Custom_bluetooth.h"


/*
    ESP_LOGE - 错误（最低）0
    ESP_LOGW - 警告
    ESP_LOGI - 信息
    ESP_LOGD - 调试
    ESP_LOGV - 详细（最高）
*/

static const char *TAG = "LOG bluetooth";


int custom_bluetooth_init (int set)
{
    int retval = 0;
#ifdef EXIST_BLUETOOTH
    if (set)
    {
        
    }
    else
    {

    }
    ESP_LOGI(TAG,"init %d ",retval);
#endif 
    return retval;
}


