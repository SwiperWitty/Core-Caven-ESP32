/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "system_parameter.h"
#include "reader_typedef.h"
#include "frame_analyse.h"
#include "extern_include.h"

TaskHandle_t rj45_TCPServerListeningRxTaskHandle = NULL;
TaskHandle_t rj45_TCPServerTxTaskHandle;
TaskHandle_t rj45_TCPServerRxTaskHandle;

int rj45_tcp_server_sock = 0; //等待客户端连接过来的sock变量
tcp_server_connect_struct  rj45_server_connect_state;
uint8_t tcp_server_port_connect_counter_when_using = 0;
int listen_sock = 0;

#define CONFIG_EXAMPLE_IPV4 1
#define PORT 4001
#define KEEPALIVE_IDLE 20    //如果该连接在60秒内没有任何数据往来，则进行探测
#define KEEPALIVE_INTERVAL 5 //探测时法宝的时间间隔为5秒
#define KEEPALIVE_COUNT 3    //探测阐释次数为3， 假如第一次探测包就收到了响应,则后面2次不再发

#define TCP_SERVER_CONNECT_MAX_NUMBER    2

static const char *TAG = "rj45_tcp_server_task";
//创建普通队列
uint8_t port_8160_using = 0;
QueueHandle_t rj45_socket_tcp_server_event_queue = NULL;
int rj45_tcp_server_listing_port = 0;


void send_kill_wifi_task_signal(void)
{
    //运行到了这里,说明网线已经插入,并且从路由器上分配到了IP地址， 按照读写器规则，这里则需要将WiFi的连接断开
    //网络连接上了,那就需要启动有线网络的TCP_Server进程了
    xEventGroupSetBits(system_gxwl_myself_events, myself_events_need_kill_wifi_tcp_server_task); //需要结束wifi任务,启动rj45 tcp server，重新监听
    //延时200ms,等待wifi tcp server任务结束
    vTaskDelay(2000 * 100);
}

void kill_rj45_all_task(void)
{
    //检测到网线插入后,强制结束wifi相关进程
    //首先关闭监听端口
    shutdown(rj45_server_connect_state.socket_id[0], 0);
    close(rj45_server_connect_state.socket_id[0]);
    shutdown(rj45_server_connect_state.socket_id[1], 0);
    close(rj45_server_connect_state.socket_id[1]);

    system_para.TCP_Link_status[2] = 0;
    system_para.TCP_Link_status[3] = 0;
    ESP_LOGW("杀掉RJ45网络所有进程", "关闭socket监听端口");
    // 设置rj45 tcp连接 失效
    xEventGroupSetBits(system_gxwl_myself_events, myself_events_rj45_tcp_server_disconnect);
    // 设置tcpserver 连接已经关闭连接， 清除tcp_server已经连接的事件
    xEventGroupWaitBits(system_gxwl_myself_events, myself_events_rj45_tcp_server_connect, pdTRUE, pdTRUE, 0);
    vTaskDelay(100);
}

void printf_rj45_tcp_connect_err_code(int code)
{
    switch (code)
    {
    case 104:
        //上位机Demo关闭了连接时候，会产生一个104的错误代码
        ESP_LOGW(TAG, "错误代码104: PC API close socket connnect.");
        //当TCP连接的进程在忘记关闭Socket而退出、程序崩溃、或非正常方式结束进程的情况下（Windows客户端），会导致TCP连接的对端进程产生“104: Connection reset by peer”
        break;
    case 113:
        ESP_LOGW(TAG, "错误代码113: rj45 connector is disconnected.");
        break;
    case 23:
        ESP_LOGW(TAG, "错误代码23:工作在Server模式下,ESP32建立SOCKET数量用完了,现在有接收到其他设备建立SOCKET的请求,会有23错误");
        esp_timer_start_once(system_restart_timer, 1000 * 200); // 200ms
        break;
    case 128:
        ESP_LOGW(TAG, "错误代码128:工作在Server模式下,SOCKET已经建立了,可能是没有收到心跳包,Server主动杀死了已经建立的Socket就会收到这个错误代码");
        break;
    case 9:
        ESP_LOGW(TAG, "错误代码9:工作在Server模式下,远程Client端关闭的时候,有的TCP助手Client关闭会传这个错误事件上来,有的不会");

        break;
    default:
        break;
    }
}

