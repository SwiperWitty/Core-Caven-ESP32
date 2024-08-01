/*
    前提是eth和wifi任意一个启动连接才可以使用
*/
#include "tcp_client_link.h"
#include "Network_manage.h"

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

static const char *TAG = "TCP client";

int tcp_client_link_init (int set)
{
    int retval = 0;
    if (set)
    {
        /* code */
    }
    else
    {
        
    }
    return retval;
}
void tcp_client_link_task(void *empty)
{

}