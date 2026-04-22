#include "Base_UART.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#if CONFIG_IDF_TARGET_ESP32
    
    #define CUSTOM_UART1
    #define ECHO_TXD1 (15)
    #define ECHO_RXD1 (34)
    #define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
    #define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

    #define ECHO_TXD2 (4)
    #define ECHO_RXD2 (35)

    QueueHandle_t uart1_event_queue;
    QueueHandle_t uart2_event_queue;
#endif

static D_pFun State_Machine_UART_pFun[5];

uint8_t uart0_enable = 0;
uint8_t uart1_enable = 0;
uint8_t uart2_enable = 0;
uint8_t uart3_enable = 0;
uint8_t uart4_enable = 0;

int Uart1_Init(int Baud,int Set)
{
    int retval = -1;
    if (Set)
    {
        uart_config_t uart_config = {
            .baud_rate = Baud,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
        uart_param_config(UART_NUM_1, &uart_config);
        uart_set_pin(UART_NUM_1, ECHO_TXD1, ECHO_RXD1, ECHO_TEST_RTS, ECHO_TEST_CTS);
        // 串口1的接收缓冲区2048，发送缓冲区2048，定义uart1_event_queue 事件消息队列大小为10
        uart_driver_install(UART_NUM_1, 2048, 2048, 10, &uart1_event_queue, 0);
        uart1_enable = 1;
    }
    else
    {
        uart1_enable = 0;
    }
    return retval;
}

int Uart2_Init(int Baud,int Set)
{
    int retval = -1;
    if (Set)
    {
        uart_config_t uart_config = {
            .baud_rate = Baud,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
        uart_param_config(UART_NUM_2, &uart_config);
        uart_set_pin(UART_NUM_2, ECHO_TXD2, ECHO_RXD2, ECHO_TEST_RTS, ECHO_TEST_CTS);
        
        uart_driver_install(UART_NUM_2, 2048, 2048, 10, &uart2_event_queue, 0);
        uart2_enable = 1;
    }
    else
    {
        uart2_enable = 0;
    }
    return retval;
}

int Base_UART_Init(UART_mType Channel,int Baud,int Set)
{
    int retval = -1;
#ifdef Exist_UART
    switch (Channel)
    {
    case 0:
    #if (Exist_UART & OPEN_0001)
        Uart0_Init(Baud,Set);
        retval = 0;
    #endif
        break;
    case 1:
    #if (Exist_UART & OPEN_0010)
        Uart1_Init(Baud,Set);
        retval = 0;
    #endif
        break;
    case 2:
    #if (Exist_UART & OPEN_0100)
        Uart2_Init(Baud,Set);
        retval = 0;
    #endif
        break;
    case 3:
    #if (Exist_UART & OPEN_1000)
        Uart3_Init(Baud,Set);
        retval = 0;
    #endif
        break;
    case 4:
    #if (Exist_UART & OPEN_10000)
        Uart4_Init(Baud,Set);
        retval = 0;
    #endif
        break;
    default:
        break;
    }
#endif
    return retval;
}

void Base_UART_Send_Data(UART_mType Channel,uint16_t Data)
{
#ifdef Exist_UART
    uint8_t *data = (uint8_t *)&Data;
    int len = 1;
    switch (Channel)
    {
    case 0:
    #if (Exist_UART & OPEN_0001)
        if (uart0_enable)
        {
            uart_write_bytes(UART_NUM_0, data, len);
        }
    #endif
        break;
    case 1:
    #if (Exist_UART & OPEN_0010)
        if (uart1_enable)
        {
            uart_write_bytes(UART_NUM_1, data, len);
        }
    #endif
        break;
    case 2:
    #if (Exist_UART & OPEN_0100)
        if (uart2_enable)
        {
            uart_write_bytes(UART_NUM_2, data, len);
        }
    #endif
        break;
    case 3:
    #if (Exist_UART & OPEN_1000)

    #endif
        break;
    case 4:
    #if (Exist_UART & OPEN_10000)

    #endif
        break;
    default:
        break;
    }
#endif
}

void Base_UART_DMA_Send_Buff(UART_mType Channel,const uint8_t *Data,int Length)
{
#ifdef Exist_UART
    uint8_t *data = (uint8_t *)Data;
    int len = Length;
    switch (Channel)
    {
    case 0:
    #if (Exist_UART & OPEN_0001)
        if (uart0_enable)
        {
            uart_write_bytes(UART_NUM_0, data, len);
        }
    #endif
        break;
    case 1:
    #if (Exist_UART & OPEN_0010)
        if (uart1_enable)
        {
            uart_write_bytes(UART_NUM_1, data, len);
        }
    #endif
        break;
    case 2:
    #if (Exist_UART & OPEN_0100)
        if (uart2_enable)
        {
            uart_write_bytes(UART_NUM_2, data, len);
        }
    #endif
        break;
    case 3:
    #if (Exist_UART & OPEN_1000)

    #endif
        break;
    case 4:
    #if (Exist_UART & OPEN_10000)

    #endif
        break;
    default:
        break;
    }
#endif
}

/*
 *  Successful : return 0
 *
 */
int State_Machine_Bind(UART_mType Channel, D_pFun UART_pFun)
{
    int retval = -1;
#ifdef Exist_UART
    if (UART_pFun == NULL)
    {
        return retval;
    }
    State_Machine_UART_pFun[Channel] = UART_pFun;
    retval = 0;
#endif
    return retval;
}

// 你找中断？UART的中断通过函数回调给MODE了！
