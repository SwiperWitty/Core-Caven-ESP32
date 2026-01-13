#include"system_app.h"

static const char *TAG = "SYS app";

SYS_cfg_Type g_SYS_Config = {
    .Board_ID = 0,
    .RS232_UART_Cfg = 115200,
    .RS485_UART_Cfg = 115200,
    .SYS_UART_Cfg = 460800,
    .Addr = 1,
    .debug = 0,

    .wifi_En = 0,
    .eth_En = 1,
    .NetCardCfg = 0,
    .tcp_server_enable = 1,
    .tcp_client_enable = 0,
    .tcp_http_enable = 1,
    .tcp_udp_enable = 0,
    .tcp_mqtt_enable = 0,
    
    .wifi_mode = 1,
    .eth_mode = 0,
    .eth_ip_str = "192.168.1.168",
    .eth_gw_str = "192.168.1.1",
    .eth_netmask_str = "255.255.255.0",

    .TCPHBT_En = 1,
    .Server_break_off = 1,              //
    .TCPServer_port = "8160",
    .TCPClient_url = "192.168.1.128:9090",   //

    .HTTPHBT_En = 1,
    .HTTP_cycle = 10000,
    .HTTP_url = "http://192.168.1.128:8080/post",
    
    .Net_falg = 0,
    .Now_time.SYS_Sec = 0,
    .Now_time.SYS_Us = 0,
    .Work_sec = 0,
    //
    .Connect_passage = 0,
    //
    .Heartbeat_nun = 0,
    .Heartbeat_Run = 0,
    .Heartbeat_MAX = 10,
};
Caven_event_Type g_SYS_events;

void system_cfg_memory_init (void)
{

}

void System_app_Restore (void)
{

}

int System_app_SYS_Config_Save (void)
{
    int retval = 0;

    return retval;
}

int System_app_SYS_Config_Gain (void)
{
    int retval = 0;

    return retval;
}

TaskHandle_t lcd_taskhanlde = NULL;
TaskHandle_t tcp_server_taskhanlde = NULL;
TaskHandle_t tcp_client_taskhanlde = NULL;
TaskHandle_t tcp_http_taskhanlde = NULL;

void system_app_init(void)
{
    // Allow other core to finish initialization
    int temp_num,temp_rtc = 0;
    char array_ip[100],array_port[20];
    //
    ESP_LOGI(TAG, "init -->");
    system_cfg_memory_init();
    MODE_RTC8564_Init (1);
    MODE_RTC8564_Read_time (&temp_rtc);
    g_SYS_Config.Now_time.SYS_Sec = temp_rtc;

    if (g_SYS_Config.eth_En)
    {
        ESP_LOGI(TAG,"enable eth");
        // Network_manage_set_mac (uint8_t *mac);
        eth_config_ip (g_SYS_Config.eth_mode,
                    g_SYS_Config.eth_ip_str,
                    g_SYS_Config.eth_gw_str,
                    g_SYS_Config.eth_netmask_str);
        Network_manage_Init (0x02,g_SYS_Config.eth_En);
    }
    if (g_SYS_Config.wifi_En)
    {
        ESP_LOGI(TAG,"enable wifi");
        wifi_config_user ("YXWL","yxwl6666");
        wifi_config_ip (g_SYS_Config.wifi_mode,
                    g_SYS_Config.wifi_ip_str,
                    g_SYS_Config.wifi_gw_str,
                    g_SYS_Config.wifi_netmask_str);
        Network_manage_Init (1,g_SYS_Config.wifi_En);
    }
    if(g_SYS_Config.tcp_server_enable)
    {
        xTaskCreate(tcp_server_link_task, "task-[server]", 1024*8, NULL, TCP_SERVER_TASK_PRIORITY, &tcp_server_taskhanlde);
        tcp_server_link_config (g_SYS_Config.TCPServer_port,g_SYS_Config.Server_break_off,g_SYS_Config.tcp_server_enable);
    }
    if(g_SYS_Config.tcp_client_enable)
    {
        xTaskCreate(tcp_client_link_task, "task-[client]", 1024*6, NULL, TCP_CLIENT_TASK_PRIORITY, &tcp_client_taskhanlde);
        tcp_client_link_config (array_ip,array_port,g_SYS_Config.tcp_client_enable);
    }
    if(g_SYS_Config.tcp_http_enable)
    {
        xTaskCreate(http_cache_port_task, "task-[http]", 1024*6, NULL, HTTP_TASK_PRIORITY, &tcp_http_taskhanlde);
        http_client_config_init (g_SYS_Config.HTTP_url,"8080",g_SYS_Config.tcp_http_enable);
        http_cache_port_data_Fun ("hello caven !");
    }
    //
    // https_request_config_init ("POST",WEB_SERVER,WEB_URL,NULL,1);
    //
    custom_uart2_init(g_SYS_Config.SYS_UART_Cfg, 1);
    ESP_LOGI(TAG,"SYS Cfg:%d",g_SYS_Config.SYS_UART_Cfg);

    ESP_LOGI(TAG, "init utc[%d] <--\n",(int)g_SYS_Config.Now_time.SYS_Sec);
}

void system_rst(void)
{
    esp_restart();
}

int sys_gpio_State_machine (Caven_BaseTIME_Type time)
{
    int retval = 0;

    return retval;
}

void sys_app_task(void *pvParam)
{
    User_GPIO_config(0,2,1);
    Task_Overtime_Type led_task = {
    .Begin_time = {0},
    .Set_time.SYS_Sec = 1,
    .Set_time.SYS_Us = 500000,
    .Switch = 1,
	};
    int tick = 0,tick_sec = 0;

    while (1)
    {
        tick_sec = xTaskGetTickCount() / 1000;
        if(tick != tick_sec)
        {
            tick = tick_sec;
            g_SYS_Config.Now_time.SYS_Sec ++;
        }
        g_SYS_Config.Now_time.SYS_Us = (xTaskGetTickCount() % 1000) * 1000;

        API_Task_Timer (&led_task,g_SYS_Config.Now_time);
        Caven_handle_event_Fun(&g_SYS_events);
        sys_gpio_State_machine (g_SYS_Config.Now_time);

        if(led_task.Trigger_Flag)
        {
            User_GPIO_set(0,2,led_task.Flip_falg);
            ESP_LOGI(TAG, "utc[%d:%d]--",(int)g_SYS_Config.Now_time.SYS_Sec,(int)g_SYS_Config.Now_time.SYS_Us);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    vTaskDelete(NULL); /*  基本不用退出 */
}
