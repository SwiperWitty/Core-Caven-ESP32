#include "Message_info_app.h"

//
#define TRANSPOND_INFO  0
#define HANDLE_INFO     1
//
static char *TAG = "Message info app";
static char *at_succ = "OK";                 // 成功
static char *at_fail = "FAIL";               // 失败
static char *at_Invalid = "Invalid command"; // 无效的

char Message_show_str[200];

static Caven_App_Type Message_info = {
    .app_ID = HANDLE_INFO,
    .layer = 1,
    .cursor = 1,
    .string = NULL,
    .p_Data = Message_show_str,
    .str_switch = 1,
};
static Caven_at_info_packet_Type Caven_at_info_data;
Caven_at_info_packet_Type Caven_at_info_standard = {
    .Head = 0x4154,
    .End = 0x0D0A,
    .dSize = 300,
    .Result = 0x40,
};

Caven_info_packet_Type Caven_standard = {
    .Head = 0xFA55,
    .Versions = 0x01,
    .Addr = 1,
    .dSize = 300,    // 最大长度
    .Result = 0x50,
};
Caven_info_packet_Type Caven_info_pack;

static uint8_t send_buff[1024];

static uint8_t server_buff[1024];
static int server_num = 0;
static int server_time = 0;
static void rj45_server_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;
    int retval = 0;
    server_time = xTaskGetTickCount();
    if (Message_info.app_ID == TRANSPOND_INFO)
    {
        retval = caven_at_info_Make_packet_Fun(Caven_at_info_standard, &Caven_at_info_data, rec);
        if (retval == 0xff)
        {
            Caven_at_info_data.Comm_way = m_Connect_Server;
            server_num = 0;
        }
    }
    else
    {
        retval = Caven_info_Make_packet_Fun(Caven_standard, &Caven_info_pack, rec);
        if (retval == 0xff)
        {
            Caven_info_pack.Comm_way = m_Connect_Server;
            server_num = 0;
        }
        else if (retval < 0)
        {
            ESP_LOGW(TAG,"caven pack error[%d]",retval);
        }
    }
    if (retval == 0)
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

static uint8_t client_buff[1024];
static int client_num = 0;
static int client_time = 0;
static void rj45_client_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;
    int retval = 0;
    client_time = xTaskGetTickCount();
    if (Message_info.app_ID == TRANSPOND_INFO)
    {
        retval = caven_at_info_Make_packet_Fun(Caven_at_info_standard, &Caven_at_info_data, rec);
        if (retval == 0xff)
        {
            Caven_at_info_data.Comm_way = m_Connect_Client;
            client_num = 0;
        }
    }
    else
    {
        retval = Caven_info_Make_packet_Fun(Caven_standard, &Caven_info_pack, rec);
        if (retval == 0xff)
        {
            Caven_info_pack.Comm_way = m_Connect_Client;
            client_num = 0;
        }
        else if (retval < 0)
        {
            ESP_LOGW(TAG,"caven pack error[%d]",retval);
        }
    }
    if (retval == 0)
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


static void uart2_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;

}

// 发送caven消息
static void Message_info_send_Caven_packet_fun (Caven_info_packet_Type data)
{
    int temp_len;
    uint8_t temp_data[512];
    temp_len = Caven_info_Split_packet_Fun(data, temp_data);
    switch (data.Comm_way)
    {
    case m_Connect_SYS:
        custom_uart2_send_data(temp_data, temp_len);
        break;
    case m_Connect_Server:
        tcp_server_send_data(temp_data, temp_len);
        break;
    case m_Connect_Client:
        tcp_client_send_data(temp_data, temp_len);
        break;
    case m_Connect_BLE:
        break;
    default:
        break;
    }
}

// 发送at pack
static void Message_info_send_AT_packet_fun (Caven_at_info_packet_Type data)
{
    int temp_len;
    uint8_t temp_data[512];
    temp_len = caven_at_info_Split_packet_Fun(data, temp_data);
    switch (data.Comm_way)
    {
    case m_Connect_SYS:
        custom_uart2_send_data(temp_data, temp_len);
        break;
    case m_Connect_Server:
        tcp_server_send_data(temp_data, temp_len);
        break;
    case m_Connect_Client:
        tcp_client_send_data(temp_data, temp_len);
        break;
    case m_Connect_BLE:
        break;
    default:
        break;
    }
}

