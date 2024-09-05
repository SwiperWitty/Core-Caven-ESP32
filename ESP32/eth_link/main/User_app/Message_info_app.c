#include "Message_info_app.h"

//
#define TRANSPOND_INFO  0
#define HANDLE_INFO     1
//
const char *TAG = "Message info app";
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

static uint8_t get_buff[0x1024];
static int get_num = 0;
static int get_time = 0;
static void rj45_get_fun (void *data)
{
    uint8_t rec = *(uint8_t *)data;
    int retval = 0;
    get_time = xTaskGetTickCount();
    if (Message_info.app_ID == TRANSPOND_INFO)
    {
        retval = caven_at_info_Make_packet_Fun(caven_at_info_standard, &caven_at_info_data, rec);
        if (retval == 0xff)
        {
            caven_at_info_data.Comm_way = 1;
        }
        else if ((get_num < sizeof(get_buff)) && retval == 0)
        {
            get_buff[get_num++] = rec;
        }
    }
    else
    {

    }
}

static uint8_t get2_buff[0x1024];
static int get2_num = 0;
static int get2_time = 0;
static void rj45_get2_fun (void *data)
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

static uint8_t get3_buff[0x1024];
static int get3_num = 0;
static int get3_time = 0;
static void uart2_get_fun (void *data)
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

static int Message_info_transpond (Caven_App_Type *message)
{
    int retval = 0;
    int temp_time = xTaskGetTickCount();
    int temp_num;

    if ((caven_at_info_data.Result & 0xF0) == caven_at_info_standard.Result)
    {
        ESP_LOGI(TAG,"get at cmd");
        ESP_LOGI(TAG,"data:%s",(char *)caven_at_info_data.p_Data);
        caven_at_info_packet_clean_Fun(&caven_at_info_data);
    }

    if (((temp_time - get_time) > 50 && get_num) || get_num > 300)
    {
        temp_num = get_num;
        memcpy(send_buff,get_buff,temp_num);
        get_num = 0;
        custom_uart2_send_data(send_buff, temp_num);
    }
    if (((temp_time - get2_time) > 50 && get2_num) || get2_num > 300)
    {
        temp_num = get2_num;
        memcpy(send_buff,get2_buff,temp_num);
        get2_num = 0;
        custom_uart2_send_data(send_buff, temp_num);
    }
    if(((temp_time - get3_time) > 10 && get3_num) || get3_num > 300)
    {
        temp_num = get3_num;
        memcpy(send_buff,get3_buff,temp_num);
        get3_num = 0;
        tcp_server_send_data(send_buff, temp_num);
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
    tcp_server_receive_State_Machine_Bind (rj45_get_fun);
    tcp_client_receive_State_Machine_Bind (rj45_get2_fun);
    custom_uart2_receive_State_Machine_Bind (uart2_get_fun);

    caven_at_info_packet_clean_Fun(&caven_at_info_data);
    caven_at_info_packet_index_Fun(&caven_at_info_data,at_info_data_buff[1]);
    
    while (1)
    {
        temp_time = xTaskGetTickCount();
        Message_info.Watch.time_us = temp_time % 1000;
        Message_info.Watch.SYS_Sec = 0;
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
        if (retval)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_restart();
        }
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}


