

#include "main.h"


void time_prt_Callback_fun (TimerHandle_t xtime)
{
    int time_num;
    static int over_num = 0;
    over_num ++;
    time_num = xTaskGetTickCount();
    // ESP_LOGI("timer prt ","Task Tick :%d ,over_num :%d",time_num,over_num);
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
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    free(temp_array);
}

TaskHandle_t led_taskhanlde;
TaskHandle_t lcd_taskhanlde;
TaskHandle_t oled_taskhanlde;

TimerHandle_t pr_timerhanlde;

void Build_task(void)
{
    led_taskhanlde = NULL;
    lcd_taskhanlde = NULL;

    custom_uart_task_Fun();
    xTaskCreatePinnedToCore(test_led_task, "task-[LED]", 4096, NULL, LED_TASK_PRIORITY, &led_taskhanlde, CORE_ZERO);
    
    // xTaskCreatePinnedToCore(refresh_lcd_task, "task-[LCD]", 4096 * 4, NULL, SHOW_TASK_PRIORITY, &lcd_taskhanlde, CORE_ONE);

    pr_timerhanlde = xTimerCreate("timer-[print]",1000,pdTRUE,TEST_TIMERID,time_prt_Callback_fun);
    

    if(pr_timerhanlde == NULL){
        ESP_LOGI("Build_time","Error ");
    }else{
        xTimerStart(pr_timerhanlde,1000);
        ESP_LOGI("Build_time","secc ");
    }
}

#include "hongshu.h"	// 图库

void Main_Init(void)
{
    // Allow other core to finish initialization
    vTaskDelay(pdMS_TO_TICKS(100));

    information_init(); // 打印初始化信息
    // disableCore1WDT();
    draw_coordinate_line_handle(0, 0, 18, 18);
    draw_coordinate_show(26, 26);
    LCD_Set_TargetModel(m_LCD_TYPE_1_69);
    LCD_Set_Horizontal(1);
    MODE_LCD_Init(1);
    Network_manage_Init (0xff);
    // LCD_Show_String(0,0, "hello ", LCD_Word_Color, LCD_Back_Color, 16);    // 显示字符串
    LCD_Show_Picture(0, 0, 240, 240, gImage_hongshu);
    ESP_LOGI("LCD Init","Model[%d],x:%d y:%d ",m_LCD_TYPE_1_69,LCD_W_Max,LCD_H_Max);
}

