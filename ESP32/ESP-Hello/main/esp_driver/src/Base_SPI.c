#include "Base_SPI.h"

#ifdef Exist_SPI



char SPI1_Width = 0;
char SPI2_Width = 0;

static void SPI_Delay (int time)
{
    volatile int temp;
    for (int i = 0; i < time; ++i)
    {
        temp = 10;            //SET
        do{
            NOP();
        }while((temp--) > 0);
    }
}


static void SPI1_GPIO_Init(int Set)
{
#if Exist_SPI & OPEN_0010
    if (Set)
    {
        User_GPIO_config(GPIO_SPI1,SPI1_MISO,SPI_MODE_IN);
        User_GPIO_config(GPIO_SPI1,SPI1_SCK,SPI_MODE_OUT);
        User_GPIO_config(GPIO_SPI1,SPI1_MOSI,SPI_MODE_OUT);
        User_GPIO_config(GPIO_SPI1,SPI1_NSS,SPI_MODE_OUT);
    }
    else
    {
        User_GPIO_config(GPIO_SPI1,SPI1_MISO,SPI_MODE_IN);
        User_GPIO_config(GPIO_SPI1,SPI1_SCK,SPI_MODE_IN);
        User_GPIO_config(GPIO_SPI1,SPI1_MOSI,SPI_MODE_IN);
        User_GPIO_config(GPIO_SPI1,SPI1_NSS,SPI_MODE_IN);
    }
#endif
}

static void SPI2_GPIO_Init(int Set)
{
#if Exist_SPI & OPEN_0100

#endif
}

/*
    这个是给硬件SPI的初始化 
*/
void Base_SPI1_Init (uint8_t Width,int Set)
{

}

void Base_SPI2_Init (uint8_t Width,int Set)
{

}
#endif

int Base_SPI_Init(SPI_mType Channel,uint8_t Width,int Set)
{
    int retval = -1;
#ifdef Exist_SPI

    SPI_Delay (1);
    switch (Channel)
    {
    case 0:
		
        break;
    case 1:
        SPI1_GPIO_Init(Set);
        SPI1_Width = Width;
        retval = 0;
        break;
    case 2:
        SPI2_GPIO_Init(Set);
        SPI2_Width = Width;
        retval = 0;
        break;
    case 3:

        break;
    default:
        break;
    }
#endif
    return retval;
}

void Base_SPI_CS_Set(SPI_mType Channel,char Serial,char State)
{
#ifdef Exist_SPI
    
    switch (Channel)
    {
    case 0:

        break;
    case 1:
    #if Exist_SPI & OPEN_0010
        if(Serial == 1)
        {
            if(State) {SPI1_NSS_L();}
            else {SPI1_NSS_H();}
        }
    #endif
        break;
    case 2:
    #if Exist_SPI & OPEN_0100
        if(Serial == 1)
        {
            if(State) {SPI2_NSS_L();}
            else {SPI2_NSS_H();}
        }
    #endif
        break;
    case 3:

        break;
    default:
        break;
    }
#endif
    
}

void Base_SPI_Send_Data(SPI_mType Channel,uint16_t Data)
{
#ifdef Exist_SPI
    /*
     * 写标志位
     * 写数据
     * 等忙位
     */
    #ifdef SPI_SOFTWARE
    int temp;
    switch (Channel)
    {
    case 0:

        break;
    case 1:
        #if Exist_SPI & OPEN_0010
        for (int i = 0; i < SPI1_Width; i++)
        {
            SPI1_SCK_L();           //预备Down沿
            temp = (Data << i) & (0x01 << (SPI1_Width - 1));
            if (temp)
                SPI1_MOSI_H();      //数据1
            else
                SPI1_MOSI_L();      //数据0
            SPI1_SCK_H();           //完成Down沿
        }
        SPI1_MOSI_H();
        #endif
        break;
    case 2:
        #if Exist_SPI & OPEN_0100
        for (int i = 0; i < SPI2_Width; i++)
        {
            SPI2_SCK_L();           //预备Down沿
            temp = (Data << i) & ((u16)0x01 << (SPI2_Width - 1));
            if (temp)
                SPI2_MOSI_H();      //数据1
            else
                SPI2_MOSI_L();      //数据0
            SPI2_SCK_H();           //完成Down沿
        }
        SPI2_MOSI_H();
        #endif
        break;
    case 3:

        break;
    default:
        break;
    }
    #else

    #endif

#endif 
}


void Base_SPI_DMA_Send_Data(SPI_mType Channel,const void *Data_array,int size)
{
#ifdef Exist_SPI
    
#endif 
}

void Base_SPI_ASK_Receive(SPI_mType Channel,uint16_t Data,uint16_t *Receive)
{
#ifdef Exist_SPI
    
#endif 
}

