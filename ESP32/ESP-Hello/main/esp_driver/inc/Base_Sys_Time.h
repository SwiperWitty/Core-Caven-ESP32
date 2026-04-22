/*
 *
本文件属于MCU专属文件，每个MCU不同，这里的内容就会不同 ———— 需要系统提供时钟
这个文件是给Time的，也就是说你得找一个能满足以下条件时钟来作为动力  （选一个幸运儿做苦力）。
-滴答定时器（首选） ch32v203的滴滴是64位，且可以向上计数
-通用定时器

为了方便上层统一调用，本文件需要提供以下函数
void SYS_Time_Init (int Set);

void SYS_Set_Tick (SYS_BaseTIME_Type * time);
void SYS_Get_Tick (SYS_BaseTIME_Type * time);

// Delay
void SYS_Base_Delay (int time,int Speed);

void SYS_Delay_us (int n);
void SYS_Delay_ms (int n);

//dog 函数内容可以为空
void SYS_IWDG_Configuration (void);
void SYS_Feed_Watchdog (void);

*/

#ifndef BASE_SYS_TIME_H__
#define BASE_SYS_TIME_H__



#include "Items.h"          // 默认功能

#include "Caven_Type.h"



/* 【宏函数群】   */

#ifdef Exist_SYS_TIME


#endif
/*  end   */



// 很长的时间戳
typedef struct
{
    volatile uint32_t SYS_Time_H;   // 每Frequency进1
    volatile uint32_t SYS_Time_L;   // 24bit 的
}SYS_Time_Type;
// 系统运行总时长 8byte
typedef struct
{
    volatile uint32_t SYS_Sec;
    volatile uint32_t SYS_Us;       // 这里最大 1000 000
}SYS_BaseTIME_Type;

void SYS_Time_Init(int Set);

//
void SYS_Time_Set(SYS_BaseTIME_Type * time);
void SYS_Time_Get(SYS_BaseTIME_Type * time);

// Delay
void SYS_Base_Delay(int time, int speed);

void SYS_Delay_us(int n);
void SYS_Delay_ms(int n);

//dog
void SYS_IWDG_Configuration(void);
void SYS_Feed_Watchdog(void);

#endif
