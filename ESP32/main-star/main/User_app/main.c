
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

uint8_t get3_buff[0x1024];
int get3_num = 0;
int get3_time = 0;
void uart1_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;

    get3_buff[get3_num] = rec;
    get3_num ++;
    if (get3_num > sizeof(get3_buff) - 1)
    {
        get3_num = sizeof(get3_buff) - 1;
        ESP_LOGI("debug","get3_num over");
    }
    get3_time = xTaskGetTickCount();
}

uint8_t send_buff[0x1024];
void app_main(void)
{
    Main_Init();
    Build_task();
    printf("app_main run to Core %d \n \n", xPortGetCoreID());
    char *temp_array;
    int temp_num = 0;
    int run_time;

    temp_array = malloc(300);
    while (1)
    {
        run_time = xTaskGetTickCount();

        //
        if (wifi_get_local_ip_status(NULL,NULL,NULL))
        {
            uint8_t ip_str[10];
            uint8_t gw_str[10];
            uint8_t netmask_str[10];
            wifi_get_local_ip_status(ip_str,gw_str,netmask_str);
            
            sprintf(temp_array,"WIFI: %03d.%03d.%03d.%03d",ip_str[0],ip_str[1],ip_str[2],ip_str[3]);
            show_set_net_information (temp_array);
        }
        else if (eth_get_local_ip_status(NULL,NULL,NULL))
        {
            uint8_t ip_str[10];
            uint8_t gw_str[10];
            uint8_t netmask_str[10];
            eth_get_local_ip_status(ip_str,gw_str,netmask_str);
            
            sprintf(temp_array,"RJ45: %03d.%03d.%03d.%03d",ip_str[0],ip_str[1],ip_str[2],ip_str[3]);
            show_set_net_information (temp_array);
        }
        //
        if (((run_time - get_time) > 50 && get_num) || get_num > 300)
        {
            temp_num = get_num;
            memcpy(send_buff,get_buff,temp_num);
            get_num = 0;
            if (0)
            {
                tcp_server_send_data(send_buff, temp_num);
                if (strlen((char *)send_buff))
                {
                    show_txt_str_information ((char *)send_buff);
                    ESP_LOGI("debug","show sync");
                }
                ESP_LOGI("debug","Collect server data updata,%d",temp_num);
            }
            else
            {
                custom_uart1_send_data(send_buff, temp_num);
                if (memcmp(send_buff,"{off server sock}",strlen("{off server sock}")) == 0)
                {
                    tcp_server_link_config ("8160",0);
                }
                else if (memcmp(send_buff,"{off client sock}",strlen("{off client sock}")) == 0)
                {
                    tcp_client_link_config ("192.168.11.57","9090",2);
                }
                //
                if (memcmp(send_buff,"{off client}",strlen("{off client}")) == 0)
                {
                    tcp_client_link_config ("192.168.11.57","9090",0);
                }
                else if (memcmp(send_buff,"{on client}",strlen("{on client}")) == 0)
                {
                    tcp_client_link_config ("192.168.11.57","9090",1);
                }
            }
        }
        if (((run_time - get2_time) > 50 && get2_num) || get2_num > 300)
        {
            temp_num = get2_num;
            memcpy(send_buff,get2_buff,temp_num);
            get2_num = 0;
            if (0)
            {
                tcp_client_send_data(send_buff, temp_num);
                ESP_LOGI("debug","Collect client data updata,%d",temp_num);
            }
            else
            {
                custom_uart1_send_data(send_buff, temp_num);
            }
        }
        if(((run_time - get3_time) > 50 && get3_num) || get3_num > 300)
        {
            temp_num = get3_num;
            memcpy(send_buff,get3_buff,temp_num);
            get3_num = 0;
            tcp_server_send_data(send_buff, temp_num);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
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
    // xTaskCreatePinnedToCore(test_led_task, "task-[LED]", 4096, NULL, GPIO_TASK_PRIORITY, &led_taskhanlde, CORE_ZERO);
    xTaskCreate(show_app_task, "task-[show app]", 1024 * 10, NULL, SHOW_TASK_PRIORITY, NULL);
    xTaskCreate(tcp_server_link_task, "tcp server task", 1024*4, NULL, TCP_SERVER_TASK_PRIORITY, NULL);
    xTaskCreate(tcp_client_link_task, "tcp client task", 1024*4, NULL, TCP_CLIENT_TASK_PRIORITY, NULL);
    // xTaskCreate(eps32_HTTPS_task, "https get task", 1024*8, NULL, HTTPS_TASK_PRIORITY, NULL);

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
    uint32_t temp_rtc = 0;
    vTaskDelay(pdMS_TO_TICKS(10));
    //
    information_init(); // 打印初始化信息
    draw_coordinate_line_handle(0, 0, 18, 18);
    draw_coordinate_show(26, 26);
    //
    eth_config_ip (1,"192.168.1.169","192.168.1.1","255.255.255.0");
    wifi_config_ip (0,NULL,NULL,NULL);  // 设置网络模式
    wifi_config_ip (2,"192.168.11.61","192.168.11.1","255.255.255.0");  // 配置静态模式下的ip
    Network_manage_Init (0x01,1);
    // Network_manage_Init (0x02,1);
    //
    tcp_client_link_config ("192.168.11.57","9090",0);
    tcp_server_link_config ("8160",1);
    tcp_server_receive_State_Machine_Bind (rj45_get_fun);
    tcp_client_receive_State_Machine_Bind (rj45_get2_fun);
    //
    custom_uart1_init(115200, 1);
    custom_uart1_receive_State_Machine_Bind(uart1_get_fun);
    //
    MODE_RTC8564_Init (1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    int a = MODE_RTC8564_Read_time (&temp_rtc);
    ESP_LOGI("RTC","get time [%d] (%d)",temp_rtc,a);
    if (temp_rtc < 1723294126)
    {
        ESP_LOGI("RTC","SET time ");
        MODE_RTC8564_Write_time (1723294126 + 60);
    }
}

