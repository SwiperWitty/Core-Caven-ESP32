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

    .WIFI_enable = 1,
    .RJ45_enable = 1,
    .AT4G_enable = 0,
    .Server_Switch = 1,
    .Client_Switch = 1,
    .HTTP_Switch = 0,
    .MQTT_Switch = 0,

    .RJ45_work_Mode = 1,
    .WIFI_work_Mode = 0,
    .RJ45_static_ip = "192.168.1.200",
    .RJ45_static_gw = "192.168.1.1",
    .RJ45_static_netmask = "255.255.255.0",

    .Net_Server_port = "8160",
    .Net_Client_ip = "192.168.1.128",
    .Net_Client_port = "9090",
    .AT4G_Client_ip = "192.168.1.128",
    .AT4G_Client_port = "8080",
    
    .Device_version_len = 0,
    .SYS_version_len = 0,
    
    .SYS_Rst = 0,
    .SYS_utc_s = 0,
    .SYS_utc_ms = 0,
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
    uint32_t temp_rtc = 0;
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
        Network_manage_Init (0x02,1);
    }
    if (g_SYS_Config.WIFI_enable)
    {
        ESP_LOGI(TAG,"enable wifi");
        wifi_config_ip (g_SYS_Config.WIFI_work_Mode,
                        g_SYS_Config.WIFI_static_ip,
                        g_SYS_Config.WIFI_static_gw,
                        g_SYS_Config.WIFI_static_netmask);
        Network_manage_Init (0x01,1);
    }
    //
    tcp_client_link_config (g_SYS_Config.Net_Client_ip,g_SYS_Config.Net_Client_port,g_SYS_Config.Client_Switch);
    tcp_server_link_config (g_SYS_Config.Net_Server_port,g_SYS_Config.Server_Switch);
    //
    custom_uart1_init(g_SYS_Config.AT4G_Baud, 1);
    custom_uart2_init(g_SYS_Config.SYS_Baud, 1);
    ESP_LOGI(TAG,"SYS Baud:%d",g_SYS_Config.SYS_Baud);
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
    else
    {
        g_SYS_Config.SYS_utc_s = temp_rtc;
        g_SYS_Config.SYS_utc_ms = xTaskGetTickCount() % 1000;
    }
    ESP_LOGI(TAG, "init <--\n");
}

void system_rst(void)
{
    esp_restart();
}
