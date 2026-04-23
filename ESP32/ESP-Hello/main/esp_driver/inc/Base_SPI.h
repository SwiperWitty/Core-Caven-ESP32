#ifndef _BASE_SPI_H__
#define _BASE_SPI_H__

#include "Items.h"
#include "Base_GPIO.h"
#include "string.h"

/****************/

/*
                                                                                                                                                           

*/

typedef enum
{
    m_SPI_CH0 = 0,
    m_SPI_CH1 = 1,
    m_SPI_CH2,
    m_SPI_CH3,
}SPI_mType;

// 选择输出模式
#ifdef Exist_SPI
    #define SPI_SOFTWARE                    // 屏蔽就是硬件模式
    #define SPI_SPEED   4
    #ifndef SPI_SOFTWARE
        #define SPI_DMA			            
    #endif
    #define HOST_MODE
#endif

#ifdef SPI_SOFTWARE                                 // 软件SPI
    #define SPI_MODE_IN     READ_Config
    #define SPI_MODE_OUT    WRITE_Config
#else                                               // 硬件SPI
    #define SPI_MODE_IN     READ_Config
    #define SPI_MODE_OUT    WRITE_Config
#endif

//SPI1
#define SPI1_NSS        -1     //(CS)
#define SPI1_SCK        14
#define SPI1_MISO       12
#define SPI1_MOSI       13
#define GPIO_SPI1       0


/*    驱动层      */

int Base_SPI_Init(SPI_mType Channel,uint8_t Width,int Set);

void Base_SPI_CS_Set(SPI_mType Channel,char Serial,char State);

void Base_SPI_ASK_Receive(SPI_mType Channel,uint16_t Data,uint16_t *Receive);
void Base_SPI_Send_Data(SPI_mType Channel,uint16_t Data);
void Base_SPI_DMA_Send_Data(SPI_mType Channel,const void *Data_array,int Length);



/*******************/
#if Exist_SPI & OPEN_0010
#define SPI1_NSS_L()    User_GPIO_set(GPIO_SPI1,SPI1_NSS,0)
#define SPI1_NSS_H()    User_GPIO_set(GPIO_SPI1,SPI1_NSS,1)
#define SPI1_SCK_L()    User_GPIO_set(GPIO_SPI1,SPI1_SCK,0)
#define SPI1_SCK_H()    User_GPIO_set(GPIO_SPI1,SPI1_SCK,1)
#define SPI1_MOSI_L()    User_GPIO_set(GPIO_SPI1,SPI1_MOSI,0)
#define SPI1_MOSI_H()    User_GPIO_set(GPIO_SPI1,SPI1_MOSI,1)

#define SPI1_MISO_R() User_GPIO_get(GPIO_SPI1,SPI1_MISO)  // 读取引脚电平
#endif

#if Exist_SPI & OPEN_0100

#endif

#endif
