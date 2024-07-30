#ifndef _NETWORK_MANAGE_H_
#define _NETWORK_MANAGE_H_

#include "stdint.h"

#define DEFAULT_WIFI_NAME "GX"    // GX Cavendish
#define DEFAULT_WIFI_PASS "GX201909"    // GX201909 1234567890

#define DEFAULT_RJ45_IP "192.168.1.168"
#define DEFAULT_RJ45_PORT 8160 

#define CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET 1
#define CONFIG_EXAMPLE_ETH_PHY_RTL8201 1

#define RTL8201_ETH_PHY_ADDR 1
#define RTL8201_ETH_RST_GPIO GPIO_NUM_5
#define RTL8201_ETH_MDC_GPIO GPIO_NUM_23
#define RTL8201_ETH_MDIO_GPIO GPIO_NUM_18

extern uint16_t tcp_server_port_connect_counter_when_using; //!!!! 
extern uint16_t tcp_server_port;                            //!!!! 
extern uint8_t lwip_task_idle_timer_counter;                //!!!! 
extern uint8_t port_8160_using;

int Network_manage_Init (int mode,int set);
void Network_manage_set_mac (uint8_t *mac);
void Network_manage_get_mac (uint8_t *mac);

int wifi_config_ip (char mode,char *ip_str,char *gw_str,char *netmask_str);
int eth_config_ip (char mode,char *ip_str,char *gw_str,char *netmask_str);

#endif
