#ifndef _SYSTEM_APP__H_
#define _SYSTEM_APP__H_

#include"Items.h"
#include"API.h"

#ifdef Exist_RTC_Clock
    #include "MODE_RTC8564.h"
#endif 
#ifdef Exist_GPIO
    #include"MODE_gpio.h"
#endif 
#ifdef Exist_UART 
    #include"MODE_uart.h" 
#endif 
#ifdef Exist_LCD
    #include "MODE_LCD.h"
#endif 

#include "Network_manage.h"
#include "tcp_server_link.h"
#include "tcp_client_link.h"
// #include "https.h"
#include "http.h"

/* 
-试一下
2023.11.8

*/

typedef enum {
    m_Protocol_CV = 0,
    m_Protocol_AT,
    m_Protocol_2,
} Protocol_mType;

typedef enum {
    m_Connect_SYS = 0,
    m_Connect_Server,
    m_Connect_Client,
    m_Connect_UDP,
    m_Connect_BLE,
    m_Connect_4G,
    m_Connect_RS232,
    m_Connect_RS485,
    m_Connect_USB,
} Connect_mType;

/*  [产品列表]    */
#define ESP32_CAVEND        314


#define BOARD_NAME      ESP32_CAVEND 
#define PROTOCOL_FRAME  m_Protocol_CV 
/*-----------------------------------*/

/*  [产品对应名称版本]    */
#if (BOARD_NAME == ESP32_CAVEND)
    #define SOFT_VERSIONS       "EC.0.0.01"

#endif // soft_versions
/*-----------------------------------*/


/*  [SYS_config]     */
typedef struct
{
    int ESP32_board_ID;     // 0(default)
    int RS232_Baud;
    int RS485_Baud;
    int SYS_Baud;
    int AT4G_Baud;
    int RS485_Addr;         //

    char RFID_Mode;          // 0透传\外挂模式
    char RS232_Mode;         // 0透传\外挂模式
    char RS485_Mode;         // 0透传\外挂模式

    char RJ45_enable;
    char WIFI_enable;
    char AT4G_enable;
    char Server_Switch;
    char Client_Switch;
    char HTTP_Switch;
    char HTTPS_Switch;
    char MQTT_Switch;

    char RJ45_work_Mode;
    char WIFI_work_Mode;    // dhcp = 0
    char RJ45_static_ip[30];
    char RJ45_static_gw[30];
    char RJ45_static_netmask[30];
    char WIFI_static_ip[30];
    char WIFI_static_gw[30];
    char WIFI_static_netmask[30];
    char MAC_addr[20];

    char Net_Server_port[10];
    char Net_Client_ip[30];
    char Net_Client_port[10];
    char AT4G_Client_ip[30];
    char AT4G_Client_port[10];

    char HTTP_url[100];
    char HTTPS_url[100];
    char HTTPS_way[50];
    char HTTPS_Host[50];
    char MQTT_url[100];
    
    char Device_version[50];
    char SYS_version[50];
    char Device_version_len;
    char SYS_version_len;

    // 设备动态管理
    char SYS_Rst;
    Caven_BaseTIME_Type time;

    char RJ45_online;
    char WIFI_online;
    char AT4G_online;
    char SYS_online;

    int Connect_passage;    // 当前连接
    int SYS_Run_Mode;
    int SYS_Run_Status;
    int Heartbeat_Run;
    int Heartbeat_MAX;
    
}SYS_cfg_Type;
extern SYS_cfg_Type g_SYS_Config;

/*-----------------------------------*/


void system_app_init(void);
void system_rst(void);


void test_led_task (void *pvParam);

#endif
