#ifndef _BASE_IIC_H__
#define _BASE_IIC_H__

#include "Items.h"
#include "Base_GPIO.h"

/****************/

/*
                                                                                                                                                           

*/


// 选择输出模式
#ifdef Exist_IIC
    #define IIC_SOFTWARE        1   // 屏蔽就是硬件模式
    #define IIC_SPEED   4
    #if IIC_SOFTWARE == 0
        #define IIC_DMA     1
        #define RTC_I2C_MASTER_FREQ_HZ  400000
    #endif
    #define HOST_MODE
#endif

#ifdef IIC_SOFTWARE                                 // 软件
    #define IIC_MODE_IN     READ_Config
    #define IIC_MODE_OUT    WRITE_Config
#else                                               // 硬件
    #define IIC_MODE_IN     READ_Config
    #define IIC_MODE_OUT    WRITE_Config
#endif

//SPI1
#define IIC_SDA        16
#define IIC_SCL        17
#define GPIO_IIC       0


/*    驱动层      */

int Base_IIC_Init(int Set);

char Base_IIC_Send_DATA(char Addr, const uint8_t *Data, char NeedAck, int Length, int Speed, char Continuous);
char Base_IIC_Receive_DATA(char Addr,uint8_t *Data,char NeedAck,int Length,int Speed);


/*******************/


#endif