extern esp_err_t read_system_parameter(char *param_type, char *para, size_t read_len);
extern esp_err_t write_system_parameter(char *param_type, char *para);
extern esp_err_t write_system_rj45_parameter(char *param_type, uint8_t *byte_value);
extern esp_err_t read_system_rj45_parameter(char *param_type, uint8_t *para);
extern int tcp_server_send_data(uint8_t *data, int size);

static void rj45_tcp_server_first_rx_task(void *parameter)
{
    int len;
    char rx_buffer[128];
    uint8_t temp_data;
    uint16_t temp_num = 0,temp_crc;
    int temp_addr;
    int sock=rj45_server_connect_state.socket_id[0];
    uint8_t temp_array[500];
    
    do
    {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0); // recv 是阻塞式的接收
        
        if (len < 0)
        {
            ESP_LOGE("RJ45 rx app", "Error occurred during receiving: errno %d", errno);
            printf_rj45_tcp_connect_err_code(errno);
            system_para.TCP_Link_status[2] = 0;
        }
        else if (len == 0)
        {
            ESP_LOGW("RJ45 rx app", "Connection closed");
            system_para.TCP_Link_status[2] = 0;
            break;
        }
        else
        {
            system_para.TCP_Link_status[2] = 1;     // rj45
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string

            for (int i = 0; i < len; i++)
            {
                temp_data = rx_buffer[i];

                temp_num = frame_recv_data_analyse(RJ45_TCP_LINK, &temp_data, 1);
            }
        }
    } while (len > 0); // 当len<0 或者len=0时候,此函数就会返回,执行关掉socket，重新监听
    if(len<0)
    ESP_LOGI("RJ45接收端程序","接收到了数据长度小于0的消息,删除接收和发送任务Task,等待新连接的时候再重新建立");
    else
    ESP_LOGE("RJ45接收端程序", "接收到了数据长度等于0的消息,上位机正常关闭Socket,删除接收和发送任务Task,等待新连接的时候再重新建立");
    system_para.current_order_from_channel = RS232_LINK;    //!!!! 回落数据通道
    ESP_LOGI("current channel","RS232_LINK");
    system_para.TCP_Link_status[2] = 0;
    
    rj45_server_connect_state.socket_id[0]=0;
    // rj45_server_connect_state.connect_state[0] = 0;

    shutdown(sock,0);
    close(sock);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    xEventGroupSetBits(system_gxwl_myself_events, myself_events_rj45_tcp_server_disconnect);
    xEventGroupWaitBits(system_gxwl_myself_events, myself_events_rj45_tcp_server_connect, pdTRUE, pdTRUE, 1);

    port_8160_using = 0;
    vTaskDelete(rj45_TCPServerTxTaskHandle);    //删除对应的发送程序task
    vTaskDelete(NULL);
}

