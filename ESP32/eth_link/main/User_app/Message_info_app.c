#include "Message_info_app.h"

//
#define TRANSPOND_INFO  0
#define HANDLE_INFO     1
//
static char *TAG = "Message info app";
static char *at_succ = "OK";                 // 成功
static char *at_fail = "FAIL";               // 失败
static char *at_Invalid = "Invalid command"; // 无效的

static Caven_App_Type Message_info;
static caven_at_info_packet_Type caven_at_info_data;
caven_at_info_packet_Type caven_at_info_standard = {
    .Head = 0x4154,
    .End = 0x0D0A,
    .dSize = 300,
    .Result = 0x40,
};

//
static uint8_t send_buff[0x1024];

static uint8_t server_buff[0x1024];
static int server_num = 0;
static int server_time = 0;
static void rj45_server_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;
    int retval = 0;
    server_time = xTaskGetTickCount();
    if (Message_info.app_ID == TRANSPOND_INFO)
    {
        retval = caven_at_info_Make_packet_Fun(caven_at_info_standard, &caven_at_info_data, rec);
        // ESP_LOGI(TAG,"debug [%x]",retval);
        if (retval == 0xff)
        {
            caven_at_info_data.Comm_way = m_Connect_Server;
            server_num = 0;
        }
        else if (retval == 0)
        {
            if (server_num < sizeof(server_buff))
            {
                server_buff[server_num++] = rec;
            }
            else
            {
                ESP_LOGI(TAG,"server over len");
            }
        }
    }
    else
    {
        
    }
}

static uint8_t client_buff[0x1024];
static int client_num = 0;
static int client_time = 0;
static void rj45_client_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;
    int retval = 0;
    client_time = xTaskGetTickCount();
    if (Message_info.app_ID == TRANSPOND_INFO)
    {
        retval = caven_at_info_Make_packet_Fun(caven_at_info_standard, &caven_at_info_data, rec);
        if (retval == 0xff)
        {
            caven_at_info_data.Comm_way = m_Connect_Client;
            client_num = 0;
        }
        else if (retval == 0)
        {
            if (client_num < sizeof(client_buff))
            {
                client_buff[client_num++] = rec;
            }
            else
            {
                ESP_LOGI(TAG,"client over len");
            }
        }
    }
    else
    {
    }
}

int debug_num = 0;
static char standby_flag = 0;
static uint8_t uart2_buff[0x1024];
static int uart2_num = 0;
static uint8_t uart2_buff_s[0x1024];
static int uart2_num_s = 0;
static int uart2_time = 0;
static void uart2_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;
    int retval = 0;
    uart2_time = xTaskGetTickCount();
    
    if (Message_info.app_ID == TRANSPOND_INFO)
    {
        // retval = caven_at_info_Make_packet_Fun(caven_at_info_standard, &caven_at_info_data, rec);
        if (retval == 0xff)
        {
            caven_at_info_data.Comm_way = m_Connect_SYS;
            uart2_num = 0;
        }
        else if (retval == 0)
        {
            if ((uart2_num < sizeof(uart2_buff)) && standby_flag == 0)
            {
                uart2_buff[uart2_num++] = rec;
                debug_num ++;
            }
            else if ((uart2_num_s < sizeof(uart2_buff_s)) && standby_flag)
            {
                uart2_buff_s[uart2_num_s++] = rec;
                debug_num ++;
            }
            else
            {
                ESP_LOGI(TAG,"uart2 over len");
            }
        }
    }
    else
    {
    }
}

// 发送at指令消息
static void Message_info_send_packet_fun (caven_at_info_packet_Type data)
{
    int temp_len;
    uint8_t temp_str[512];
    temp_len = caven_at_info_Split_packet_Fun(data, temp_str);
    switch (data.Comm_way)
    {
    case m_Connect_SYS:
        custom_uart2_send_data(temp_str, temp_len);
        break;
    case m_Connect_Server:
        tcp_server_send_data(temp_str, temp_len);
        break;
    case m_Connect_Client:
        tcp_client_send_data(temp_str, temp_len);
        break;
    case m_Connect_BLE:
        break;
    default:
        break;
    }
}

// 发送at指令结果
static void Message_info_send_result_fun (char *data,int way)
{
    int temp_len = 0;
    uint8_t temp_str[512];
    if (data == NULL)
    {
        return;
    }
    temp_len = strlen(data);
    memcpy(temp_str,data,temp_len);
    memcpy(&temp_str[temp_len],"\r\n",2);
    temp_len += 2;
    switch (way)
    {
    case m_Connect_SYS:
        custom_uart2_send_data(temp_str, temp_len);
        break;
    case m_Connect_Server:
        tcp_server_send_data(temp_str, temp_len);
        break;
    case m_Connect_Client:
        tcp_client_send_data(temp_str, temp_len);
        break;
    case m_Connect_BLE:
        break;
    default:
        break;
    }
}

