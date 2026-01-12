#include"information.h"

/*
    ESP_LOGE - 错误（最低）0
    ESP_LOGW - 警告
    ESP_LOGI - 信息
    ESP_LOGD - 调试
    ESP_LOGV - 详细（最高）
*/

// static const char *TAG = "information";


int information_init (void)
{
    int retval = 0;
    uint32_t flash_size;
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    ESP_LOGI(" ","\n <------------------RUN------------------> \n ");
    ESP_LOGI(" ","This is %s chip with %d CPU core(s), WiFi",
            CONFIG_IDF_TARGET,
            chip_info.cores);

    int major_rev = chip_info.model;
    int minor_rev = chip_info.revision;
    ESP_LOGI(" ","silicon revision v%d.%d,", major_rev, minor_rev);
    esp_flash_get_size(NULL, &flash_size);
    ESP_LOGI(" ","%d MB %s flash", (int)flash_size / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGI(" ","Minimum free heap size: %d bytes", (int)esp_get_minimum_free_heap_size());
    ESP_LOGI(" ","FreeRTOS HZ is %d \n", CONFIG_FREERTOS_HZ);
    return retval;
}


