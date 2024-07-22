/* Ethernet Basic Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "reader_typedef.h"

#include "frame_analyse.h"
#include "system_parameter.h"
#include "system_gpio.h"
#include "extern_include.h"
#if CONFIG_ETH_USE_SPI_ETHERNET
#include "driver/spi_master.h"
#endif // CONFIG_ETH_USE_SPI_ETHERNET

static const char *TAG = "rj45_network";
#define CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET 1
#define CONFIG_EXAMPLE_ETH_PHY_RTL8201 1

#define CONFIG_EXAMPLE_ETH_PHY_ADDR 1
#define rtl8201_50MHz_Control_Pin GPIO_NUM_5 //!!!!
#define CONFIG_EXAMPLE_ETH_MDC_GPIO GPIO_NUM_23
#define CONFIG_EXAMPLE_ETH_MDIO_GPIO GPIO_NUM_18

extern uint8_t port_8160_using;
esp_netif_t *eth_netif;

#define rtl8201_50mhz_pin_output_pin_sel (1ULL << rtl8201_50MHz_Control_Pin)

static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{

    uint8_t mac_addr[6] = {0};
    char *p = NULL;
    EventBits_t r_event; /* 定义一个事件接收变量 */
    // 这里要赋值mac_address,
    uint8_t counter = 0;
    get_mac_address_resepond_info(mac_addr, &counter);
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id)
    {
    case ETHERNET_EVENT_CONNECTED:
        // esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        //  esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        port_8160_using = 0;
        system_para.rj45_connect_state = 1;

        // 网口连接   挂起wifi  client任务
        if ((wifi_TCPClientTxTaskHandle != NULL) || (wifi_TCPClientListeningRxTaskHandle != NULL))
        {
            vTaskSuspend(wifi_TCPClientTxTaskHandle);
            vTaskSuspend(wifi_TCPClientListeningRxTaskHandle);
        }
        tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, "gxwl_rfid"); // 这个语句实际测试在DHCP动态分配IP时候才生效，静态IP时候不生效
        // 启用DHCP功能,就注释掉下面这段话
        if (strcmp(system_para.rj45_get_ip_model, "static") == 0)
        { // 未启用DHCP才使用以下语句，设置相关静态IP
            tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);
            tcpip_adapter_ip_info_t eth;
            eth.ip.addr = ipaddr_addr(system_para.local_device_ip);
            ESP_LOGI(TAG, "Ethernet set subnet mask ip : %s", system_para.rj45_subnet_mask_ip);
            eth.netmask.addr = ipaddr_addr(system_para.rj45_subnet_mask_ip);
            eth.gw.addr = ipaddr_addr(system_para.rj45_gateway_ip);
            tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &eth);
            ESP_LOGI(TAG, "Ethernet got ip : static");

            // 设置DNS
            {
                uint8_t ip_temp[4];
                tcpip_adapter_dns_info_t dns_info = {0};
                ip4_addr_t ipaddr;
                memset(ip_temp, 0, 4);
                // ipstr_to_ip_address(system_para.rj45_dns_ip1, ip_temp);
                // IP4_ADDR(&dns_info.ip.u_addr.ip4, ip_temp[0], ip_temp[1], ip_temp[2], ip_temp[3]);
                // IP_ADDR4(ipaddr, ip_temp[0], ip_temp[1], ip_temp[2], ip_temp[3]);
                dns_info.ip.type = IPADDR_TYPE_V4;
                dns_info.ip.u_addr.ip4.addr = ipaddr_addr(system_para.rj45_dns_ip1);
                // dns_info.ip.u_addr.ip4.addr=1920103026;
                tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_ETH, ESP_NETIF_DNS_FALLBACK, &dns_info);
                // tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, 0, &dns_info);
                ESP_LOGW(TAG, "设置DNS信息");
            }
        }
        else
            ESP_LOGI(TAG, "Ethernet got ip : dhcp");

        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW MAC Addr %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

        // memset(str,0,sizeof(str));
        // tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_ETH,(const char *)(str));
        // ESP_LOGW(TAG, "tcpip_adapter_get_hostname:%s",str);
        // 运行到了这里,说明网线已经插入,并且从路由器上分配到了IP地址， 按照读写器规则，这里则需要将WiFi的连接断开
        // 网络连接上了,那就需要启动有线网络的TCP_Server进程了
        // xEventGroupSetBits(system_gxwl_myself_events,myself_events_need_kill_wifi_tcp_server_task);       //需要结束wifi任务,启动rj45 tcp server，重新监听
        // 延时200ms,等待结束任务
        //     vTaskDelay(1000*100);
        xEventGroupWaitBits(system_gxwl_myself_events, myself_events_rj45_client_connect_event, pdTRUE, pdTRUE, 1);

        if (NULL != (p = strstr(system_para.network_server_client_module, "tcpserver")))
        {

            ESP_LOGW("system program", "------>tcp server    rj45 network ");
        }
        else
        {
            ESP_LOGW("system program", "------>tcp client    rj45 network ");
            rj45_tcp_client_task_init();
        }

        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");

        if ((wifi_TCPClientTxTaskHandle != NULL) || (wifi_TCPClientListeningRxTaskHandle != NULL))
        {
            vTaskResume(wifi_TCPClientTxTaskHandle);
            vTaskResume(wifi_TCPClientListeningRxTaskHandle);
        }

        system_para.rj45_connect_state = 0;
        {
            // 网络断开了,有可能需要重新设置http_post_get 任务，要么WiFi建立了连接,不需要任何变化,要么需要退出重新连接的变量
            r_event = xEventGroupWaitBits(system_gxwl_myself_events, myself_events_wifi_connect_ap, pdFALSE, pdFALSE, 0);
            xEventGroupWaitBits(system_gxwl_myself_events, myself_events_rj45_client_connect_event, pdTRUE, pdTRUE, 1);
            if (r_event & myself_events_wifi_connect_ap)
            {
                ESP_LOGI(TAG, "wifi worknet is connected ");
            }
            else
            { // wifi没有连接,同时rj45网络也断开了,

                ESP_LOGI(TAG, "wifi worknet is disconnected");
            }
        }
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        xEventGroupSetBits(system_gxwl_myself_events, myself_events_rj45_init_finish);
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{

    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");

    system_para.rj45_got_ipv4_address_byte[0] = esp_ip4_addr1_16(&ip_info->ip);
    system_para.rj45_got_ipv4_address_byte[1] = esp_ip4_addr2_16(&ip_info->ip);
    system_para.rj45_got_ipv4_address_byte[2] = esp_ip4_addr3_16(&ip_info->ip);
    system_para.rj45_got_ipv4_address_byte[3] = esp_ip4_addr4_16(&ip_info->ip);

    system_para.rj45_dhcp_info.ip_address[0] = esp_ip4_addr1_16(&ip_info->ip);
    system_para.rj45_dhcp_info.ip_address[1] = esp_ip4_addr2_16(&ip_info->ip);
    system_para.rj45_dhcp_info.ip_address[2] = esp_ip4_addr3_16(&ip_info->ip);
    system_para.rj45_dhcp_info.ip_address[3] = esp_ip4_addr4_16(&ip_info->ip);

    system_para.rj45_dhcp_info.subnet_ip[0] = esp_ip4_addr1_16(&ip_info->netmask);
    system_para.rj45_dhcp_info.subnet_ip[1] = esp_ip4_addr2_16(&ip_info->netmask);
    system_para.rj45_dhcp_info.subnet_ip[2] = esp_ip4_addr3_16(&ip_info->netmask);
    system_para.rj45_dhcp_info.subnet_ip[3] = esp_ip4_addr4_16(&ip_info->netmask);

    system_para.rj45_dhcp_info.gateway_ip[0] = esp_ip4_addr1_16(&ip_info->gw);
    system_para.rj45_dhcp_info.gateway_ip[1] = esp_ip4_addr2_16(&ip_info->gw);
    system_para.rj45_dhcp_info.gateway_ip[2] = esp_ip4_addr3_16(&ip_info->gw);
    system_para.rj45_dhcp_info.gateway_ip[3] = esp_ip4_addr4_16(&ip_info->gw);

    memset(system_para.rj45_dhcp_info.ip_str, 0, sizeof(system_para.rj45_dhcp_info.ip_str));
    memset(system_para.rj45_dhcp_info.gateway_ip_str, 0, sizeof(system_para.rj45_dhcp_info.gateway_ip_str));
    memset(system_para.rj45_dhcp_info.subnet_ip_str, 0, sizeof(system_para.rj45_dhcp_info.subnet_ip_str));
    snprintf(system_para.rj45_dhcp_info.ip_str, sizeof(system_para.rj45_dhcp_info.ip_str), "%d.%d.%d.%d", system_para.rj45_dhcp_info.ip_address[0], system_para.rj45_dhcp_info.ip_address[1], system_para.rj45_dhcp_info.ip_address[2], system_para.rj45_dhcp_info.ip_address[3]);
    snprintf(system_para.rj45_dhcp_info.gateway_ip_str, sizeof(system_para.rj45_dhcp_info.gateway_ip_str), "%d.%d.%d.%d", system_para.rj45_dhcp_info.gateway_ip[0], system_para.rj45_dhcp_info.gateway_ip[1], system_para.rj45_dhcp_info.gateway_ip[2], system_para.rj45_dhcp_info.gateway_ip[3]);
    snprintf(system_para.rj45_dhcp_info.subnet_ip_str, sizeof(system_para.rj45_dhcp_info.subnet_ip_str), "%d.%d.%d.%d", system_para.rj45_dhcp_info.subnet_ip[0], system_para.rj45_dhcp_info.subnet_ip[1], system_para.rj45_dhcp_info.subnet_ip[2], system_para.rj45_dhcp_info.subnet_ip[3]);
    // 动态获取IP了,需要更新一下默认的wifiIP地址
    // memset(system_para.wifi_ip_address, 0, sizeof(system_para.wifi_ip_address));
    // strcpy(system_para.wifi_ip_address, system_para.wifi_dhcp_info.ip_str);
    // memset(system_para.wifi_gateway_ip, 0, sizeof(system_para.wifi_gateway_ip));
    // strcpy(system_para.wifi_gateway_ip, system_para.wifi_dhcp_info.gateway_ip_str);
    // memset(system_para.wifi_subnet_mask_ip, 0, sizeof(system_para.wifi_subnet_mask_ip));
    // strcpy(system_para.wifi_subnet_mask_ip, system_para.wifi_dhcp_info.subnet_ip_str);

    system_para.rj45_dhcp_info.using_flag = true;
    {
        extern uint8_t connect_wifi_finish_flag;
        connect_wifi_finish_flag = 0; // 有线网络连接上了,不管WiFi是否连接上,先将其置零
        // 有线网络已经连接了,这里启动UDP组播任务
        udp_multicast_send_device_info_task();
    }
}

