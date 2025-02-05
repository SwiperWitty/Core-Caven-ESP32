#include "Message_info_app.h"

//
#define TRANSPOND_INFO  0
#define HANDLE_INFO     1
//
static char *TAG = "Message info app";
static char *at_succ = "OK";                 // 成功
static char *at_fail = "FAIL";               // 失败
static char *at_Invalid = "Invalid command"; // 无效的

static Caven_App_Type Message_info = {
    .app_ID = HANDLE_INFO,
    .layer = 1,
    .cursor = 1,
    .string = NULL,
    .p_Data = NULL,
};
static Caven_at_info_packet_Type Caven_at_info_data;
Caven_at_info_packet_Type Caven_at_info_standard = {
    .Head = 0x4154,
    .End = 0x0D0A,
    .dSize = 300,
    .Result = 0x40,
};

/* 半字节CRC16(Dow_右移逆序) 0xA001 余式表*/
const unsigned int CRC_16_Tab[16] =
{
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};

int ModBusCRC16(int crc_start,unsigned char *data, unsigned char len)
{
    unsigned char i,temp;
    unsigned int crc_16 = crc_start & 0xffff;     //CRC寄存器初始值
    for(i=0;i<len;i++)
    {
    temp = ((unsigned char)(crc_16&0x000F))^(*data&0x0F);
    crc_16 >>= 4;
    crc_16 ^= CRC_16_Tab[temp];
    temp = ((unsigned char)(crc_16&0x000F))^(*data>>4);
    crc_16 >>= 4;
    crc_16 ^= CRC_16_Tab[temp];
    }
    return crc_16;
}

// int ModBusCRC16(int crc_start,unsigned char *data, int len)
// {
//     int i, j, temp, CRC16;

//     CRC16 = crc_start & 0xffff;     //CRC寄存器初始值
//     for (i = 0; i < len; i++)
//     {
//         CRC16 ^= data[i];
//         for (j = 0; j < 8; j++)
//         {
//             temp = (int)(CRC16 & 0x0001);
//             CRC16 >>= 1;
//             if (temp == 1)
//             {
//                 CRC16 ^= 0xA001;    //异或多项式
//             }
//         }
//     }
//     return CRC16;
// }

