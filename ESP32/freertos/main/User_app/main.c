/* FreeRTOS Example


*/

#include "main.h"

TaskHandle_t test_taskhanlde;
TaskHandle_t test_led_taskhanlde;

void test_task(void *pvParam)
{
    int num = 0;
    while (1)
    {
        ESP_LOGI("test_task FUN", "doing : %d", num++);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (num > 10)
        {
            num = 0;
            break;
        }
    }
    ESP_LOGW("test_task FUN", "exit test_task ");
    vTaskDelete(NULL);
}

void Main_Init(void);
void Build_task(void);
void app_main(void)
{
    Main_Init();
    Build_task();
    while (1)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);       /* 不加延迟会 triggered dog */ 

    }
}

void Build_task(void)
{
    test_taskhanlde = NULL;
    test_led_taskhanlde = NULL;
    xTaskCreate(test_task, "test task", 4096, NULL, 1, &test_taskhanlde);
    xTaskCreate(test_led_task, "test les task", 4096, NULL, 1, &test_led_taskhanlde);
}

void Main_Init(void)
{
    // Allow other core to finish initialization
    vTaskDelay(pdMS_TO_TICKS(100));

    information_init();     /* 打印初始化信息 */
    rtc_time_init(TURE);    /* 外置RTC时钟 */
    Custom_gpio_init(TURE); /* 简单的外设GPIO */

    draw_coordinate_line_handle(0, 0, 18, 18);
    draw_coordinate_show(26, 26);
}