void rj45_network_rtl8201_init(void)
{
    // Initialize TCP/IP network interface (should be called only once in application)
    // ESP_ERROR_CHECK(esp_netif_init());
    // // Create default event loop that running in background
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    uint8_t counter = 0;
    uint8_t mac_addr[6] = {0x10, 0x97, 0xbd, 0x34, 0x01, 0x50};
    get_mac_address_resepond_info(mac_addr, &counter);
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&cfg);
    // Set default handlers to process TCP/IP stuffs
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
    // esp_netif_set_hostname(eth_netif, "esp32_");
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG(); // ETH_MAC_DEFAULT_CONFIG 这个函数里面定义了MDC和MDIO引脚，这些RMII接口引脚是固定引脚，所以写在底层函数中，不能改变
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG(); // 这个ETH_PHY_DEFAULT_CONFIG函数中定义了LAN8720复位引脚和phy_addr 获取方式
    phy_config.phy_addr = 0;                                // 1
    phy_config.reset_gpio_num = rtl8201_50MHz_Control_Pin;
    // phy_config.reset_gpio_num = -1;

#if CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
    mac_config.smi_mdc_gpio_num = 23;
    mac_config.smi_mdio_gpio_num = 18;
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
#elif CONFIG_ETH_USE_SPI_ETHERNET
    gpio_install_isr_service(0);
    spi_device_handle_t spi_handle = NULL;
    spi_bus_config_t buscfg = {
        .miso_io_num = CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO,
        .mosi_io_num = CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO,
        .sclk_io_num = CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_EXAMPLE_ETH_SPI_HOST, &buscfg, 1));
