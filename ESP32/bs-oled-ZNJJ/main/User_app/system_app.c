#include"system_app.h"

static const char *TAG = "SYS app";

SYS_cfg_Type g_SYS_Config = {
    .ESP32_board_ID = 0,
    .RS232_Baud = 115200,
    .RS485_Baud = 115200,
    .SYS_Baud = 460800,
    .AT4G_Baud = 115200,
    .RS485_Addr = 1,
    .RFID_Mode = 0,
    .RS232_Mode = 0,
    .RS485_Mode = 0,

    .WIFI_enable = 0,
    .RJ45_enable = 1,
    .AT4G_enable = 0,
    .Server_Switch = 1,
    .Client_Switch = 1,
    .HTTP_Switch = 1,
    .HTTPS_Switch = 1,
    .MQTT_Switch = 0,
    
    .WIFI_work_Mode = 1,
    .RJ45_work_Mode = 0,
    .RJ45_static_ip = "192.168.1.168",
    .RJ45_static_gw = "192.168.1.1",
    .RJ45_static_netmask = "255.255.255.0",

    .Net_Server_port = "8160",          //
    .Net_Client_ip = "192.168.1.128",
    .Net_Client_port = "9090",          //
    .AT4G_Client_ip = "192.168.1.128",
    .AT4G_Client_port = "8080",         //
    
    .HTTP_url = "http://192.168.1.128:9010/post",

    .Device_version_len = 0,
    .SYS_version_len = 0,
    
    .SYS_Rst = 0,
    .time.SYS_Sec = 0,
    .time.SYS_Us = 0,
    //
    .RJ45_online = 0,
    .WIFI_online = 0,
    .AT4G_online = 0,
    .SYS_online = 1,
    //
    .Connect_passage = 0,
    .SYS_Run_Mode = 0,
    .SYS_Run_Status = 0,
    //
    .Heartbeat_Run = 0,
    .Heartbeat_MAX = 10,
};

void system_cfg_memory_init(void)
{

}

void system_app_init(void)
{
    // Allow other core to finish initialization
    int temp_rtc = 0;
    //
    ESP_LOGI(TAG, "init -->");
    system_cfg_memory_init();

    if (g_SYS_Config.RJ45_enable)
    {
        ESP_LOGI(TAG,"enable eth");
        // Network_manage_set_mac (uint8_t *mac);
        eth_config_ip (g_SYS_Config.RJ45_work_Mode,
                    g_SYS_Config.RJ45_static_ip,
                    g_SYS_Config.RJ45_static_gw,
                    g_SYS_Config.RJ45_static_netmask);
        Network_manage_Init (0x02,g_SYS_Config.RJ45_enable);
    }
    if (g_SYS_Config.WIFI_enable)
    {
        ESP_LOGI(TAG,"enable wifi");
        wifi_config_user ("YXWL","yxwl6666");
        wifi_config_ip (g_SYS_Config.WIFI_work_Mode,
                    g_SYS_Config.WIFI_static_ip,
                    g_SYS_Config.WIFI_static_gw,
                    g_SYS_Config.WIFI_static_netmask);
        Network_manage_Init (1,g_SYS_Config.WIFI_enable);
    }
    tcp_server_link_config (g_SYS_Config.Net_Server_port,1,g_SYS_Config.Server_Switch);

    //
    // https_request_config_init ("POST",WEB_SERVER,WEB_URL,NULL,1);
    //
    custom_uart2_init(g_SYS_Config.SYS_Baud, 1);
    ESP_LOGI(TAG,"SYS Baud:%d",g_SYS_Config.SYS_Baud);

    ESP_LOGI(TAG, "init <--\n");
}

void system_rst(void)
{
    esp_restart();
}

void test_led_task(void *pvParam)
{
    int temp_num = 0;
    User_GPIO_config(0,2,1);
    TickType_t absolute_Time = 100;
    while (1)
    {
        temp_num++;
        
        vTaskDelay(pdMS_TO_TICKS(absolute_Time*10));
        User_GPIO_set(0,LED_T_IO, temp_num%2);
    }
    vTaskDelete(NULL); /*  基本不用退出 */
}
