#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
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
#include "freertos/queue.h"
#include "system_parameter.h"
#include "reader_typedef.h"
#include "frame_analyse.h"
#include "extern_include.h"

#include "all_task_h.h"

static const char *TAG = " rj45_tcp_client_task";
int rj45_tcp_client_sock = 0;
int wifi_tcp_client_sock = 0;
QueueHandle_t rj45_socket_tcp_client_event_queue = NULL;
QueueHandle_t wifi_socket_tcp_client_event_queue = NULL;

TaskHandle_t rj45_TCPClientTxTaskHandle = NULL;
TaskHandle_t rj45_TCPClientListeningRxTaskHandle = NULL;

TaskHandle_t wifi_TCPClientTxTaskHandle = NULL;
TaskHandle_t wifi_TCPClientListeningRxTaskHandle = NULL;

char rj45_tcp_client_ENLink_flag = 1;

void printf_log_err(int err)
{
    switch (err)
    {
    case 104:
        // 上位机Demo关闭了连接时候，会产生一个104的错误代码
        ESP_LOGW(TAG, "错误代码104: PC API close socket connnect.");
        // 当TCP连接的进程在忘记关闭Socket而退出、程序崩溃、或非正常方式结束进程的情况下（Windows客户端），会导致TCP连接的对端进程产生“104: Connection reset by peer”
        break;
    case 113:
        ESP_LOGW(TAG, "错误代码113: rj45 connector is disconnected.");
        break;

        break;
    default:
        break;
    }
}

int get_client_status (void)
{
    int retval = system_para.TCP_Link_status[0] + system_para.TCP_Link_status[1] + system_para.TCP_Link_status[4];
    // ESP_LOGW(TAG, "get client sock : %d",retval);
    return retval;
}

//!!!! 1.4.3
void time_wifi_Callback_Fun (TimerHandle_t xtime)    
{
    if(system_para.connect_state_timer_counter > 10)
    {
        ESP_LOGW("tcp_client", "wifi heart over ! ");
        shutdown(wifi_tcp_client_sock, 0);
        close(wifi_tcp_client_sock);
        system_para.TCP_Link_status[1] = 0;     // Callback wifi
        wifi_tcp_client_sock = 0;
        system_para.connect_state_timer_counter = 8;
    }

}

void time_rj45_Callback_Fun (TimerHandle_t xtime)    
{
    if(system_para.connect_state_timer_counter > 10)
    {
        ESP_LOGW("tcp_client", "rj45 heart over ! ");
        shutdown(rj45_tcp_client_sock, 0);
        close(rj45_tcp_client_sock);
        system_para.TCP_Link_status[0] = 0;     // Callback rj45
        rj45_tcp_client_sock = 0;
        system_para.connect_state_timer_counter = 8;
    }

}

void rj45_tcp_client_close_task(void)
{
    if (rj45_socket_tcp_client_event_queue != NULL)
    {
        if (rj45_tcp_client_ENLink_flag)
        {
            vTaskDelay(1000);
            ESP_LOGW("tcp_client", " client connect DIS ! ");
            if (rj45_tcp_client_sock != 0)
            {
                shutdown(rj45_tcp_client_sock, 0);
                close(rj45_tcp_client_sock);
                rj45_tcp_client_sock = 0;
                system_para.TCP_Link_status[0] = 0; // 
            }
            rj45_tcp_client_ENLink_flag = 0;
        }
    }

}

void rj45_tcp_client_link_task(void)
{
    if (rj45_socket_tcp_client_event_queue != NULL)
    {
        if (rj45_tcp_client_ENLink_flag == 0)
        {
            ESP_LOGW("tcp_client", " client connect EN ! ");
            rj45_tcp_client_ENLink_flag = 1;
        }
    }
}

