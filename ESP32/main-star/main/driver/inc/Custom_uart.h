#ifndef _CUSTOM_UART__H_
#define _CUSTOM_UART__H_
/*
    this is Exist_UART.h feil    
    

    2023.9.13   
*/
#include "Items.h"
#include "sys_typedef.h"
#include "Caven_Type.h"

#include "info_frame_handle.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#ifdef Exist_UART
    #if CONFIG_IDF_TARGET_ESP32
        
        #define CUSTOM_UART1
        #define ECHO_TXD1 (GPIO_NUM_15)
        #define ECHO_RXD1 (GPIO_NUM_34)
        #define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
        #define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

        #define ECHO_TXD2 (GPIO_NUM_15)
        #define ECHO_RXD2 (GPIO_NUM_34)
    #endif

#endif

typedef void (*D_Callback_pFun) (void *data);   // 数据回调类型

int custom_uart1_init(int baud_rate, int set);
int custom_uart2_init(int baud_rate, int set);
int custom_uart1_send_data(uint8_t *data, int size);
int custom_uart2_send_data(uint8_t *data, int size);
void custom_uart1_receive_State_Machine_Bind (D_Callback_pFun Callback_pFun);
void custom_uart2_receive_State_Machine_Bind (D_Callback_pFun Callback_pFun);

void custom_uart_task_Fun(void);

#endif
