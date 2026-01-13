#include "Message_info_app.h"

static const char *TAG = "Message_info app";

Caven_BaseTIME_Type message_time = {0};

void debug_info_handle (void *data);
void server_info_handle (void *data);

void Message_info_task (void * empty)
{
    int retval = 0,get_State = 0;
    tcp_server_receive_State_Machine_Bind (server_info_handle);
	Caven_app_Init ();
	Task_Overtime_Type tcp_heart_task = {
    .Begin_time = {0},
    .Set_time.SYS_Sec = 5,
    .Set_time.SYS_Us = 0,
    .Switch = g_SYS_Config.TCPHBT_En,
	};
	Task_Overtime_Type http_heart_task = {
		.Begin_time = {0},
		.Set_time.SYS_Sec = 8,
		.Set_time.SYS_Us = 0,
		.Switch = 0,
		// .Switch = g_SYS_Config.HTTPHBT_En,
	};
    while (1)
    {
        message_time = g_SYS_Config.Now_time;
        /* code */
        get_State |= Caven_app_State_machine (message_time);

		API_Task_Timer (&tcp_heart_task,g_SYS_Config.Now_time);
		API_Task_Timer (&http_heart_task,g_SYS_Config.Now_time);
		if (tcp_heart_task.Trigger_Flag && 
			(g_SYS_Config.Connect_passage == TCP_Server_Link || g_SYS_Config.Connect_passage == TCP_Client_Link))
		{
			ESP_LOGI(TAG, "send tcp heart [%d]-[%d]",g_SYS_Config.Heartbeat_Run,g_SYS_Config.Heartbeat_num);
			Caven_app_send_heart_packet(g_SYS_Config.Heartbeat_Run,g_SYS_Config.Connect_passage);
			g_SYS_Config.Heartbeat_num ++;
			g_SYS_Config.Heartbeat_Run ++;
			if(g_SYS_Config.Heartbeat_num >= g_SYS_Config.Heartbeat_MAX)
			{
				ESP_LOGE(TAG, "send tcp heart over time !!! ,rst tcp !!!");
				if (g_SYS_Config.Connect_passage == TCP_Server_Link)
				{
					tcp_server_link_config (g_SYS_Config.TCPServer_port,g_SYS_Config.Server_break_off,0);
					tcp_server_link_config (g_SYS_Config.TCPServer_port,g_SYS_Config.Server_break_off,1);
				}
				else if (g_SYS_Config.Connect_passage == TCP_Client_Link)
				{
					tcp_client_link_config (g_SYS_Config.TCPClient_url,NULL,0);
					tcp_client_link_config (g_SYS_Config.TCPClient_url,NULL,1);
				}
				g_SYS_Config.Connect_passage = SYS_Link;
				g_SYS_Config.Heartbeat_num = 0;
			}
		}
		if (http_heart_task.Trigger_Flag)		// 这个就不在意返回
		{
			ESP_LOGI(TAG, "send http heart");
			http_cache_port_data_Fun ("caven http heart");
		}
		vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void debug_info_handle (void *data)
{
	uint8_t temp_data = *(uint8_t *)data;
	int temp_num = 0;
    temp_num = Caven_app_Make_pack (temp_data,SYS_Link,message_time);

	if(temp_num == 0xff)
	{
		g_SYS_Config.Connect_passage = SYS_Link;
	}
	else if (temp_num == -1)
	{
	}
}

void server_info_handle (void *data)
{
	uint8_t temp_data = *(uint8_t *)data;
	int temp_num = 0;
    temp_num = Caven_app_Make_pack (temp_data,TCP_Server_Link,message_time);
	if(temp_num == 0xff)
	{
		g_SYS_Config.Connect_passage = TCP_Server_Link;
	}
	else if (temp_num == -1)
	{
		ESP_LOGI(TAG,"server_info fail");
	}
}