#if CONFIG_EXAMPLE_USE_DM9051
    spi_device_interface_config_t devcfg = {
        .command_bits = 1,
        .address_bits = 7,
        .mode = 0,
        .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .spics_io_num = CONFIG_EXAMPLE_ETH_SPI_CS_GPIO,
        .queue_size = 20};
    ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_EXAMPLE_ETH_SPI_HOST, &devcfg, &spi_handle));
    /* dm9051 ethernet driver is based on spi driver */
    eth_dm9051_config_t dm9051_config = ETH_DM9051_DEFAULT_CONFIG(spi_handle);
    dm9051_config.int_gpio_num = CONFIG_EXAMPLE_ETH_SPI_INT_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_dm9051(&dm9051_config, &mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_dm9051(&phy_config);
#elif CONFIG_EXAMPLE_USE_W5500
    spi_device_interface_config_t devcfg = {
        .command_bits = 16, // Actually it's the address phase in W5500 SPI frame
        .address_bits = 8,  // Actually it's the control phase in W5500 SPI frame
        .mode = 0,
        .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .spics_io_num = CONFIG_EXAMPLE_ETH_SPI_CS_GPIO,
        .queue_size = 20};
    ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_EXAMPLE_ETH_SPI_HOST, &devcfg, &spi_handle));
    /* w5500 ethernet driver is based on spi driver */
    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(spi_handle);
    w5500_config.int_gpio_num = CONFIG_EXAMPLE_ETH_SPI_INT_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);
#endif
#endif // CONFIG_ETH_USE_SPI_ETHERNET
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
#if !CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
    /* The SPI Ethernet module might doesn't have a burned factory MAC address, we cat to set it manually.
       02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
    */
    ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, (uint8_t[]){0x02, 0x00, 0x00, 0x12, 0x34, 0x56}));
#endif
    esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, mac_addr);

    ESP_LOGI(TAG, "IP mac add : %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    ESP_LOGW("DEBUG", "RJ45 [1] rtl8201 START ");
    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    ESP_LOGW("DEBUG", "RJ45 [2] rtl8201 START ");
    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    ESP_LOGW("DEBUG", "RJ45 [3] rtl8201 START ");
}
