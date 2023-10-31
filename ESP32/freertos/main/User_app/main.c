/* FreeRTOS Example


*/

#include "main.h"

void test_task(void *pvParam)
{
    int num = 0;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        num++;
        if (num > 10)
        {
            ESP_LOGI("test_task FUN", "doing : %d", num++);
            num = 0;
        }

    }
    
}

void Main_Init(void);
void Build_task(void);
void app_main(void)
{
    Main_Init();
    Build_task();

    char *temp_array;
        temp_array = malloc(300);
    while (1)
    {
        vTaskList(temp_array);
        printf("-------------------------------------------------->\n");
        printf(" Name          state   Priority  task    num\n");
        printf("%s \n",temp_array);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

TaskHandle_t test_taskhanlde;
TaskHandle_t led_taskhanlde;
TaskHandle_t lcd_taskhanlde;
void Build_task(void)
{
    test_taskhanlde = NULL;
    led_taskhanlde = NULL;
    lcd_taskhanlde = NULL;

    xTaskCreatePinnedToCore(test_task, "task-[test]", 4096, NULL, 1, &test_taskhanlde,CORE_ZERO);
    xTaskCreatePinnedToCore(test_led_task, "task-[LED]", 4096, NULL, 2, &led_taskhanlde,CORE_ZERO);

    xTaskCreatePinnedToCore(refresh_lcd_task, "task-[LCD]", 4096*3, NULL, 1, &lcd_taskhanlde,CORE_ONE);
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
    // LCD_Show_String(0,0,"123456", WHITE, BLACK, 16);

}
