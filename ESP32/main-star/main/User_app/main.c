

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

uint8_t get_buff[0x1024];
int get_num = 0;
int get_time = 0;
void rj45_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;
    get_buff[get_num] = rec;
    get_num ++;
    get_time = xTaskGetTickCount();
    if (get_num > sizeof(get_buff) - 1)
    {
        get_num = sizeof(get_buff) - 1;
    }
}

uint8_t get2_buff[0x1024];
int get2_num = 0;
int get2_time = 0;
void rj45_get2_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;
    get2_buff[get2_num] = rec;
    get2_num ++;
    get2_time = xTaskGetTickCount();
    if (get2_num > sizeof(get2_buff) - 1)
    {
        get2_num = sizeof(get2_buff) - 1;
    }
}

void app_main(void)
{
    Main_Init();
    Build_task();
    printf("app_main run to Core %d \n \n", xPortGetCoreID());
    char *temp_array;
    int temp_num = 0;
    temp_array = malloc(300);
    tcp_server_receive_State_Machine_Bind (rj45_get_fun);
    tcp_client_receive_State_Machine_Bind (rj45_get2_fun);
    int run_time = xTaskGetTickCount();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        run_time = xTaskGetTickCount();
        if (((run_time - get_time) > 50 && get_num) || get_num > 300)
        {
            temp_num = get_num;
            get_num = 0;
            tcp_server_send_data(get_buff, temp_num);
            ESP_LOGI("debug","Collect server data updata,%d",temp_num);
        }
        if (((run_time - get2_time) > 50 && get2_num) || get2_num > 300)
        {
            temp_num = get2_num;
            get2_num = 0;
            tcp_client_send_data(get2_buff, temp_num);
            ESP_LOGI("debug","Collect client data updata,%d",temp_num);
        }
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
    tcp_client_link_ip_config ("192.168.1.128","9090",1);
    tcp_server_link_ip_config ("8160",1);

    // xTaskCreatePinnedToCore(refresh_lcd_task, "task-[LCD]", 4096 * 4, NULL, SHOW_TASK_PRIORITY, &lcd_taskhanlde, CORE_ONE);
    xTaskCreate(eps32_HTTPS_task, "https get task", 8192, NULL, 5, NULL);
    xTaskCreate(tcp_server_link_task, "tcp server task", 1024*4, NULL, 10, NULL);
    xTaskCreate(tcp_client_link_task, "tcp client task", 1024*4, NULL, 11, NULL);
    
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
    draw_coordinate_line_handle(0, 0, 18, 18);
    draw_coordinate_show(26, 26);
    LCD_Set_TargetModel(m_LCD_TYPE_1_69);
    LCD_Set_Horizontal(1);
    MODE_LCD_Init(1);
    eth_config_ip (1,"192.168.1.169","192.168.1.1","255.255.255.0");
    wifi_config_ip (0,NULL,NULL,NULL);  // 设置网络模式
    wifi_config_ip (2,"192.168.11.61","192.168.11.1","255.255.255.0");  // 配置静态模式ip

    Network_manage_Init (0xff,1);
    // LCD_Show_String(0,0, "hello ", LCD_Word_Color, LCD_Back_Color, 16);    // 显示字符串
    LCD_Show_Picture(0, 0, 240, 240, gImage_hongshu);
    ESP_LOGI("LCD Init","Model[%d],x:%d y:%d ",m_LCD_TYPE_1_69,LCD_W_Max,LCD_H_Max);

}

