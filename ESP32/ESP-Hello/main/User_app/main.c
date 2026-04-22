#include "center_app.h"
#include "Mode.h"

void Main_Init(void);

int app_main(void)
{
    Caven_BaseTIME_Type now_time;
	
    Main_Init();
    now_time.SYS_Sec = 1742299486;
    Mode_Use.TIME.Set_BaseTIME_pFun(now_time);
	
	while(1)
    {
		now_time = Mode_Use.TIME.Get_BaseTIME_pFun();
		User_GPIO_set(0,2,now_time.SYS_Sec % 2);
		Mode_Use.TIME.Delay_Ms(200);
		Debug_printf("Hello world !");
    }
	Mode_Use.TIME.Delay_Ms (250);
    SYS_RESET();
}

void Main_Init(void)
{
    Mode_Index();
	Mode_Init.TIME(ENABLE);
	Mode_Use.TIME.Delay_Ms (1);

	User_GPIO_config(0,2,WRITE_Config);
	User_GPIO_set(0,2,0);

	Debug_printf("ESP_Hello !");
	// Center_app_Init ();
	// System_app_Init ();
}


