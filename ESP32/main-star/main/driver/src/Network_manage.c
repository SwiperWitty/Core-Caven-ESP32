#include "Network_manage.h"

#include <stdio.h>
#include <string.h>
#include "stdlib.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_eth.h"

#include "nvs_flash.h"
#include "driver/gpio.h"
#include "freertos/event_groups.h"

#if CONFIG_ETH_USE_SPI_ETHERNET
#include "driver/spi_master.h"
#endif

uint16_t tcp_server_port_connect_counter_when_using; //!!!! 
uint16_t tcp_server_port;                            //!!!! 
uint8_t lwip_task_idle_timer_counter;                //!!!! 
uint8_t port_8160_using;
static const char *TAG = "Network_manage";

static char NET_ip[30] = {0};
static int NET_port = 0;
static int esp_netif_init_flag = 0;

#define CONFIG_ESP_WIFI_AUTH_OPEN 1
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN

#define WIFI_SSID_SCAN_MAX_VALUE 20
static wifi_ap_record_t wifi_ap_info[WIFI_SSID_SCAN_MAX_VALUE];
static uint16_t wifi_ap_num = WIFI_SSID_SCAN_MAX_VALUE;
static char WIFI_name[30] = {0};
static char WIFI_pass[30] = {0};

static void print_auth_mode(int authmode)
{
    switch (authmode) {
    case WIFI_AUTH_OPEN:
        ESP_LOGI("wifi scan", "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI("wifi scan", "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI("wifi scan", "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI("wifi scan", "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI("wifi scan", "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        ESP_LOGI("wifi scan", "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI("wifi scan", "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI("wifi scan", "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    default:
        ESP_LOGI("wifi scan", "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
    switch (pairwise_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI("wifi scan", "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI("wifi scan", "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI("wifi scan", "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI("wifi scan", "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI("wifi scan", "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI("wifi scan", "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    default:
        ESP_LOGI("wifi scan", "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }

    switch (group_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI("wifi scan", "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI("wifi scan", "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI("wifi scan", "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI("wifi scan", "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI("wifi scan", "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI("wifi scan", "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    default:
        ESP_LOGI("wifi scan", "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:      // WIFI以STA模式启动后触发此事件
            esp_wifi_connect();         // 启动WIFI连接
            break;
        case WIFI_EVENT_STA_CONNECTED:  // WIFI连上路由器后，触发此事件
            ESP_LOGI(TAG, "connected to AP succ !!!");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:   // WIFI从路由器断开连接后触发此事件
            esp_wifi_connect();             // 继续重连
            ESP_LOGI(TAG,"connect to the AP fail,retry now[%s]",WIFI_name);
            break;
        default:
            break;
        }
    }
    if(event_base == IP_EVENT)                  // IP相关事件
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        switch(event_id)
        {
            case IP_EVENT_STA_GOT_IP:           // 只有获取到路由器分配的IP，才认为是连上了路由器
                ESP_LOGW(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
                break;
            default:
                break;
        }
    }
}

static int ipstr_to_numeric(const char *_str, uint32_t *_addr)
{
    const char *index;
    unsigned char *addr = (unsigned char *)_addr;
    int isnumeric = 1;
    int i = 3;
    *_addr = 0;
    index = _str;
    while ((*index) && (isnumeric))
    {
        if (isdigit((unsigned char)*index))
        {
            addr[i] = addr[i] * 10 + (*index - '0'); // big-endian
        }
        else if (*index == '.')
        {
            i--;
            if (i == -1)
                isnumeric = 0;
        }
        else
        {
            isnumeric = 0;
        }
        index++;
    }
    if (isnumeric && i)
        return -1; // error
    if (isnumeric)
        return 0; // successful
    else
        return -1;
}

static void get_ip_address_info(uint32_t ipadd, uint8_t *ip_buffer)
{
    ip_buffer[0] = ipadd >> 24;
    ipadd = ipadd << 8;
    ip_buffer[1] = ipadd >> 24;
    ipadd = ipadd << 8;
    ip_buffer[2] = ipadd >> 24;
    ipadd = ipadd << 8;
    ip_buffer[3] = ipadd >> 24;
}

void ipstr_to_ip_address(char *str, uint8_t *ip)
{
    uint32_t ip_add = 0;
    if (0 == ipstr_to_numeric(str, &ip_add))
    {
        get_ip_address_info(ip_add, ip);
    }
}
/*
    mode = 0,dhcp
    mode = 0,static
    ip_str  ip地址
    gw_str  默认网关
    netmask_str 掩码
*/
void wifi_config_ip (char mode,char *ip_str,char *gw_str,char *netmask_str)
{
    if (mode == 0 || ip_str == NULL)
    {

    }
    else
    {
        tcpip_adapter_ip_info_t wifi_ip_info;
        uint8_t ip_temp[4];
        ESP_LOGW(TAG,"set wifi Custom ip ->");
        // 第一步先关闭DHCP功能,默认是打开的
        tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
        // 第二步设置IP地址相关参数
        memset(ip_temp,0,4);
        ipstr_to_ip_address(ip_str,ip_temp);
        // IP4_ADDR(&wifi_ip_info.ip,192,168,137,200);
        IP4_ADDR(&wifi_ip_info.ip,ip_temp[0],ip_temp[1],ip_temp[2],ip_temp[3]);
        ESP_LOGI(TAG,"set device wifi ip address:%d.%d.%d.%d",ip_temp[0],ip_temp[1],ip_temp[2],ip_temp[3]);

        memset(ip_temp,0,4);
        ipstr_to_ip_address(gw_str,ip_temp);
        // IP4_ADDR(&wifi_ip_info.gw,192,168,0,1);
        IP4_ADDR(&wifi_ip_info.gw,ip_temp[0],ip_temp[1],ip_temp[2],ip_temp[3]);
        ESP_LOGI(TAG,"set device wifi gateway address:%d.%d.%d.%d",ip_temp[0],ip_temp[1],ip_temp[2],ip_temp[3]);

        memset(ip_temp,0,4);
        ipstr_to_ip_address(netmask_str,ip_temp);
        // IP4_ADDR(&wifi_ip_info.netmask,255,255,255,0);
        IP4_ADDR(&wifi_ip_info.netmask,ip_temp[0],ip_temp[1],ip_temp[2],ip_temp[3]);
        ESP_LOGI(TAG,"set device wifi netmask address:%d.%d.%d.%d",ip_temp[0],ip_temp[1],ip_temp[2],ip_temp[3]);

        tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA,&wifi_ip_info);
        //第三步打开dhcp
        tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_STA);
    }
}

static void wifi_init(void)
{
#if 1
    uint16_t ap_count = 0;
    
    if (strlen(WIFI_name) == 0)
    {
        memcpy(WIFI_name,DEFAULT_WIFI_NAME,strlen(DEFAULT_WIFI_NAME));
        memcpy(WIFI_pass,DEFAULT_WIFI_PASS,strlen(DEFAULT_WIFI_PASS));
    }
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    if (esp_netif_init_flag == 0)
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());   // 创建一个默认系统事件调度循环，之后可以注册回调函数来处理系统的一些事件
        esp_netif_init_flag = 1;
    }
    esp_netif_create_default_wifi_sta();    // 使用默认配置创建STA对象

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // wifi logo
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    esp_wifi_scan_stop();
    memset(wifi_ap_info, 0, sizeof(wifi_ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&wifi_ap_num, wifi_ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    ESP_LOGI(TAG, "Total APs ap_count[%d],wifi_ap_num[%d]", ap_count,wifi_ap_num);
    for (int i = 0; (i < wifi_ap_num) && (i < ap_count); i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", wifi_ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", wifi_ap_info[i].rssi);
        if (wifi_ap_info[i].rssi > (-60))
        {
            print_auth_mode(wifi_ap_info[i].authmode);
            if (wifi_ap_info[i].authmode != WIFI_AUTH_WEP) {
                print_cipher_type(wifi_ap_info[i].pairwise_cipher, wifi_ap_info[i].group_cipher);
            }
        }
        ESP_LOGI(TAG, "Channel \t\t%d\n", wifi_ap_info[i].primary);
    }
    esp_wifi_stop();

    // 注册事件
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event_handler,NULL,&instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&wifi_event_handler,NULL,&instance_got_ip));
    // 静态IP
    wifi_config_ip (1,"192.168.11.166","192.168.11.1","255.255.255.0");
    // 修改成自定义的SSID和密码
    wifi_config_t wifi_config = {
        .sta = {
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
	     .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    memcpy(wifi_config.sta.ssid,WIFI_name,strlen(WIFI_name));
    memcpy(wifi_config.sta.password,WIFI_pass,strlen(WIFI_pass));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "WIFI Start, target [%s][%s] --> \n", wifi_config.sta.ssid,wifi_config.sta.password);
    ESP_LOGW(TAG, "wifi init sta finished");
#endif
}

esp_netif_t *eth_netif;
uint8_t RJ45_ip[4];
uint8_t RJ45_netmask[4];
uint8_t RJ45_gateway[4];

static void eth_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    char *p = NULL;
    uint8_t counter = 0;
    // get_mac_address_resepond_info(mac_addr, &counter);
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id)
    {
    case ETHERNET_EVENT_CONNECTED:
        ESP_LOGW(TAG, "Ethernet Link up");
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    RJ45_ip[0] = esp_ip4_addr1_16(&ip_info->ip);
    RJ45_ip[1] = esp_ip4_addr2_16(&ip_info->ip);
    RJ45_ip[2] = esp_ip4_addr3_16(&ip_info->ip);
    RJ45_ip[3] = esp_ip4_addr4_16(&ip_info->ip);

    RJ45_netmask[0] = esp_ip4_addr1_16(&ip_info->netmask);
    RJ45_netmask[1] = esp_ip4_addr2_16(&ip_info->netmask);
    RJ45_netmask[2] = esp_ip4_addr3_16(&ip_info->netmask);
    RJ45_netmask[3] = esp_ip4_addr4_16(&ip_info->netmask);

    RJ45_gateway[0] = esp_ip4_addr1_16(&ip_info->gw);
    RJ45_gateway[1] = esp_ip4_addr2_16(&ip_info->gw);
    RJ45_gateway[2] = esp_ip4_addr3_16(&ip_info->gw);
    RJ45_gateway[3] = esp_ip4_addr4_16(&ip_info->gw);
    
    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, " \n");
}

static void rtl8201_init(void)
{
#if 1
    uint8_t counter = 0;
    uint8_t mac_addr[6] = {0x10, 0x97, 0xbd, 0x34, 0x01, 0x50};

    ESP_LOGI(TAG, "RJ45 Start --> \n");
    // get_mac_address_resepond_info(mac_addr, &counter);
    if (esp_netif_init_flag == 0)
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_init_flag = 1;
    }

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&cfg);
    // Set default handlers to process TCP/IP stuffs
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG(); // ETH_MAC_DEFAULT_CONFIG 这个函数里面定义了MDC和MDIO引脚，这些RMII接口引脚是固定引脚，所以写在底层函数中，不能改变
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG(); // 这个ETH_PHY_DEFAULT_CONFIG函数中定义了LAN8720复位引脚和phy_addr 获取方式
    phy_config.phy_addr = 1;

    #if CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
    mac_config.smi_mdc_gpio_num = RTL8201_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = RTL8201_ETH_MDIO_GPIO;
    phy_config.reset_gpio_num = RTL8201_ETH_RST_GPIO;
    // phy_config.reset_gpio_num = -1;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);

    #if CONFIG_EXAMPLE_ETH_PHY_IP101
    esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
    #elif CONFIG_EXAMPLE_ETH_PHY_RTL8201
    esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);
    #elif CONFIG_EXAMPLE_ETH_PHY_LAN8720
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
    #elif CONFIG_EXAMPLE_ETH_PHY_DP83848
    esp_eth_phy_t *phy = esp_eth_phy_new_dp83848(&phy_config);
    #elif CONFIG_EXAMPLE_ETH_PHY_KSZ8041
    esp_eth_phy_t *phy = esp_eth_phy_new_ksz8041(&phy_config);
    #endif

    #endif
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, mac_addr);

    ESP_LOGI(TAG, "IP mac add : %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    ESP_LOGW(TAG, "RJ45 init finished");
#endif
}

int Network_manage_Init (int mode)
{
    int retval = 0;
    switch (mode)
    {
    case 1:
        wifi_init();
        break;
    case 2:
        rtl8201_init();
        break;
    case 0xFF:
        rtl8201_init();
        wifi_init();
        break;
    default:
        esp_wifi_stop();

        break;
    }
    return retval;
}