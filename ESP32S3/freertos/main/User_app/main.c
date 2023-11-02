/* FreeRTOS Example


*/

#include "main.h"

extern LCD_data_Type ESP32_lcd_show;

void test_task(void *pvParam)
{
    int num = 0;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        num++;
        if (num % 3)
        {   
            ESP_LOGI("test_task", "num (%d)", num);
        }

        if (num == 5)
        {
            // if (ESP32_lcd_show.flag == 0)
            // {
            //     ESP32_lcd_show.flag = 1;
            //     ESP32_lcd_show.refresh = 3;
            //     ESP32_lcd_show.flag = 0;
            // }
        }
        else if (num == 10)
        {
            if (ESP32_lcd_show.flag == 0)
            {
                ESP32_lcd_show.flag = 1;
                ESP32_lcd_show.refresh = 2;
                ESP32_lcd_show.pic_place[0] = 0;
                ESP32_lcd_show.pic_place[1] = 0;
                ESP32_lcd_show.pic_place[2] = 240;
                ESP32_lcd_show.pic_place[3] = 240;
                ESP32_lcd_show.flag = 0;
            }
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
    printf("app_main run to Core %d \n \n", xPortGetCoreID());
    char *temp_array;
    temp_array = malloc(300);
    while (1)
    {
        printf("-------------------------------------------------->\n");
#if 1
        vTaskList(temp_array);
        printf(" Name          state   Priority  task    num\n");
        printf("%s \n",temp_array);
#else
        vTaskGetRunTimeStats(temp_array);
        printf("\r\n任务名       运行计数         使用率\r\n");
        printf("\r\n%s\r\n", temp_array);
        
#endif
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    free(temp_array);
}

TaskHandle_t test_taskhanlde;
TaskHandle_t led_taskhanlde;
TaskHandle_t lcd_taskhanlde;
void Build_task(void)
{
    test_taskhanlde = NULL;
    led_taskhanlde = NULL;
    lcd_taskhanlde = NULL;

    // xTaskCreatePinnedToCore(test_task, "task-[test]", 4096, NULL, 1, &test_taskhanlde, CORE_ZERO);
    xTaskCreatePinnedToCore(test_led_task, "task-[LED]", 4096, NULL, 2, &led_taskhanlde, CORE_ZERO);

    xTaskCreatePinnedToCore(refresh_lcd_task, "task-[LCD]", 4096 * 4, NULL, 3, &lcd_taskhanlde, CORE_ONE);
}

void Main_Init(void)
{
    // Allow other core to finish initialization
    vTaskDelay(pdMS_TO_TICKS(100));

    information_init(); /* 打印初始化信息 */

    draw_coordinate_line_handle(0, 0, 18, 18);
    draw_coordinate_show(26, 26);
    // LCD_Show_String(0,0,"123456", WHITE, BLACK, 16);
}
