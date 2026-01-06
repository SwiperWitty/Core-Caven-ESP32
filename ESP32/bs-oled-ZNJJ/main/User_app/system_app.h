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
#elif (BOARD_NAME == EY1001)
    #define SOFT_VERSIONS       "EY.0.0.01"
#endif // soft_versions
/*-----------------------------------*/

/*  [指令列表]    */
typedef enum
{
    m_query_reader_params_order = 0x00,
    m_query_reader_software_version_order = 0x01,
    m_config_reader_serial_params_order = 0x02,
    m_query_reader_serial_params_order = 0x03,
    m_config_reader_ip_address_params_order = 0x04,
    m_query_reader_ip_address_params_order = 0x05,
    m_query_reader_mac_address_params_order = 0x06,
    m_config_server_client_modle_params_order = 0x07,
    m_query_server_client_modle_params_order = 0x08,
    m_config_reader_GPO_state_order = 0x09,                 // !!!!
    m_query_reader_GPI_state_order = 0x0A,                  // !!!!
    m_config_reader_GPI_trigger_order = 0x0B,
    m_query_reader_GPI_trigger_order = 0x0C,
    m_config_reader_WG_params_order = 0x0D,                 // !!!!
    m_query_reader_WG_params_order = 0x0E,
    m_restart_reader_order = 0x0F,                          // !!!!
    m_config_reader_time_date_params_order = 0x10,
    m_query_reader_time_date_params_order = 0x11,
    m_reader_connect_state_confirm_order = 0x12,            // !!!!
    m_config_reader_mac_address_params_order = 0x13,
    m_reader_restore_default_params_order = 0x14,
    m_config_reader_rs485_address_order = 0x15,
    m_query_reader_rs485_addresss_order = 0x16,
    m_config_reader_breakpoint_continuation_order = 0x17,
    m_query_reader_breakpoint_continuation_order = 0x18,
    m_get_reader_6B_6C_tag_data_buffer_order = 0x1B,
    m_clear_reader_6B_6C_tag_data_buffer_order = 0x1C,
    m_tag_6B_6C_respeond_order = 0x1D,
    m_beep_device_power_on_order = 0x1E,
    m_beep_device_control_order = 0x1F,
    m_get_reader_white_list_order = 0x20,
    m_import_reader_white_list_order = 0x21,
    m_del_reader_white_list_order = 0x22,
    m_config_reader_white_list_params_order = 0x23,
    m_get_reader_white_list_params_order = 0x24,
    m_config_reader_white_list_switch_order = 0x25,
    m_query_reader_white_list_switch_order = 0x26,
    m_config_reader_udp_send_data_params_order = 0x27,
    m_query_reader_udp_send_data_params_order = 0x28,
    m_config_reader_http_send_data_params_order = 0x29,
    m_query_reader_http_send_data_params_order = 0x2a,
    m_usb_keyboard_enable_or_disable_order = 0x2B,
    m_config_HL_company_reader_rj45_ip_params_order = 0x2F,
    m_query_HL_Company_reader_rj45_ip_params_order = 0x30,
    m_config_reader_wifi_start_scan_order = 0x31,
    m_query_reader_wifi_scan_result_order = 0x32,
    m_config_reader_wifi_ap_params_order = 0x33,
    m_query_reader_wifi_connect_state_order = 0x34,
    m_config_reader_wifi_network_params_order = 0x35,
    m_query_reader_wifi_network_params_order = 0x36,
    m_config_reader_wifi_enable_or_disable_order = 0x37,
    m_query_reader_wifi_enable_or_disable_order = 0x38,
    m_query_reader_hostname_order = 0x3e,
    m_config_reader_hostname_order = 0x3d,
    m_config_reader_EAS_order = 0x3F,
    m_query_reader_EAS_order = 0x40,
    m_config_sf_express_forklift_reader_order = 0x43,
    m_query_sf_express_forklift_reader_order = 0x44,
    m_config_wild_horse_ding_wei_entrance_guard_order = 0x45,
    m_query_wild_horse_ding_wei_entrance_guard_order = 0x46,
    m_config_sike_plc_tcp_params_reader_order = 0x47,
    m_query_sike_plc_tcp_params_reader_order = 0x48,
    m_config_nxp_antenna_control_order = 0x49,
    m_config_wifi_roaming_control_order = 0x4A,
    m_config_system_device_watch_dog_params_order = 0x4B,
    m_config_system_device_gps_params_order = 0x4C,
    m_query_system_device_gps_params_order = 0x4d,
    m_config_system_device_4G_params_order = 0x4E,
    m_query_system_device_IO_params_order = 0x4F,
    m_query_system_device_4G_params_order = 0x50,
    m_query_system_device_internet_connect_state_order = 0x51,
    m_config_system_device_internet_connect_sequence_order = 0x52,
    m_config_China_Constructiong_Bank_Corp_entrance_guard_order = 0x53,
    m_cofing_system_device_http_update_bin_order = 0x54,
    m_system_plug_in_device_send_data_order = 0x63,       // 外挂设备
    m_stop_read_tag_order = 0xff
}reader_order_MID_mType;

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

int custom_gpio_init (int set);    // 示例 

void LCD_POW_Set (int set);
void POW_KILL_Set (int set);
int LED_Set (char n,int set);

void test_led_task (void *pvParam);

#endif