// 发送at指令
static void Message_info_send_AT_CMD_fun (char *data,int way)
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
int debug_num;
static int Message_info_transpond (Caven_App_Type *message)
{
    int retval = 0;
    int temp_time = xTaskGetTickCount();
    int temp_num;

    if ((Caven_at_info_data.Result & 0xF0) == Caven_at_info_standard.Result)
    {
        char *p_temp_str = (char *)Caven_at_info_data.p_Data;
        int temp_len = strlen(p_temp_str);

        ESP_LOGI(TAG,"get at cmd,from [%d]",Caven_at_info_data.Comm_way);
        Message_info_send_AT_packet_fun (Caven_at_info_data);

        if((strcmp(p_temp_str,"\r\n") == 0) && temp_len == 2)
        {
            Message_info_send_AT_CMD_fun(at_succ,Caven_at_info_data.Comm_way);
            ESP_LOGI(TAG,"uart1[%d],uart2[%d],get[%d]",uart1_task_num,uart2_task_num,debug_num);
            uart1_task_num = 0;
            uart2_task_num = 0;
            debug_num = 0;
        }
        else if (p_temp_str[0] == '+')
        {
            Message_info_send_AT_CMD_fun(at_succ,Caven_at_info_data.Comm_way);
        }
        else if (p_temp_str[0] == 'E')
        {
            Message_info_send_AT_CMD_fun(at_fail,Caven_at_info_data.Comm_way);
        }
        else
        {
            Message_info_send_AT_CMD_fun(at_Invalid,Caven_at_info_data.Comm_way);
        }
        caven_at_info_packet_clean_Fun(&Caven_at_info_data);
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

    return retval;
}
uint8_t caven_buff[500];
static int Message_info_handle (Caven_App_Type *message)
{
    int retval = 0;
    uint8_t temp_array[300];
    Caven_info_packet_Type temp_pack = *(Caven_info_packet_Type *)message->p_Data;
    
    if (temp_pack.Result & Caven_standard.Result)
    {
    #if 1
        ESP_LOGW(TAG,"get Caven_info_pack,from [%d]",temp_pack.Comm_way);
        ESP_LOGI(TAG,"Caven_info_pack Head[%X]",temp_pack.Head);
        ESP_LOGI(TAG,"Caven_info_pack Type[%X]",temp_pack.Type);
        ESP_LOGI(TAG,"Caven_info_pack Addr[%X]",temp_pack.Addr);
        ESP_LOGI(TAG,"Caven_info_pack CMD[%X]",temp_pack.Cmd);
        ESP_LOGI(TAG,"Caven_info_pack CMD_SUB[%X]",temp_pack.Cmd_sub);
        ESP_LOGI(TAG,"Caven_info_pack len[%X]",temp_pack.dSize);
        ESP_LOGI(TAG,"Caven_info_pack Result[%X]",temp_pack.Result & 0x0f);
        ESP_LOGI(TAG,"Caven_info_pack data:");
        memcpy(temp_array,temp_pack.p_Data,temp_pack.dSize);
        for (int i = 0; i < temp_pack.dSize; i++)
        {
            printf("0X%02X ",temp_array[i]);
        }
        printf("\r\n");
        ESP_LOGI(TAG,"Caven_info_pack all_data[%p] p_Data[%p]",temp_pack.p_AllData,temp_pack.p_Data);
        ESP_LOGI(TAG,"Caven_info_pack end \n");
    #endif
        Message_info_send_Caven_packet_fun (temp_pack);
        Caven_info_packet_fast_clean_Fun((Caven_info_packet_Type *)message->p_Data);
    }
    
    return retval;
}



void Message_info_task (void * empty)
{
    int temp_time;
    int retval = 0;
    uint8_t at_info_data_buff[5][500];
    
    Message_info.app_ID = HANDLE_INFO;
    tcp_server_receive_State_Machine_Bind (rj45_server_get_fun);
    tcp_client_receive_State_Machine_Bind (rj45_client_get_fun);
    custom_uart1_receive_State_Machine_Bind (uart2_get_fun);
    custom_uart2_receive_State_Machine_Bind (uart2_get_fun);

    caven_at_info_packet_clean_Fun(&Caven_at_info_data);
    caven_at_info_packet_index_Fun(&Caven_at_info_data,at_info_data_buff[1]);

    Caven_info_packet_fast_clean_Fun(&Caven_info_pack);
    Caven_info_packet_index_Fun(&Caven_info_pack, at_info_data_buff[2]);

    ESP_LOGW(TAG,"work mode app_ID[%d]",Message_info.app_ID);
    while (1)
    {
        Message_info.Watch = g_SYS_Config.time;
        
        switch (Message_info.app_ID)
        {
        case TRANSPOND_INFO:
            Message_info.p_Data = server_buff;
            retval = Message_info_transpond (&Message_info);
            break;
        case HANDLE_INFO:
            Message_info.p_Data = &Caven_info_pack;
            retval = Message_info_handle (&Message_info);
            break;
        default:
            break;
        }

        retval |= g_SYS_Config.SYS_Rst;
        if (retval)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_restart();
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}