typedef struct
{
    unsigned short Head;    //2

    unsigned char Versions; //1
    unsigned char Type;     //1
    unsigned char Addr;     //1
    unsigned char Cmd;      //1
    unsigned char Cmd_sub;  //1

    unsigned short dSize;    //2
    unsigned char *p_Data;  // 

    unsigned char Result;   //1

    unsigned short Add_crc; //2
    unsigned short End_crc; //2

    int Get_num;
    int Run_status;
    unsigned char Comm_way;
}Caven_info_frame_Type; 
Caven_info_frame_Type Caven_standard = {
    .Head = 0xFA55,
    .Versions = 0x01,
    .dSize = 500,    // 最大长度
    .Result = 0x40,
};
Caven_info_frame_Type Caven_info_pack;
int Caven_info_Make_packet_Fun(Caven_info_frame_Type const standard, Caven_info_frame_Type *target, unsigned char data)
{
    int retval = 0;
    int temp = 0;
    Caven_info_frame_Type temp_packet = *target;
    unsigned char *tepm_pData = temp_packet.p_Data;

    if (temp_packet.Result & standard.Result) /* 目标有数据没处理 */
    {
        retval -= standard.Result;
        return retval;
    }
    if (target == NULL || temp_packet.p_Data == NULL)
    {
        retval -= standard.Result + 1;
        return retval;
    }
    if (temp_packet.Run_status > 0 && temp_packet.Run_status < 9)   // 跳过头帧和crc校验帧
    {
        temp_packet.Add_crc = ModBusCRC16(temp_packet.Add_crc,&data,1);
    }
    switch (temp_packet.Run_status)
    {
    case 0: /* Head */
        temp_packet.Head <<= 8;
        temp_packet.Head |= data;
        temp_packet.dSize = 0;
        temp_packet.Add_crc = 0xFFFF;   // !!!!
        temp_packet.End_crc = 0;
        if (temp_packet.Head == standard.Head)
        {
            temp_packet.Get_num = 0;
            tepm_pData[temp_packet.Get_num++] = temp_packet.Head >> 8;
            tepm_pData[temp_packet.Get_num++] = temp_packet.Head & 0xff;
            temp_packet.Run_status++;
        }
        break;
    case 1: /* Versions */
        tepm_pData[temp_packet.Get_num++] = data;
        temp_packet.Versions = data;
        if (temp_packet.Versions <= standard.Versions)
        {
            temp_packet.Run_status++;
        }
        else {
            temp_packet.Run_status = -temp_packet.Run_status;
        }
        break;
    case 2: /* Type */
        tepm_pData[temp_packet.Get_num++] = data;
        temp_packet.Type = data;
        temp_packet.Run_status++;
        break;
    case 3: /* Addr */
        tepm_pData[temp_packet.Get_num++] = data;
        temp_packet.Addr = data;
        temp_packet.Run_status++;
        break;
    case 4: /* Cmd */
        tepm_pData[temp_packet.Get_num++] = data;
        temp_packet.Cmd = data;
        temp_packet.Run_status++;
        break;
    case 5: /* Cmd_sub */
        tepm_pData[temp_packet.Get_num++] = data;
        temp_packet.Cmd_sub = data;
        temp_packet.Run_status++;
        break;
    case 6: /* dSize */
        tepm_pData[temp_packet.Get_num++] = data;
        temp_packet.dSize <<= 8;
        temp_packet.dSize |= data;
        temp = sizeof(temp_packet.Head) + sizeof(temp_packet.Versions) + sizeof(temp_packet.Type) + 
        sizeof(temp_packet.Addr) + sizeof(temp_packet.Cmd) + sizeof(temp_packet.Cmd_sub) + sizeof(temp_packet.dSize);       //9
        if (temp_packet.Get_num >= temp)
        {
            if (temp_packet.dSize > standard.dSize)
            {
                temp_packet.Run_status = -temp_packet.Run_status;
            }
            else if (temp_packet.dSize == 0){
                temp_packet.Run_status += 2; /* 0个 p_Data ，直接去 End_crc */
            }
            else{
                temp_packet.Run_status++;
            }
        }
        break;
    case 7: /* p_Data */
        tepm_pData[temp_packet.Get_num++] = data;
        temp = sizeof(temp_packet.Head) + sizeof(temp_packet.Versions) + sizeof(temp_packet.Type) + 
        sizeof(temp_packet.Addr) + sizeof(temp_packet.Cmd) + sizeof(temp_packet.Cmd_sub) + sizeof(temp_packet.dSize) + temp_packet.dSize; // 9 + dSize
        if (temp_packet.Get_num >= temp)
        {
            temp_packet.Run_status++;
        }
        break;
    case 8: /* Result */
        tepm_pData[temp_packet.Get_num++] = data;
        temp_packet.Result |= data;
        temp_packet.Run_status++;
        break;
    case 9: /* End_crc */
        tepm_pData[temp_packet.Get_num++] = data;
        temp_packet.End_crc <<= 8;
        temp_packet.End_crc |= data;
        temp = sizeof(temp_packet.Head) + sizeof(temp_packet.Versions) + sizeof(temp_packet.Type) + 
        sizeof(temp_packet.Addr) + sizeof(temp_packet.Cmd) + sizeof(temp_packet.Cmd_sub) + sizeof(temp_packet.dSize) + temp_packet.dSize + 
        sizeof(temp_packet.Result) + sizeof(temp_packet.End_crc);
        if (temp_packet.Get_num >= temp)
        {
            if (temp_packet.End_crc == temp_packet.Add_crc)
            {
                temp_packet.Result |= standard.Result; // crc successful
                temp_packet.Run_status = 0xff;
            }
            else {
                temp_packet.Run_status = -temp_packet.Run_status;
            }
        }
        break;
    default:
        break;
    }
    /*  结果    */
    if (temp_packet.Run_status < 0) // error
    {
        retval = temp_packet.Run_status;
        temp_packet.Run_status = 0;
        temp_packet.Head = 0;
        temp_packet.dSize = 0;
        temp_packet.Add_crc = 0;
        temp_packet.End_crc = 0;
        *target = temp_packet;
    }
    else if (temp_packet.Run_status == 0xff) // Successful
    {
        retval = temp_packet.Run_status;
        *target = temp_packet;
    }
    else // doing
    {
        retval = 0;
        *target = temp_packet;
    }
    return retval;
}

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
    // ESP_LOGI(TAG,"debug [%d]",retval);
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
        retval = caven_at_info_Make_packet_Fun(Caven_at_info_standard, &Caven_at_info_data, rec);
        if (retval == 0xff)
        {
            Caven_at_info_data.Comm_way = m_Connect_Client;
            client_num = 0;
        }
    }
    else
    {
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

int debug_num = 0;
static uint8_t uart2_buff[0x1024];
static uint8_t uart2_buff_s[0x1024];
Caven_cache_Type uart2_cache = {0};
static void uart2_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;
    int retval = 0;
    uart2_cache.time = xTaskGetTickCount();
    if (uart2_cache.cache_max == 0)
    {
        uart2_cache.p_buff_a = uart2_buff;
        uart2_cache.p_buff_b = uart2_buff_s;
        uart2_cache.len_a = 0;
        uart2_cache.len_b = 0;
        uart2_cache.cache_max = 1024;
        uart2_cache.time_out = 10;
        uart2_cache.len_max = sizeof(uart2_buff);
        if (uart2_cache.cache_max > uart2_cache.len_max)
        {
            uart2_cache.cache_max = uart2_cache.len_max;
        }
    }
    if (Message_info.app_ID == TRANSPOND_INFO)
    {
    }
    else
    {
    }
    if (retval == 0)
    {
        if ((uart2_cache.len_a < uart2_cache.len_max) && uart2_cache.flag == 0)
        {
            *((uint8_t *)uart2_cache.p_buff_a + (uart2_cache.len_a++)) = rec;
            debug_num ++;
        }
        else if ((uart2_cache.len_b < uart2_cache.len_max) && uart2_cache.flag)
        {
            *((uint8_t *)uart2_cache.p_buff_b + (uart2_cache.len_b++)) = rec;
            debug_num ++;
        }
        else
        {
            ESP_LOGI(TAG,"uart2 over len");
        }
    }
}