extern int uart1_task_num;
extern int uart2_task_num;
static int Message_info_transpond (Caven_App_Type *message)
{
    int retval = 0;
    int temp_time = xTaskGetTickCount();
    int temp_num;

    if ((caven_at_info_data.Result & 0xF0) == caven_at_info_standard.Result)
    {
        char *p_temp_str = (char *)caven_at_info_data.p_Data;
        int temp_len = strlen(p_temp_str);

        ESP_LOGI(TAG,"get at cmd");
        Message_info_send_packet_fun (caven_at_info_data);

        if((strcmp(p_temp_str,"\r\n") == 0) && temp_len == 2)
        {
            Message_info_send_result_fun(at_succ,caven_at_info_data.Comm_way);
            ESP_LOGI(TAG,"uart1[%d],uart2[%d],get[%d]",uart1_task_num,uart2_task_num,debug_num);
            uart1_task_num = 0;
            uart2_task_num = 0;
            debug_num = 0;
        }
        else if (p_temp_str[0] == '+')
        {
            Message_info_send_result_fun(at_succ,caven_at_info_data.Comm_way);
        }
        else if (p_temp_str[0] == 'E')
        {
            Message_info_send_result_fun(at_fail,caven_at_info_data.Comm_way);
        }
        else
        {
            Message_info_send_result_fun(at_Invalid,caven_at_info_data.Comm_way);
        }
        caven_at_info_packet_clean_Fun(&caven_at_info_data);
    }

    if (((temp_time - server_time) > 50 && server_num) || server_num > 300)
    {
        temp_num = server_num;
        memcpy(send_buff,server_buff,temp_num);
        // ESP_LOGI(TAG,"server transpond");
        server_num = 0;
        custom_uart1_send_data(send_buff, temp_num);
        custom_uart2_send_data(send_buff, temp_num);
    }
    if (((temp_time - client_time) > 50 && client_num) || client_num > 300)
    {
        temp_num = client_num;
        memcpy(send_buff,client_buff,temp_num);
        ESP_LOGI(TAG,"client transpond");
        client_num = 0;
        custom_uart1_send_data(send_buff, temp_num);
        custom_uart2_send_data(send_buff, temp_num);
    }
    if (standby_flag == 0)
    {
        if(((temp_time - uart2_time) > 10 && uart2_num) || uart2_num > 1024)
        {
            standby_flag = 1;
            temp_num = uart2_num;
            memcpy(send_buff,uart2_buff,temp_num);
            uart2_num = 0;
            tcp_server_send_data(send_buff, temp_num);
        }
    }
    else
    {
        if(((temp_time - uart2_time) > 10 && uart2_num_s) || uart2_num_s > 1024)
        {
            standby_flag = 0;
            temp_num = uart2_num_s;
            memcpy(send_buff,uart2_buff_s,temp_num);
            uart2_num_s = 0;
            tcp_server_send_data(send_buff, temp_num);
        }
    }

    return retval;
}

static int Message_info_handle (Caven_App_Type *message)
{
    int retval = 0;
    int temp_time = xTaskGetTickCount();
    int temp_num;

    return retval;
}

uint8_t at_info_data_buff[5][500];

void Message_info_task (void * empty)
{
    int temp_time;
    int retval = 0;

    Message_info.app_ID = g_SYS_Config.SYS_Run_Mode;
    tcp_server_receive_State_Machine_Bind (rj45_server_get_fun);
    tcp_client_receive_State_Machine_Bind (rj45_client_get_fun);
    custom_uart1_receive_State_Machine_Bind (uart2_get_fun);
    custom_uart2_receive_State_Machine_Bind (uart2_get_fun);

    caven_at_info_packet_clean_Fun(&caven_at_info_data);
    caven_at_info_packet_index_Fun(&caven_at_info_data,at_info_data_buff[1]);
    
    while (1)
    {
        temp_time = xTaskGetTickCount() % 1000;
        g_SYS_Config.SYS_utc_ms = temp_time;
        Message_info.Watch.time_us = g_SYS_Config.SYS_utc_ms * 1000;
        Message_info.Watch.SYS_Sec = g_SYS_Config.SYS_utc_s;
        switch (Message_info.app_ID)
        {
        case 0:
            retval = Message_info_transpond (&Message_info);
            break;
        case 1:
            retval = Message_info_handle (&Message_info);
            break;
        default:
            break;
        }

        g_SYS_Config.WIFI_online = wifi_get_local_ip_status(NULL,NULL,NULL);
        g_SYS_Config.RJ45_online = eth_get_local_ip_status(NULL,NULL,NULL);
        g_SYS_Config.AT4G_online = 0;
        g_SYS_Config.SYS_online = 1;        // 恒为1
        retval |= g_SYS_Config.SYS_Rst;
        if (retval)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_restart();
        }
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}