static void rj45_tcp_server_first_tx_task(void *pvParameters)
{
    gxwl_socket_message_event_t rj45_tcp_socket_tx_event;
    int sock = rj45_server_connect_state.socket_id[0];
    while (1)
    {

        if (sock > 0)
        { // 建立了sock通讯

            if (xQueueReceive(rj45_socket_tcp_server_event_queue, (void *)&rj45_tcp_socket_tx_event, (portTickType)portMAX_DELAY))
            {
                switch (rj45_tcp_socket_tx_event.type)
                {
                case TX_DATA: // 有数据需要发送
                    //ESP_LOGI("rj45_tcp_server_first_tx_task","接收到了数据,需要发送到上位机");
                    if (rj45_tcp_socket_tx_event.size > 0)
                    { 
                        tcp_server_port_connect_counter_when_using = 0;
                        send(sock, (uint8_t *)rj45_tcp_socket_tx_event.param, rj45_tcp_socket_tx_event.size, 0);
                        free(rj45_tcp_socket_tx_event.param);
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

static void rj45_tcp_server_socket_manager_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int ram_size;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;
    uint32_t erron = 0;
    uint8_t start = 0;
    // u_long non_blocking=1;
    uint8_t flag=1;
    vTaskDelay(100);
    ESP_LOGI("Tcp 服务socket监听管理程序", "等待获取网络IP地址,再启动网络监听");
    while (flag)
    {
        flag = judge_wifi_or_rj45_ip_address();
        if (flag == 0)
        {
            break;
        }
        else
            vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI("Tcp 服务socket监听管理程序", "已经获取到了网络IP地址");
    rj45_tcp_server_listing_port = atoi(system_para.local_device_tcp_server_port); // atoi  ------字符串转为10进制数  "8160" 转换为8160

    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(rj45_tcp_server_listing_port);
        //  dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }

    listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0)
    {
        ESP_LOGE("RJ45服务器监听程序", "Unable to create socket: errno %d", errno);
        esp_restart();
        goto RJ45_CLEAN_UP;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // SO_REUSEADDR 是让端口释放后立即可以被再次利用

    ESP_LOGI("RJ45服务器监听程序", "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE("RJ45服务器监听程序", "Socket unable to bind: errno %d", errno);
        ESP_LOGE("RJ45服务器监听程序", "IPPROTO: %d", addr_family);
        esp_restart();
        goto RJ45_CLEAN_UP;
    }
    ESP_LOGI("RJ45服务器监听程序", "Socket bound, port %d", rj45_tcp_server_listing_port);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        shutdown(listen_sock, 0);
        close(listen_sock);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }
    ESP_LOGI("RJ45服务器监听程序", "Socket listening");

    xEventGroupSetBits(system_gxwl_myself_events, myself_events_rj45_tcp_server_disconnect);
    rj45_server_connect_state.connect_state[0] = 0;
    rj45_server_connect_state.socket_id[0] = 0;
    rj45_server_connect_state.connect_state[1] = 0;
    rj45_server_connect_state.socket_id[1] = 0;
    rj45_server_connect_state.connect_state[2] = 0;
    rj45_server_connect_state.socket_id[2] = 0;

    xEventGroupSetBits(system_gxwl_myself_events, myself_events_rj45_tcp_server_disconnect);
    xEventGroupWaitBits(system_gxwl_myself_events, myself_events_rj45_tcp_server_connect, pdTRUE, pdTRUE, 0);
    while (1)
    {
        system_para.connect_state_timer_counter = 0; //进入了侦听状态,说明上一个socket连接已经断开了
        //system_para.socket_disconnect_restar_board_second_counter = 0;
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        rj45_tcp_server_sock = 0;
        rj45_tcp_server_sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len); //建立连接后,会分配另外一个实际通信的socket
        ESP_LOGI("RJ45服务器监听程序", "建立的SOCKET套接字=:  %d", rj45_tcp_server_sock);
        ram_size = esp_get_free_heap_size();
        ESP_LOGW("ram log","free_heap_size = %d \n", ram_size);
        
        if (rj45_tcp_server_sock < 0)
        {
            ESP_LOGE("RJ45服务器监听程序", "Unable to accept connection: 建立套接字失败,错误代码 %d", errno);
            printf_rj45_tcp_connect_err_code(errno);
            shutdown(rj45_tcp_server_sock, 0);
            close(rj45_tcp_server_sock);
          
        }
        else if (ram_size < 4096*8)
        {
            shutdown(rj45_tcp_server_sock, 0);
            close(rj45_tcp_server_sock);
            ESP_LOGE("RJ45","free heap min");
        }
        else
        {
            if (system_para.TCP_Link_status[2] == 0)
            {
                // rj45_server_connect_state.connect_state[0] = 1;
                
                rj45_server_connect_state.socket_id[0] = rj45_tcp_server_sock;
                //这里新建一个socket接收任务,完成recv阻塞接收
                setsockopt(rj45_server_connect_state.socket_id[0], SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
                setsockopt(rj45_server_connect_state.socket_id[0], IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
                setsockopt(rj45_server_connect_state.socket_id[0], IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
                setsockopt(rj45_server_connect_state.socket_id[0], IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
                tcp_server_port_connect_counter_when_using = 0;
                if (source_addr.ss_family == PF_INET)
                {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
                }

                ESP_LOGI("RJ45服务器监听程序", "RJ45 network socket accepted ip address: %s", addr_str);
                system_para.connect_state_timer_counter = 0; // 进入了侦听状态,说明上一个socket连接已经断开了
                //system_para.socket_disconnect_restar_board_second_counter = 0;

                xEventGroupSetBits(system_gxwl_myself_events, myself_events_rj45_tcp_server_connect);
                xEventGroupWaitBits(system_gxwl_myself_events, myself_events_rj45_tcp_server_disconnect, pdTRUE, pdTRUE, 1);
                ESP_LOGI("RJ45服务器监听程序", "检测到了新的TCP连接,建立新的接收和发送线程Task");
                vTaskDelay(100 / portTICK_PERIOD_MS);
                xTaskCreate(rj45_tcp_server_first_rx_task, "rj45_first_recv", 4096*2, (void *)AF_INET, rj45_tcp_server_first_rx_task_priority, &rj45_TCPServerRxTaskHandle);
                xTaskCreate(rj45_tcp_server_first_tx_task, "rj45_first_tx", 4096, (void *)AF_INET, rj45_tcp_server_tx_task_priority, &rj45_TCPServerTxTaskHandle);
                system_para.TCP_Link_status[2] = 1;     // rj45
            }
            else if (rj45_server_connect_state.connect_state[1] == 0)
            {
              
                // 有第二个客户端连接进来了,直接关掉
                rj45_server_connect_state.connect_state[1] = 0;
                rj45_server_connect_state.socket_id[1] = 0;
                shutdown(rj45_tcp_server_sock, 0);
                close(rj45_tcp_server_sock);
                ESP_LOGI("RJ45服务器监听程序", "检测到了第二个TCP连接, 已经正常流程关闭了对应的SOCKET套接字");
            }
        }
    }

RJ45_CLEAN_UP:
    shutdown(listen_sock, 0);
    close(listen_sock);
    shutdown(rj45_server_connect_state.socket_id[0],0);
    close(rj45_server_connect_state.socket_id[0]);
    shutdown(rj45_server_connect_state.socket_id[1],0);
    close(rj45_server_connect_state.socket_id[1]);
    rj45_server_connect_state.connect_state[0] = 0;
    rj45_server_connect_state.connect_state[1] = 0;
    port_8160_using = 0;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_LOGE("RJ45服务器监听程序", "RJ45有线网络线程TCPserver模式下运行出错,跳转到了关闭网络线程的地方,这里主动重新启动一下");
    printf_rj45_tcp_connect_err_code(errno);
    if(((erron == 104) || (erron == 128))||(start != 0))
    {
        ESP_LOGE("RJ45服务器监听程序", "断开连接后需要初始化");
    }
    esp_restart();
}




void rj45_tcp_server_task_init(void)
{
#ifdef rj45_tcp_server_socket_manager_task_priority

    if(rj45_TCPServerListeningRxTaskHandle == NULL)
        xTaskCreate(rj45_tcp_server_socket_manager_task, "rj45_tcp_servr", 4096, (void *)AF_INET, rj45_tcp_server_socket_manager_task_priority, &rj45_TCPServerListeningRxTaskHandle);
    if(rj45_socket_tcp_server_event_queue == NULL)
        rj45_socket_tcp_server_event_queue = xQueueCreate(10, sizeof(gxwl_socket_message_event_t)); //定义一个消息队列,能传递10个消息，每一个消息大小等于sizeof(gxwl_socket_message_event_t)

#endif

}
