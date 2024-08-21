#ifndef _SYS_TYPEDEF__H_
#define _SYS_TYPEDEF__H_

#include "Caven_Type.h"

/*  log     *//* 
-试一下
2023.11.8

*//*-----------------------------------*/

typedef enum {
    m_Protocol_CV = 0,
    m_Protocol_1,
    m_Protocol_2,

} Protocol_mType;

/*  [产品列表]    */
#define ESP32_CAVEND        314


#define BOARD_NAME      ESP32_CAVEND 
#define PROTOCOL_FRAME  m_Protocol_CV 
/*-----------------------------------*/

/*  [产品对应名称版本]    */
#if (BOARD_NAME == ESP32_CAVEND)
    #define SOFT_VERSIONS       "EC.0.0.01"
#elif (BOARD_NAME == EY1001)
    #define SOFT_VERSIONS       "EY.0.0.01"
#endif // soft_versions
/*-----------------------------------*/

/*  [指令列表]    */


/*-----------------------------------*/

/*  [priority]    */

#define OTA_TASK_PRIORITY   1
#define GPIO_TASK_PRIORITY  2 
#define SHOW_TASK_PRIORITY  3 

#define UART1_TASK_PRIORITY 5 
#define UART2_TASK_PRIORITY 6 
#define TCP_SERVER_TASK_PRIORITY  7 
#define TCP_CLIENT_TASK_PRIORITY  8 
#define HTTPS_TASK_PRIORITY  9 
#define HTTP_TASK_PRIORITY  10 
#define MQTT_TASK_PRIORITY  11 
#define UDP_TASK_PRIORITY  12 

/*-----------------------------------*/

/*  [timerID]     */
#define TEST_TIMERID        (void *)0 
#define CHECK_TIMERID       (void *)1 
/*-----------------------------------*/

#endif
