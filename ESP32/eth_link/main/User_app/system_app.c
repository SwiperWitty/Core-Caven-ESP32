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
    .HTTP_Switch = 1,
    .HTTPS_Switch = 1,
    .MQTT_Switch = 0,

    .RJ45_work_Mode = 1,
    .WIFI_work_Mode = 0,
    .RJ45_static_ip = "192.168.0.200",
    .RJ45_static_gw = "192.168.0.1",
    .RJ45_static_netmask = "255.255.255.0",

    .Net_Server_port = "8160",          //
    .Net_Client_ip = "192.168.0.128",
    .Net_Client_port = "9090",          //
    .AT4G_Client_ip = "192.168.1.128",
    .AT4G_Client_port = "8080",         //
    
    .HTTP_url = "http://192.168.0.128:9010/post",

    .Device_version_len = 0,
    .SYS_version_len = 0,
    
    .SYS_Rst = 0,
    .time.SYS_Sec = 0,
    .time.time_us = 0,
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
    http_client_config_init (g_SYS_Config.HTTP_url,"20",g_SYS_Config.HTTP_Switch);
    //
    https_request_config_init ("POST",WEB_SERVER,WEB_URL,NULL,1);
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
        g_SYS_Config.time.SYS_Sec = temp_rtc;
        g_SYS_Config.time.time_us = xTaskGetTickCount() % 1000;
    }
    ESP_LOGI(TAG, "init <--\n");
}

void system_rst(void)
{
    esp_restart();
}

int custom_gpio_init(int set)
{
    int retval = 0;
#ifdef Exist_GPIO

#if (BOARD_NAME == ESP32_CAVEND)
    if (set)
    {
        User_GPIO_config(0,LED_T_IO,1);
        User_GPIO_config(0,KEY_IO,0);
        User_GPIO_config(0,POW_KILL_IO,1);

        LCD_POW_Set (1);
        POW_KILL_Set (0);
    }
    else
    {
        gpio_pad_select_gpio(LED_T_IO);
        gpio_set_direction(LED_T_IO, GPIO_MODE_INPUT_OUTPUT_OD);
    }
    retval = ESP_OK;
#elif (BOARD_NAME == EY1001)

#endif

    ESP_LOGI(TAG, "init (%d) ", retval);
#endif
    return retval;
}

int LED_Set(char n, int set)
{
    int retval = ESP_OK;
#ifdef Exist_GPIO
    #if (BOARD_NAME == ESP32_CAVEND)
    switch (n)
    {
    case 1:
        if (set)
        {
            gpio_set_level(LED_T_IO, 1);        // high open 
        }
        else
        {
            gpio_set_level(LED_T_IO, 0);
        }
        break;
    case 2:
        if (set)
        {
        }
        else
        {
        }
        break;

    default:
        retval = -1;
        break;
    }
    
    #elif (BOARD_NAME == EY1001)

    #endif

#endif
    return retval;
}

int KEY_Get (void)
{
    int retval = 0;
    retval = gpio_get_level(KEY_IO);
    return retval;
}

void LCD_POW_Set (int set)
{
    if (set)
    {
        gpio_set_level(LCD_POW_IO, 0);
    }
    else
    {
        gpio_set_level(LCD_POW_IO, 1);
    }
}

void POW_KILL_Set (int set)
{
    User_GPIO_set(0,POW_KILL_IO,set);
}

void test_led_task(void *pvParam)
{
    int temp_num = 0;
    custom_gpio_init(TURE); /* 简单的外设GPIO */
    TickType_t xLast_Time = xTaskGetTickCount();
    TickType_t absolute_Time = 100;
    while (1)
    {
        temp_num++;
        if (KEY_Get () == 0)
        {
            temp_num = 0;
            xLast_Time = xTaskGetTickCount();
            ESP_LOGI(TAG, "key get low");
            do{
                if ((xTaskGetTickCount() - xLast_Time) > 2000 && temp_num == 0)
                {
                    ESP_LOGI(TAG, "get off");
                    temp_num = 1;
                }
                vTaskDelay(pdMS_TO_TICKS(absolute_Time));
            }while (KEY_Get () == 0);
            if (temp_num)
            {
                vTaskDelay(pdMS_TO_TICKS(absolute_Time));
                POW_KILL_Set (1);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(absolute_Time*10));
        gpio_set_level(LED_T_IO, temp_num%2);
    }
    vTaskDelete(NULL); /*  基本不用退出 */
}
