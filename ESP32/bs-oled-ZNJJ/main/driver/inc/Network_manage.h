#ifndef _NETWORK_MANAGE_H_
#define _NETWORK_MANAGE_H_

#include "API.h"
#include "Items.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#ifndef CONFIG_EXAMPLE_IPV4
#define CONFIG_EXAMPLE_IPV4         1
#endif
#ifndef CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
#define CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET 1
#endif
#ifndef CONFIG_EXAMPLE_ETH_PHY_RTL8201
#define CONFIG_EXAMPLE_ETH_PHY_RTL8201 1
#endif

#define RTL8201_ETH_PHY_ADDR 1
#define RTL8201_ETH_RST_GPIO GPIO_NUM_5
#define RTL8201_ETH_MDC_GPIO GPIO_NUM_23
#define RTL8201_ETH_MDIO_GPIO GPIO_NUM_18
#endif

typedef struct{
    int use_dhcp;
    char ip[16];
    char netmask[16];
    char gateway[16];
    char dns1[16];
    char dns2[16];
}net_cfg_Type;

typedef void (*D_Callback_pFun) (void *data);   // 数据回调类型

int Network_manage_Init (int mode,int set);
void Network_manage_set_mac (uint8_t *mac);
void Network_manage_get_mac (uint8_t *mac);

int wifi_config_user (char *ssid,char *pass);
int wifi_config_ip (char mode,char *ip_str,char *gw_str,char *netmask_str);
int eth_config_ip (char mode,char *ip_str,char *gw_str,char *netmask_str);

int wifi_get_local_ip_status (char *ip_str,char *gw_str,char *netmask_str);
int eth_get_local_ip_status (char *ip_str,char *gw_str,char *netmask_str);

void Network_url_resolve_client(const char *url,char *ret_data);
int Network_manage_IPprot (char *url,char *ip,char *port);
int Network_manage_get_status (void);

#endif