int rj45_tcp_client_connect_port = 8160;
uint8_t lost_connect_coutner = 0;
extern uint8_t judge_wifi_or_rj45_ip_address(void);
#if SC_CUSTOM
SC_info_packet_Type client_standard = {
    .Head = 0x7E04,
    .End = 0x7E05,
};
SC_info_packet_Type tcp_client_SC_packet;
SC_class_info_packet_Type tcp_client_SC_class_packet;
#endif
static void rj45_tcp_client_rx_task(void *pvParameters)
{
    TimerHandle_t rj45_timerhanlde;         //!!!! time 1.4.3
    char rx_buffer[256];
    uint8_t temp_array[300];
    int addr_family = 0;
    int ip_protocol = 0;
    int temp_num = 0;
    uint8_t temp_data = 0;
    lost_connect_coutner = 0;
    rj45_tcp_client_sock = 0;
    rj45_tcp_client_connect_port = atoi(system_para.remote_ip_tcp_server_port);
    vTaskDelay(4000 / portTICK_PERIOD_MS);
    ESP_LOGI("RT45 client", "远程的端口号字符串=%s", system_para.remote_ip_tcp_server_port);
    ESP_LOGI("RJ45 client", "远程的端口号是=%d", rj45_tcp_client_connect_port);
    ESP_LOGI("RJ45 client", "等待通过RJ45网线或者WiFi获取网络地址");
    uint8_t flag = 1;
    vTaskDelay(100);
    rj45_timerhanlde = xTimerCreate("timer-[rj45]",3000,pdTRUE,(void *)0,time_rj45_Callback_Fun);       //!!!! time 1.4.3

    while (flag)
    {
        flag = judge_wifi_or_rj45_ip_address();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI("RJ45 client", "已经获取到了网络IP地址");
    system_para.TCP_Link_status[0] = 0;
    while (1)
    {
        struct sockaddr_in dest_addr;

        if (strlen(system_para.remote_ip) > 5)
        {
            dest_addr.sin_addr.s_addr = inet_addr(system_para.remote_ip);
            // if(system_para.rj45_connect_state == 1)
            //  ESP_LOGW(TAG, "dest_addr.sin_addr.s_addr= %d", dest_addr.sin_addr.s_addr);
        }
        else
            dest_addr.sin_addr.s_addr = inet_addr(system_para.remote_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(rj45_tcp_client_connect_port);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

    RESART:

        if (system_para.rj45_connect_state == 1)
        {
            if (rj45_tcp_client_sock != 0)
            {
                shutdown(rj45_tcp_client_sock, 0);
                close(rj45_tcp_client_sock);
            }

            if(rj45_tcp_client_ENLink_flag)
            {
                rj45_tcp_client_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
                if (rj45_tcp_client_sock < 0)
                {
                    ESP_LOGE(TAG, "1、Socket unable to connect: errno %d, connect host ip:%s   connect port is:%d", errno, system_para.remote_ip, rj45_tcp_client_connect_port);
                    xEventGroupWaitBits(system_gxwl_myself_events, myself_events_rj45_client_connect_event, pdTRUE, pdTRUE, 1); // 将client 连接状态清零
                    shutdown(rj45_tcp_client_sock, 0);
                    close(rj45_tcp_client_sock);
                    rj45_tcp_client_sock = 0;
                    system_para.TCP_Link_status[0] = 0;
                    vTaskDelay(2000); // 2秒后重试
                    goto RESART;
                }
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
            else
            {
                vTaskDelay(5000); // 5秒后重试
                goto RESART;        //!!!!
            }

            // ESP_LOGI(TAG, "Socket created, connecting to 192.168.2.51:%d", rj45_tcp_client_connect_port);
            int err = connect(rj45_tcp_client_sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
            if (err != 0)
            {
                ESP_LOGE(TAG, "2、Socket unable to connect: errno %d, connect host ip:%s   connect port is:%d", errno, system_para.remote_ip, rj45_tcp_client_connect_port);
                if (errno == 113) // 113还没连接拔网线
                {
                    ESP_LOGE(TAG, "socket 没有连接上,有可能是上位机没有开启服务器,关闭现在建立的SOCKET,重新尝试");
                    shutdown(rj45_tcp_client_sock, 0);
                    close(rj45_tcp_client_sock);
                    rj45_tcp_client_sock = 0;
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }

                xEventGroupWaitBits(system_gxwl_myself_events, myself_events_rj45_client_connect_event, pdTRUE, pdTRUE, 1); // 将client 连接状态清零
                shutdown(rj45_tcp_client_sock, 0);
                close(rj45_tcp_client_sock);
                rj45_tcp_client_sock = 0;
                vTaskDelay(2000 / portTICK_PERIOD_MS);

                goto RESART;
            }
            ESP_LOGW(TAG, "tcp client successfully connected to tcp server, remote host ip:%s, connect port:%d ", system_para.remote_ip, rj45_tcp_client_connect_port);
            
            // 建立了client连接，相关标志位标记一下
            xEventGroupSetBits(system_gxwl_myself_events, myself_events_rj45_client_connect_event);
            system_para.TCP_Link_status[0] = 1; // rj45
            while (1)
            {
                xTimerStart(rj45_timerhanlde,1000);       //!!!! time 1.4.3
                int len = recv(rj45_tcp_client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);

                if ((len < 0) || (len == 0))
                {
                    ESP_LOGE(TAG, "recv failed: errno %d", errno);
                    system_para.TCP_Link_status[0] = 0;
                    if (errno == 104) // server正常断开连接
                    {
                        lost_connect_coutner++;
                        // 在tcp_client模式下,有时候远程服务器可以连接上,但发送的数据导致客户端接收数据小于0,这里统计一下次数,超过20次，重新启动一下
                        if (lost_connect_coutner > 10)
                            esp_restart();
                    }
                    else if (errno == 113) // 连接成功了拔网线,在client模式下检测到有时候客户连接Wifi和有线网络,会发同样的事件过来
                    {
                        ESP_LOGE(TAG, "这里主动重新启动一下");
                        {
                            mengjing_heartackage_restart();
                        }
                        esp_restart();
                    }
                    break;
                }
                else
                {
                    lost_connect_coutner = 0;
                    char temp_str[50];
                    for (int i = 0; i < len; i++)
                    {
                        temp_data = rx_buffer[i];
                        temp_num = frame_recv_data_analyse(RJ45_CLIENT_LINK, &temp_data, 1);
    #if SC_CUSTOM
                        SC_info_Make_packet_Fun(client_standard, &tcp_client_SC_packet, temp_data);
                        if (tcp_client_SC_packet.Result & 0x50)
                        {
                            // ESP_LOGW("SC info", "tcp_client_SC_packet Result");
                            Encrypt_AES_decrypt_Fun (&tcp_client_SC_packet.Data[2], tcp_client_SC_packet.dSize,temp_array,&temp_num);     // 还原数据
                            // ESP_DEBUG_Hex_Fun (&tcp_client_SC_packet.Data[2],tcp_client_SC_packet.dSize);
                            // ESP_DEBUG_Hex_Fun (temp_array,temp_num);
                            
                            SC_class_info_Make_packet_Fun (temp_array,&tcp_client_SC_class_packet);         // 还原数据处理
                            switch (tcp_client_SC_class_packet.type_cmd)       // 配置时间
                            {
                            case 2:         // other
                                /* code */
                                break;
                            case 3:         // time
                            {
                                memset(temp_str,0,sizeof(temp_str));
                                if (tcp_client_SC_class_packet.ret_len <= 0x0d && tcp_client_SC_class_packet.ret_len >= 4)
                                {
                                    memcpy(temp_str,tcp_client_SC_class_packet.Data,tcp_client_SC_class_packet.ret_len - 3);
                                    temp_str[10] = 0;
                                    temp_num = atoi(temp_str);
                                    system_para.system_rtc_second_counter = temp_num;
                                    ESP_LOGI("SC info", "%d,get time :%s-->\n ",temp_num,temp_str);
                                    SC_online_config (1);
                                }
                            }
                                break;
                            default:
                                break;
                            }

                            SC_info_packet_clean_Fun(&tcp_client_SC_packet);
                            SC_class_info_packet_clean_Fun(&tcp_client_SC_class_packet);
                        }
    #endif
                    }
                }

                vTaskDelay(1 / portTICK_PERIOD_MS);
            }

            if (rj45_tcp_client_sock != -1)
            {
                ESP_LOGE(TAG, "Shutting down socket and restarting...");
                shutdown(rj45_tcp_client_sock, 0);
                close(rj45_tcp_client_sock);
                rj45_tcp_client_sock = 0;
                system_para.TCP_Link_status[0] = 0;
            }
        }
        else
            vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    ESP_LOGE(TAG, "RJ45有线网络线程Client模式下运行出错,跳转到了关闭网络线程的地方,这里主动重新启动一下");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_restart();
}

static void rj45_tcp_client_tx_task(void *pvParameters)
{
    gxwl_socket_message_event_t rj45_tcp_client_tx_event;
    while (1)
    {

        if (rj45_tcp_client_sock > 0)
        { // 建立了sock通讯

            if (xQueueReceive(rj45_socket_tcp_client_event_queue, (void *)&rj45_tcp_client_tx_event, (portTickType)portMAX_DELAY))
            {
                switch (rj45_tcp_client_tx_event.type)
                {
                case TX_DATA: // 有数据需要发送
                    if (rj45_tcp_client_tx_event.size > 0)
                    {
                        send(rj45_tcp_client_sock, (uint8_t *)rj45_tcp_client_tx_event.param, rj45_tcp_client_tx_event.size, 0);
                        free(rj45_tcp_client_tx_event.param);
                    }
                    break;
                default:
                    break;
                }
            }
        }
        else
            vTaskDelay(10);
    }
}

void rj45_tcp_client_task_init(void)
{
    if (rj45_socket_tcp_client_event_queue == NULL)
        rj45_socket_tcp_client_event_queue = xQueueCreate(10, sizeof(gxwl_socket_message_event_t)); // 定义一个消息队列,能传递10个消息，每一个消息大小等于s
    if (rj45_TCPClientTxTaskHandle == NULL)
        xTaskCreate(rj45_tcp_client_tx_task, "tcp_tx_client", 2048, NULL, rj45_tcp_client_task_tx_priority, &rj45_TCPClientTxTaskHandle);
    if (rj45_TCPClientListeningRxTaskHandle == NULL)
    {
        xTaskCreate(rj45_tcp_client_rx_task, "tcp_rx_client", 1024*6, NULL, rj45_tcp_client_task_rx_priority, &rj45_TCPClientListeningRxTaskHandle);
    }
}

#define WIFI_TAG "wifi_tcp_client_task"


// 先连wifi 再连网口 是没有问题的 因为网口任务会将wifi任务挂起
// 先练网口 再开启wifi可能有问题。

static void wifi_tcp_client_rx_task(void *pvParameters)
{
    TimerHandle_t wifi_timerhanlde;     //!!!! time 1.2.3
    char rx_buffer[256];
    int addr_family = 0;
    int ip_protocol = 0;
    int counter = 0;
    uint8_t temp_data = 0;
    rj45_tcp_client_connect_port = atoi(system_para.remote_ip_tcp_server_port);
    vTaskDelay(5000); // 等待5秒
    wifi_timerhanlde = xTimerCreate("timer-[wifi]",2000,pdTRUE,(void *)0,time_wifi_Callback_Fun);       //!!!! time 1.4.3
    system_para.TCP_Link_status[1] = 0;
    while (1)
    {

        struct sockaddr_in dest_addr;

        if (strlen(system_para.remote_ip) > 5)
        {
            dest_addr.sin_addr.s_addr = inet_addr(system_para.remote_ip);
            // if(system_para.rj45_connect_state == 1)
            //  ESP_LOGW(TAG, "dest_addr.sin_addr.s_addr= %d", dest_addr.sin_addr.s_addr);
        }
        else
        {
            dest_addr.sin_addr.s_addr = inet_addr(system_para.remote_ip);
        }
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(rj45_tcp_client_connect_port);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        ESP_LOGW("prt connect head run7", "%d ", system_para.connect_state_timer_counter);
    WIFIRESART:

        if (system_para.wifi_connect_state == 1)
        {
            if (wifi_tcp_client_sock != 0)
            {
                shutdown(wifi_tcp_client_sock, 0);
                close(wifi_tcp_client_sock);
            }

            wifi_tcp_client_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
            if (wifi_tcp_client_sock < 0)
            {
                ESP_LOGE(WIFI_TAG, "1、Socket unable to connect: errno %d, connect host ip:%s   connect port is:%d", errno, system_para.remote_ip, wifi_tcp_client_sock);
                xEventGroupWaitBits(system_gxwl_myself_events, myself_events_wifi_client_connect_event, pdTRUE, pdTRUE, 1); // 将client 连接状态清零
                shutdown(wifi_tcp_client_sock, 0);
                close(wifi_tcp_client_sock);
                wifi_tcp_client_sock = 0;
                system_para.TCP_Link_status[1] = 0;
                vTaskDelay(10000); // 10秒后重试
                goto WIFIRESART;
            }

            int err = connect(wifi_tcp_client_sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
            if (err != 0)
            {
                ESP_LOGE(WIFI_TAG, "2、Socket unable to connect: errno %d, connect host ip:%s   connect port is:%d", errno, system_para.remote_ip, rj45_tcp_client_connect_port);
                if (errno == 113) // 113还没连接拔网线
                {
                    ESP_LOGE(WIFI_TAG, "123这里主动重新启动一下");
                    shutdown(wifi_tcp_client_sock, 0);
                    close(wifi_tcp_client_sock);
                    wifi_tcp_client_sock = 0;
                }

                xEventGroupWaitBits(system_gxwl_myself_events, myself_events_wifi_client_connect_event, pdTRUE, pdTRUE, 1); // 将client 连接状态清零
                shutdown(wifi_tcp_client_sock, 0);
                close(wifi_tcp_client_sock);
                wifi_tcp_client_sock = 0;
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                goto WIFIRESART;
            }

            ESP_LOGW(WIFI_TAG, "tcp client successfully connected to tcp server, remote host ip:%s, connect port:%d ", system_para.remote_ip, rj45_tcp_client_connect_port);
            
            // 建立了client连接，相关标志位标记一下
            xEventGroupSetBits(system_gxwl_myself_events, myself_events_wifi_client_connect_event);
            system_para.TCP_Link_status[1] = 1; // wifi
            while (1)
            {
                xTimerStart(wifi_timerhanlde,1000);       //!!!! time 
                int len = recv(wifi_tcp_client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);

                if ((len < 0) || (len == 0))
                {
                    ESP_LOGE(WIFI_TAG, "recv failed: errno %d", errno);
                    system_para.TCP_Link_status[1] = 0;
                    if (errno == 104) // server正常断开连接
                    {
                        lost_connect_coutner++;
                        // 在tcp_client模式下,有时候远程服务器可以连接上,但发送的数据导致客户端接收数据小于0,这里统计一下次数,超过20次，重新启动一下
                        if (lost_connect_coutner > 10)
                            esp_restart();
                    }
                    else if (errno == 113) // 连接成功了拔网线
                    {
                        ESP_LOGE(WIFI_TAG, "123这里主动重新启动一下");
                        shutdown(wifi_tcp_client_sock, 0);
                        close(wifi_tcp_client_sock);
                        wifi_tcp_client_sock = 0;
                        // vQueueDelete(wifi_socket_tcp_client_event_queue);
                        // vTaskDelete(wifi_TCPClientTxTaskHandle);
                        wifi_socket_tcp_client_event_queue = NULL;
                        wifi_TCPClientTxTaskHandle = NULL;
                        wifi_TCPClientListeningRxTaskHandle = NULL;
                        vTaskDelay(1000 / portTICK_PERIOD_MS);
                        esp_restart();
                    }
                    break;
                }
                else
                {
                    lost_connect_coutner = 0;
                    for (counter = 0; counter < len; counter++)
                    {
                        temp_data = rx_buffer[counter];
                        frame_recv_data_analyse(WIFI_CLIENT_LINK, &temp_data, 1);
                    }
                }
            }
            if (wifi_tcp_client_sock != -1)
            {
                ESP_LOGE(WIFI_TAG, "Shutting down socket and restarting...");
                shutdown(wifi_tcp_client_sock, 0);
                close(wifi_tcp_client_sock);
                wifi_tcp_client_sock = 0;
                system_para.TCP_Link_status[1] = 0;
            }
        }
        else
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void wifi_tcp_client_tx_task(void *pvParameters)
{

    gxwl_socket_message_event_t wifi_tcp_client_tx_event;
    while (1)
    {

        if (wifi_tcp_client_sock > 0)
        { // 建立了sock通讯

            if (xQueueReceive(wifi_socket_tcp_client_event_queue, (void *)&wifi_tcp_client_tx_event, (portTickType)portMAX_DELAY))
            {
                //   ESP_LOGW("WiFi client","接收到了数据");

                switch (wifi_tcp_client_tx_event.type)
                {
                case TX_DATA: // 有数据需要发送
                    if (wifi_tcp_client_tx_event.size > 0)
                    {
                        send(wifi_tcp_client_sock, (uint8_t *)wifi_tcp_client_tx_event.param, wifi_tcp_client_tx_event.size, 0);
                        free(wifi_tcp_client_tx_event.param);
                    }
                    break;
                default:
                    break;
                }
            }
        }
        else
            vTaskDelay(10);
    }
}

void wifi_tcp_client_task_init(void)
{
    if (wifi_socket_tcp_client_event_queue == NULL)
        wifi_socket_tcp_client_event_queue = xQueueCreate(10, sizeof(gxwl_socket_message_event_t));
    if (wifi_TCPClientTxTaskHandle == NULL)
        xTaskCreate(wifi_tcp_client_tx_task, "wif_tx_client", 4096, NULL, wifi_tcp_client_task_tx_priority, &wifi_TCPClientTxTaskHandle);
    if (wifi_TCPClientListeningRxTaskHandle == NULL)
        xTaskCreate(wifi_tcp_client_rx_task, "wif_rx_client", 4096, NULL, wifi_tcp_client_task_rx_priority, &wifi_TCPClientListeningRxTaskHandle);
}
