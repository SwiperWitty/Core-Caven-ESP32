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
    #if (BOARD_NAME == ESP32_CAVEND)
        
        #define CUSTOM_UART1
        #define ECHO_TXD1 (GPIO_NUM_15)
        #define ECHO_RXD1 (GPIO_NUM_34)

    #elif (Board_Name == EY1001)
        #include"Items.h"
        #define CUSTOM_UART1
        #define ECHO_TXD1 (GPIO_NUM_15)
        #define ECHO_RXD1 (GPIO_NUM_35)

        #define CUSTOM_UART2
        #define ECHO_TXD2 (GPIO_NUM_4)
        #define ECHO_RXD2 (GPIO_NUM_35)
    #endif
    #define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
    #define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)
#endif

void custom_uart_task_Fun(void);

#endif