// 发送at指令消息
static void Message_info_send_packet_fun (Caven_at_info_packet_Type data)
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

    if ((Caven_at_info_data.Result & 0xF0) == Caven_at_info_standard.Result)
    {
        char *p_temp_str = (char *)Caven_at_info_data.p_Data;
        int temp_len = strlen(p_temp_str);

        ESP_LOGI(TAG,"get at cmd,from [%d]",Caven_at_info_data.Comm_way);
        Message_info_send_packet_fun (Caven_at_info_data);

        if((strcmp(p_temp_str,"\r\n") == 0) && temp_len == 2)
        {
            Message_info_send_result_fun(at_succ,Caven_at_info_data.Comm_way);
            ESP_LOGI(TAG,"uart1[%d],uart2[%d],get[%d]",uart1_task_num,uart2_task_num,debug_num);
            uart1_task_num = 0;
            uart2_task_num = 0;
            debug_num = 0;
        }
        else if (p_temp_str[0] == '+')
        {
            Message_info_send_result_fun(at_succ,Caven_at_info_data.Comm_way);
        }
        else if (p_temp_str[0] == 'E')
        {
            Message_info_send_result_fun(at_fail,Caven_at_info_data.Comm_way);
        }
        else
        {
            Message_info_send_result_fun(at_Invalid,Caven_at_info_data.Comm_way);
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
    if (uart2_cache.flag == 0)
    {
        if(((temp_time - uart2_cache.time) > uart2_cache.time_out && uart2_cache.len_a) || uart2_cache.len_a > uart2_cache.cache_max)
        {
            uart2_cache.flag = 1;
            temp_num = uart2_cache.len_a;
            if (uart2_cache.p_buff_a != NULL)
            {
                memcpy(send_buff,uart2_cache.p_buff_a,temp_num);
            }
            uart2_cache.len_a = 0;
            tcp_server_send_data(send_buff, temp_num);
        }
    }
    else
    {
        if(((temp_time - uart2_cache.time) > uart2_cache.time_out && uart2_cache.len_b) || uart2_cache.len_b > uart2_cache.cache_max)
        {
            uart2_cache.flag = 0;
            temp_num = uart2_cache.len_b;
            if (uart2_cache.p_buff_b != NULL)
            {
                memcpy(send_buff,uart2_cache.p_buff_b,temp_num);
            }
            uart2_cache.len_b = 0;
            tcp_server_send_data(send_buff, temp_num);
        }
    }

    return retval;
}
uint8_t caven_buff[500];
static int Message_info_handle (Caven_App_Type *message)
{
    int retval = 0;
    int temp_time = xTaskGetTickCount();
    uint8_t temp_array[300];
    if (Caven_info_pack.p_Data == NULL)
    {
        Caven_info_pack.p_Data = caven_buff;
    }
    
    if (Caven_info_pack.Result & Caven_standard.Result)
    {
        ESP_LOGW(TAG,"get Caven_info_pack,from [%d]",Caven_info_pack.Comm_way);
        ESP_LOGI(TAG,"Caven_info_pack Head[%X]",Caven_info_pack.Head);
        ESP_LOGI(TAG,"Caven_info_pack Type[%X]",Caven_info_pack.Type);
        ESP_LOGI(TAG,"Caven_info_pack Addr[%X]",Caven_info_pack.Addr);
        ESP_LOGI(TAG,"Caven_info_pack CMD[%X]",Caven_info_pack.Cmd);
        ESP_LOGI(TAG,"Caven_info_pack CMD_SUB[%X]",Caven_info_pack.Cmd_sub);
        ESP_LOGI(TAG,"Caven_info_pack len[%X]",Caven_info_pack.dSize);
        ESP_LOGI(TAG,"Caven_info_pack Result[%X]",Caven_info_pack.Result & 0x80);
        memcpy(temp_array,&Caven_info_pack.p_Data[9],Caven_info_pack.dSize);
        for (int i = 0; i < Caven_info_pack.dSize; i++)
        {
            printf("0X%02X ",temp_array[i]);
        }
        printf("\r\n");
        ESP_LOGI(TAG,"Caven_info_pack end \n");
        Caven_info_pack.Result = 0;
        Caven_info_pack.Run_status = 0;
    }
    
    return retval;
}



void Message_info_task (void * empty)
{
    int temp_time;
    int retval = 0;
    uint8_t at_info_data_buff[5][500];
    
    Message_info.app_ID = TRANSPOND_INFO;
    tcp_server_receive_State_Machine_Bind (rj45_server_get_fun);
    tcp_client_receive_State_Machine_Bind (rj45_client_get_fun);
    custom_uart1_receive_State_Machine_Bind (uart2_get_fun);
    custom_uart2_receive_State_Machine_Bind (uart2_get_fun);

    caven_at_info_packet_clean_Fun(&Caven_at_info_data);
    caven_at_info_packet_index_Fun(&Caven_at_info_data,at_info_data_buff[1]);
    
    while (1)
    {
        temp_time = xTaskGetTickCount() % 1000;
        g_SYS_Config.SYS_utc_ms = temp_time;
        Message_info.Watch.time_us = g_SYS_Config.SYS_utc_ms * 1000;
        Message_info.Watch.SYS_Sec = g_SYS_Config.SYS_utc_s;
        switch (Message_info.app_ID)
        {
        case TRANSPOND_INFO:
            retval = Message_info_transpond (&Message_info);
            break;
        case HANDLE_INFO:
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


