/*
    前提是eth和wifi任意一个启动连接才可以使用
*/

#include "tcp_server_link.h"

#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


#define KEEPALIVE_IDLE              20
#define KEEPALIVE_INTERVAL          5
#define KEEPALIVE_COUNT             3

static const char *TAG = "TCP server";

static int tcp_server_sock = 0;
static char sock_port_str[10] = {0};

/*
    server 只能修改端口，如果需要修改ip请修改[eth_config_ip]/[wifi_config_ip]
*/
int tcp_server_link_ip_config (char *port_str,int enable)
{
    int retval = 0;
    if (enable)
    {
        if (port_str == NULL )
        {
            retval = 1;
            ESP_LOGE(TAG, "where are you IP ?");
            return retval;
        }
        else
        {
            memset(sock_port_str,0,sizeof(sock_port_str));
            strcpy(sock_port_str, port_str);
            ESP_LOGW(TAG, "config link ip[xx.xx.xx.xx:%s]",sock_port_str);
        }
    }
    else
    {
        if (tcp_server_sock > 0)
        {
            tcp_server_sock = 0;
            ESP_LOGW(TAG, "close sock");
        }
    }
    return retval;
}

/*
    使用前确保网络通畅
*/
int tcp_server_send_data(uint8_t *data, int size)
{
    int retval = 0;
    int len = size;
    if (data == NULL || size == 0)
    {
        retval = 1;
        return retval;
    }
    
    if (tcp_server_sock > 0)
    {
        int to_write = len;
        while (to_write > 0) 
        {
            int written = send(tcp_server_sock, data + (len - to_write), to_write, 0);
            if (written < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            }
            to_write -= written;
        }
    }
    return retval;
}

/*
    接收回调函数执行指针
*/
static D_Callback_pFun tcp_server_Callback_Fun = NULL;
/*
    接收回调函数绑定
*/
void tcp_server_receive_State_Machine_Bind (D_Callback_pFun Callback_pFun)
{
    tcp_server_Callback_Fun = Callback_pFun;
}

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[128];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);  //  这里是阻塞的，想退出此函数，得靠定时器监控来kill
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            tcp_server_sock = 0;
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else 
        {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            // ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
            // tcp_server_send_data((uint8_t *)rx_buffer, len);
            if (tcp_server_Callback_Fun != NULL)
            {
                for (int i = 0; i < len; i++)
                {
                    tcp_server_Callback_Fun(rx_buffer + i);
                }
            }
        }
    } while (len > 0);
}

/*
    网络的应用层任务必须先确保底层网络是启动的，否则不应该启动这个任务
*/
void tcp_server_link_task(void *empty)
{
    char addr_str[128];
    int addr_family = (int)AF_INET;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;
    int ip_port = 0;
    int temp_num = 0;
    do
    {
        if (wifi_get_local_ip_status(NULL,NULL,NULL))
        {
            temp_num = 1;
        }
        if (eth_get_local_ip_status(NULL,NULL,NULL))
        {
            temp_num += 2;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    } while (temp_num == 0);        // 等待网络连接

    if (strlen(sock_port_str) == 0)
    {
        strcpy(sock_port_str,"8160");
    }
    ip_port = atoi(sock_port_str);
    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(ip_port);
        ip_protocol = IPPROTO_IP;
    }
#ifdef CONFIG_EXAMPLE_IPV6
    else if (addr_family == AF_INET6) {
        struct sockaddr_in6 *dest_addr_ip6 = (struct sockaddr_in6 *)&dest_addr;
        bzero(&dest_addr_ip6->sin6_addr.un, sizeof(dest_addr_ip6->sin6_addr.un));
        dest_addr_ip6->sin6_family = AF_INET6;
        dest_addr_ip6->sin6_port = htons(ip_port);
        ip_protocol = IPPROTO_IPV6;
    }
#endif

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        tcp_server_sock = 0;
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, open port %d", ip_port);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1)
    {
        ESP_LOGI(TAG, "Socket listening");
        temp_num = 0;
        do
        {
            if (wifi_get_local_ip_status(NULL,NULL,NULL))
            {
                temp_num = 1;
            }
            if (eth_get_local_ip_status(NULL,NULL,NULL))
            {
                temp_num += 2;
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        } while (temp_num == 0);        // 等待网络连接
        if (temp_num)
        {
            ESP_LOGW(TAG, "get network ID [%d]",temp_num);
        }
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
#ifdef CONFIG_EXAMPLE_IPV6
        else if (source_addr.ss_family == PF_INET6) {
            inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
        ESP_LOGI(TAG, "Socket accepted host ip address: %s", addr_str);
        ESP_LOGI(TAG, "Socket num: %d", sock);
        tcp_server_sock = sock;
        do_retransmit(sock);
        ESP_LOGW(TAG, "loss sock");
        shutdown(sock, 0);
        close(sock);
        sock = 0;
        tcp_server_sock = sock;
    }

CLEAN_UP:
    tcp_server_sock = 0;
    close(listen_sock);
    ESP_LOGW(TAG, "Socket kill");
    vTaskDelete(NULL);
}
