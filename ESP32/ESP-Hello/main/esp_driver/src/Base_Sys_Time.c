#include "Base_Sys_Time.h"

/*
        SYS_Time
    功能在模块中实现，这里全是底层SDK
                                ———— 2022.7.7
*/

//* 底层 *//

#ifdef Exist_SYS_TIME
SYS_BaseTIME_Type Sys_BaseTIME;
TickType_t base_tick = 0;
#endif

void SYS_Time_Init(int Set)
{

#ifdef Exist_SYS_TIME

#endif
}

// Tick Controls
void SYS_Time_Set(SYS_BaseTIME_Type *time)
{
#ifdef Exist_SYS_TIME
    Sys_BaseTIME = *time;
    TickType_t tick_time;
    int temp_val;
    tick_time = xTaskGetTickCount();
    temp_val = tick_time / 1000;
    temp_val = time->SYS_Sec - temp_val;
    if (temp_val > 0)
    {
        base_tick = temp_val;
    }
    else
    {
        base_tick = 0;
    }
#endif
}

void SYS_Time_Get(SYS_BaseTIME_Type *time)
{
#ifdef Exist_SYS_TIME
    TickType_t tick_time;
    tick_time = xTaskGetTickCount();
    time->SYS_Sec = (tick_time / 1000) + base_tick;
    time->SYS_Us = (tick_time % 1000)*1000;
#endif
}

// dog
void SYS_IWDG_Configuration(void)
{
#ifdef Exist_SYS_TIME

#endif
}

void SYS_Feed_Watchdog(void)
{
#ifdef Exist_SYS_TIME

#endif
}

// delay
void SYS_Base_Delay(int time, int speed)
{
#ifdef NOP
    volatile int temp;
    for (int i = 0; i < time; ++i)
    {
        temp = speed; // SET
        do
        {
            NOP();
        } while ((temp--) > 0);
    }
#endif
}

void SYS_Delay_us(int n)
{
#ifdef Exist_SYS_TIME
    n = MIN(5000,n);
    vTaskDelay(pdMS_TO_TICKS(1));
#endif
}

void SYS_Delay_ms(int n)
{
#ifdef Exist_SYS_TIME
    n = MIN(5000,n);
    vTaskDelay(pdMS_TO_TICKS(n));
#endif
}

void SYS_Delay_S(int n)
{
    for (int var = 0; var < n; ++var)
    {
        SYS_Delay_ms(1000);
    }
}

// Interrupt Handler
#ifdef Exist_SYS_TIME


#endif
