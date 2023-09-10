/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"


#define LED_R   GPIO_NUM_13     // GPIO_NUM_13(13) 
#define LED_B   GPIO_NUM_14     // GPIO_NUM_14(14) 

const static char *TAG = "TEST : ";
/*
    ESP_LOGE - 错误（最低）
    ESP_LOGW - 警告
    ESP_LOGI - 信息
    ESP_LOGD - 调试
    ESP_LOGV - 详细（最高）
*/

void Main_init (void);
void app_main(void)
{
    Main_init ();
    int RUN_NUM = 0;
    gpio_set_level(LED_B, 0);
    while (1)
    {
        RUN_NUM++;
        gpio_set_level(LED_R, (RUN_NUM % 2) & 0X01);
        printf("Run time is %d s \r\n",RUN_NUM);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
    }
    fflush(stdout);
    esp_restart();

}

void Main_init (void)
{
    int temp = 0;
    printf("Hello world!\n");
    ESP_LOGI(TAG, "hi I !\n");
    ESP_LOGW(TAG, "hi warning !\n");
    ESP_LOGE(TAG, "hi error !\n");

    gpio_pad_select_gpio(LED_R);
    gpio_set_direction(LED_R, GPIO_MODE_OUTPUT); 
    gpio_pad_select_gpio(LED_B);
    gpio_set_direction(LED_B, GPIO_MODE_OUTPUT); 

    // 1为高电平，0为低电平
    gpio_set_level(LED_R, 1);
    gpio_set_level(LED_B, 1);

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "\n <---------RUN----------> \n ");
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    unsigned major_rev = chip_info.full_revision / 100;
    unsigned minor_rev = chip_info.full_revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}