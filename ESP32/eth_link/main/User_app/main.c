
#include "main.h"


void time_prt_Callback_fun (TimerHandle_t xtime)
{
    static int over_num = 0;
    over_num ++;
    g_SYS_Config.time.SYS_Sec ++;
}

void http_Callback_fun (void *data)
{
    char *temp_str;
    temp_str = data;
    ESP_LOGW("http_Callback","\n[%s]\n",temp_str);
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
    ESP_LOGI("ram log 2","free_heap_size = %d \r\n",ram_size);
    temp_array = malloc(300);
    while (1)
    {
        g_SYS_Config.time.time_us = (xTaskGetTickCount() % 1000)*1000;
        g_SYS_Config.WIFI_online = wifi_get_local_ip_status(NULL,NULL,NULL);
        g_SYS_Config.RJ45_online = eth_get_local_ip_status(NULL,NULL,NULL);
        g_SYS_Config.AT4G_online = 0;
        g_SYS_Config.SYS_online = 1;        // 恒为1
        //
        if (g_SYS_Config.WIFI_online)
        {
            uint8_t ip_str[10];
            uint8_t gw_str[10];
            uint8_t netmask_str[10];
            wifi_get_local_ip_status(ip_str,gw_str,netmask_str);
            sprintf(temp_array,"WIFI: %03d.%03d.%03d.%03d",ip_str[0],ip_str[1],ip_str[2],ip_str[3]);
            // ESP_LOGW("main log","%s \r\n\r\n", temp_array);
            show_set_net_information (temp_array);
        }
        else if (g_SYS_Config.RJ45_online)
        {
            uint8_t ip_str[10];
            uint8_t gw_str[10];
            uint8_t netmask_str[10];
            eth_get_local_ip_status(ip_str,gw_str,netmask_str);
            sprintf(temp_array,"RJ45: %03d.%03d.%03d.%03d",ip_str[0],ip_str[1],ip_str[2],ip_str[3]);
            // ESP_LOGW("main log","%s \r\n\r\n", temp_array);
            show_set_net_information (temp_array);
        }

        ram_size = esp_get_free_heap_size();
        if (ram_size < 4096*10)
        {
            ESP_LOGE("ram log","error free_heap_size = %d \r\n\r\n", ram_size);
            system_rst();
        }
        http_cache_port_data_Fun ("{\"deviceSerial\":\"N6801FFFF241116A0040\",\"heartbeatTime\":\"1738749813000\",\"upTime\":\"1209126\"}");

        vTaskDelay(pdMS_TO_TICKS(3000));
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
    int ram_size = esp_get_free_heap_size();
    ESP_LOGI("ram log 1","free_heap_size = %d \r\n",ram_size);
    //
    http_receive_Callback_Bind (http_Callback_fun);
    //
    custom_uart_task_Fun();
    xTaskCreatePinnedToCore(test_led_task, "task-[LED]", 1024*2, NULL, GPIO_TASK_PRIORITY, &led_taskhanlde, CORE_ZERO);
    // xTaskCreate(show_app_task, "task-[show app]", 1024 * 10, NULL, SHOW_TASK_PRIORITY, NULL);
    xTaskCreate(tcp_server_link_task, "tcp server task", 1024*4, NULL, TCP_SERVER_TASK_PRIORITY, NULL);
    xTaskCreate(tcp_client_link_task, "tcp client task", 1024*4, NULL, TCP_CLIENT_TASK_PRIORITY, NULL);
    xTaskCreate(eps32_HTTPS_task, "https get task", 1024*6, NULL, HTTP_TASK_PRIORITY, NULL);
    xTaskCreate(Message_info_task, "tcp Message Main task", 1024*8, NULL, MESSAGE_INFO_TASK_PRIORITY, NULL);

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
    draw_coordinate_line_handle(0, 0, 18, 18);
    draw_coordinate_show(26, 26);
    // net + uart
    system_app_init();
}

