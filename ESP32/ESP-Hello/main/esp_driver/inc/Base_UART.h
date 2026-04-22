/*
 * Base_UART.h
 * 需要规定什么使用宏函数，什么时候直接用函数。
 * 但是他们调用的时候是一样的，所以单纯在Base里面纠结。
 * 
为了方便上层统一调用，本文件需要提供以下函数

int Base_UART_Init(UART_mType Channel,int Baud,int Set);
void Base_UART_Send_Data(UART_mType Channel, uint16_t Data)

int State_Machine_Bind (UART_mType Channel,D_pFun UART_pFun);

 */
#ifndef _Base_UART__H_
#define _Base_UART__H_

#include "Items.h"          // 默认功能

#include "Caven_Type.h"

typedef enum
{
    m_UART_CH0 = 0,
    m_UART_CH1 = 1,
    m_UART_CH2,
    m_UART_CH3,
    m_UART_CH4,
    m_UART_CH5,
}UART_mType;

#ifdef Exist_UART
    #define DMA_UART        0
    #define UART_BUFF_MAX   500
#endif

// fun
int Base_UART_Init(UART_mType Channel,int Baud,int Set);
void Base_UART_Send_Data(UART_mType Channel,uint16_t Data);
void Base_UART_DMA_Send_Buff(UART_mType Channel,const uint8_t *Data,int Length);

/*
 * 接收入口的状态机，这个很重要
 * 在初始化之后执行一次
 * 不可用阻塞
 */
int State_Machine_Bind(UART_mType Channel,D_pFun UART_pFun);

#endif
