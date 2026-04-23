#include "center_app.h"
#include "show_app.h"
#include "Mode.h"

void Main_Init(void);

int app_main(void)
{
    Caven_BaseTIME_Type now_time;
	Task_Overtime_Type led_task;
	struct tm date;
    Main_Init();
    now_time.SYS_Sec = 1742299486;
    Mode_Use.TIME.Set_BaseTIME_pFun(now_time);
	Debug_printf("Hello world !");
	int led = 0;
	led_task.Switch = 1;
	led_task.Set_time.SYS_Sec = 0;
	led_task.Set_time.SYS_Us = 500000;
	while(1)
    {
		now_time = Mode_Use.TIME.Get_BaseTIME_pFun();
		date = API_UTC_Get_Date (now_time.SYS_Sec,8);
		API_Task_Timer (&led_task,now_time);
		User_GPIO_set(0,2,now_time.SYS_Sec % 2);
		if (led_task.Trigger_flag)
		{
			led_task.Trigger_flag = 0;
			led ++;
			// User_GPIO_set(0,22,led % 2);
			// Debug_printf("Begin_time %d:%d us,code:%d",led_task.Begin_time.SYS_Sec,led_task.Begin_time.SYS_Us,led_task.error_code);
			// Debug_printf("set_time %d:%d us",led_task.Set_time.SYS_Sec,led_task.Set_time.SYS_Us);
			// Debug_printf("utc %d:%d us",now_time.SYS_Sec,now_time.SYS_Us);
		}
		Mode_Use.TIME.Delay_Ms(1);
    }
	Mode_Use.TIME.Delay_Ms (250);
    SYS_RESET();
}

TaskHandle_t show_app_taskhanlde = NULL;

void Main_Init(void)
{
    Mode_Index();
	Mode_Init.TIME(ENABLE);
	Mode_Use.TIME.Delay_Ms (1);

	User_GPIO_config(0,2,WRITE_Config);
	User_GPIO_config(0,22,WRITE_Config);	// out1
	User_GPIO_config(0,25,READ_Config);		// ds18b20
	User_GPIO_config(0,36,READ_Config);		// key
	User_GPIO_set(0,2,0);
	User_GPIO_set(0,22,0);
	xTaskCreate(Show_app_Task, "task-[show_app]", 1024*8, NULL, SHOW_TASK_PRIORITY, &show_app_taskhanlde);
	Debug_printf("ESP_Hello !");
	// Center_app_Init ();
	// System_app_Init ();
}
