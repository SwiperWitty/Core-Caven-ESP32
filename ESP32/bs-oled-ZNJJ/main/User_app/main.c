
#include "main.h"


void time_prt_Callback_fun (TimerHandle_t xtime)
{
    static int over_num = 0;
    over_num ++;
    g_SYS_Config.time.SYS_Sec ++;
}

void Main_Init(void);
void Build_task(void);

void app_main(void)
{
    Main_Init();
    Build_task();
    printf("app_main run to Core %d \n \n", xPortGetCoreID());
    char *temp_array;

    int ram_size = esp_get_free_heap_size();
    ESP_LOGI("RAM log 2","free_heap_size = %d \r\n",ram_size);
    temp_array = malloc(300);
    while (1)
    {
        g_SYS_Config.time.SYS_Sec = (xTaskGetTickCount() % 1000)*1000;
        g_SYS_Config.SYS_online = 1;        // 恒为1
        //

        ram_size = esp_get_free_heap_size();
        if (ram_size < 4096*10)
        {
            ESP_LOGE("ram log","error free_heap_size = %d \r\n\r\n", ram_size);
            system_rst();
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
    free(temp_array);
}

TaskHandle_t led_taskhanlde = NULL;
TaskHandle_t lcd_taskhanlde = NULL;
TaskHandle_t tcp_server_taskhanlde = NULL;

TimerHandle_t pr_timerhanlde;

void Build_task(void)
{
    /*
    xTaskCreate 默认在1核
    */
    int ram_size = esp_get_free_heap_size();
    ESP_LOGI("RAM log 1","free_heap_size = %d \r\n",ram_size);
    //
    custom_uart_task_Fun();

    xTaskCreate(test_led_task, "task-[LED]", 1024*2, NULL, GPIO_TASK_PRIORITY, &led_taskhanlde);
    xTaskCreate(tcp_server_link_task, "task-[server]", 1024*8, NULL, TCP_SERVER_TASK_PRIORITY, &tcp_server_taskhanlde);
    // xTaskCreatePinnedToCore(show_app_task, "task-[show app]", 1024 * 10, NULL, SHOW_TASK_PRIORITY, NULL,CORE_ZERO);
    pr_timerhanlde = xTimerCreate("timer-[print]",1000,pdTRUE,TEST_TIMERID,time_prt_Callback_fun);

    if(pr_timerhanlde == NULL){
        ESP_LOGI("Build_time","Error ");
    }else{
        xTimerStart(pr_timerhanlde,1000);
        ESP_LOGI("Build_time","secc ");
    }
}

void Main_Init(void)
{
    // Allow other core to finish initialization
    vTaskDelay(pdMS_TO_TICKS(10));
    //
    information_init(); // 打印初始化信息
    // net + uart
    system_app_init();

}

