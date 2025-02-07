# FreeRTOS Real Time Stats Example

首先，这个文件源于乐鑫IDF 4.4.5 `Espressif\frameworks\esp-idf-v4.4.5\examples\system\freertos\real_time_stats` 

更新日志在git提交记录



其次，我简单汪两句（给小白）：

拿到这个文件用vscode打开你会发现此文件头文件一片红，那是因为你没让vscode知道你参考库的位置。

![image-20230913112604984](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131126036.png)

how to do it ？



vscode打开工程文件夹，使用按住`ctrl` `shift` 再按 `p` ，你就会触发如图所示，接下来在这个出现的框里输入`ESP-IDF:Add vscod`，选择如图所示即可。

![image-20230913112025350](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131120428.png)



此工程文件夹下的`.vscode`就会跟新，如图。

![image-20230913112349704](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131123748.png)



这样你的头文件就不会报红了。

![image-20230913112752519](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131127561.png)



功能列表

- GPIO 100%
- UART （Callback_pFun）100%
- IIC (RTC8564为例) 100%
- SPI (st7789为例) 100%
- ETH+WIFI 100%
- Client （Callback_pFun）100%
- Server （Callback_pFun）100%
- MQTT 
- HTTP+HTTPS （Callback_pFun）100%
- File 100%

软件协议层

- GUI 100%
- EAS 100%
- 缓存算法 100%



#### 通信底层基本框架

通信一般是双向的，即收发。

发送实现逻辑

发送需要数据本体、数据长度，如果有不同channel那就算上

~~~c
void MODE_UART_Send_Data_Fun(char Channel, const U8 *Data, int Length)
~~~

当然还会有缓存发送的情况（DMA）,他们的写法没有什么不一样，只不过底层机制不一样罢了。

~~~c
void MODE_UART_DMA_Send_Data_Fun(char Channel, const U8 *Data, int Length)
~~~

接收实现逻辑

接收分为两种不同方式中断和轮询，中断即自带一个线程，轮询需要在另一个线程中自行做获取处理，中断一般情况是获取单字节，轮询一般自带缓存，中断的实时性比轮询强一些，如果是给接收单开线程并阻塞接收那就当我没说。为了在应用层统一，需要引入一个回调机制：

~~~~c
typedef void (*D_pFun) (void *data);	// 有传参函数指针
void MODE_UART_Receive_Bind_Fun(char Channel, D_pFun UART_pFun);
~~~~

以中断为例，每次进入中断都执行一次绑定的`D_pFun`类型函数，所以要求`D_pFun`为非阻塞行，至于`D_pFun`咋写，也有案例：

~~~c
Caven_Watch_Type Now_time;		// 至于这个时间超时机制，那就需要系统参数了(UTC全局变量)
Caven_Watch_Type RS232_Get_last_time;
void RS232_handle_Fun (void *data)
{
    int temp_retval = 0;
    uint8_t temp_data = *(uint8_t *)data;
    int temp_num = Now_time.SYS_Sec - RS232_Get_last_time.SYS_Sec;
    if (temp_num > 1 && RS232_Get_last_time.SYS_Sec)       // 间隔超过1s
    {
        // clean packet
        RS232_Get_last_time = Now_time;
    }
    else
    {
        RS232_Get_last_time = Now_time;
    }
    // make packet
    if (temp_retval == 0XFF)
    {
    }
    else
    {
    }
}

int main (void)
{
    MODE_UART_Receive_Bind_Fun(1, RS232_handle_Fun);
    MODE_UART_Init(1,115200,1);
    while(1)
    {
        //
    }
}
~~~

如果是缓存接收，那就在线程中for循环来完成：

~~~c
/*
    接收回调函数执行指针
*/
static D_Callback_pFun custom_uart1_Callback_Fun = NULL;
/*
    接收回调函数绑定
*/
void custom_uart1_receive_State_Machine_Bind (D_Callback_pFun Callback_pFun)
{
    custom_uart1_Callback_Fun = Callback_pFun;
}

/*
    接收线程/任务
*/
void uart1_task(void *pvParam)
{
    EventBits_t r_event;
    uart_event_t event;
    TickType_t absolute_Time = 1;
    int counter = 0;
    uint8_t data[512];
    if (custom_uart1_init_flag == 0)
    {
        custom_uart1_init(115200, 1);
    }
    memset(data, 0, sizeof(data));
    while (1)
    {
        // Waiting for UART event.
        if (xQueueReceive(uart1_event_queue, (void *)&event, 10))
        {
            switch (event.type)
            {
            case UART_DATA:
            {
            }
            case UART_BUFFER_FULL:
            {
                counter = uart_read_bytes(UART_NUM_1, data, 512, 0);
                if (counter > 0)
                {
                    if (custom_uart1_Callback_Fun != NULL)
                    {
                        for (int i = 0; i < counter; i++)
                        {
                            custom_uart1_Callback_Fun(data + i);
                        }
                    }
                    // ESP_LOGI(TAG, "uart1 get %d ", counter);
                }
            }
            break;
            default:
                break;
            }
        }
        else
        {
        }
        vTaskDelay(pdMS_TO_TICKS(absolute_Time));
    }
    vTaskDelete(NULL); /*  基本不用退出 */
}

xTaskCreate(uart1_task, "task-[uart1]", 4096 * 1, NULL, UART1_TASK_PRIORITY, NULL);
~~~

当然，如果是网络协议那还涉及到网络管理，通常他们的逻辑是【1.初始化配置】->【2.初始化sock】->【3.建立网络监听】->【4.开始获取接收缓存】->【5.清理缓存】->【6.如果发生网络中断/异常清除socket】->【7.回到网络监听(3)】->【向下执行...】。由此可见网络管理和数据接收在同一个线程，并且形成前后关系，没有网络连接就不存在数据接收，所以在这个场景下会有一个独立线程/任务给网络管理并且兼数据接收。

~~~c
/*
    接收回调函数执行指针
*/
static D_pFun tcp_server_Callback_Fun = NULL;
/*
    接收回调函数绑定
*/
void tcp_server_receive_State_Machine_Bind (D_pFun Callback_pFun)
{
    tcp_server_Callback_Fun = Callback_pFun;
}

/*
    enable 0,会关闭当前sock，没有则不生效
    enable 1,打开server_link,并重置str,若此时str为NULL，理解为询问tcp_server_sock
    server 只能修改端口(改完请重启)，如果需要修改ip请修改[eth_config_ip]/[wifi_config_ip]
*/
int tcp_server_link_config (char *port_str,int enable)
{
    int retval = 0;
    tcp_server_enable = enable;
    if (enable == 0)
    {
        if (tcp_server_sock > 0)
        {
            // ESP_LOGW(TAG, "config close sock <-- \n");
            shutdown(tcp_server_sock, 0);
            close(tcp_server_sock);
            tcp_server_sock = 0;
        }
    }
    else
    {
        if (port_str == NULL)
        {
            retval = tcp_server_sock;
            // ESP_LOGW(TAG, "where are you IP ?");
            return retval;
        }
        else
        {
            memset(sock_port_str,0,sizeof(sock_port_str));
            strcpy(sock_port_str, port_str);
            if (tcp_server_sock > 0)
            {
                shutdown(tcp_server_sock, 0);
                close(tcp_server_sock);
                tcp_server_sock = 0;
            }
            // ESP_LOGW(TAG, "config link ip[xx.xx.xx.xx:%s]",sock_port_str);
        }
    }
    retval = tcp_server_sock;
    return retval;
}

void *tcp_server_link_task(void *empty)
{
    int ip_port = 0;
    int temp_num = 0;
    int tcp_rst = 0;
    uint8_t wifi_ip_str[10];
    uint8_t wifi_gw_str[10];
    uint8_t wifi_netmask_str[10];
    uint8_t data[10240];
    GT_tcp_server_start:
    temp_num = 0;
    do
    {
        if (wifi_get_local_ip_status(NULL,NULL,NULL) > 0)
        {
            temp_num = 1;
        }
        if (eth_get_local_ip_status(NULL,NULL,NULL) > 0)
        {
            temp_num += 2;
        }
        if (temp_num == 0)
        {
            sleep(1);
        }
    } while (temp_num == 0 || tcp_server_enable == 0);        // 等待网络连接,否则不开启服务器
    // start

    if (strlen(sock_port_str) <= 0)
    {
        strcpy(sock_port_str,"8160");
    }
    ip_port = atoi(sock_port_str);
    if (tcp_rst)
    {
        if (tcp_rst > 1)
        {
            sleep(5);
        }
        tcp_rst = 0;
    }
    // 定义sockfd
    int server_sockfd = 0;
    // 定义sockaddr_in
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(ip_port);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 建立服务器端socket 
	if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		perror("server_sockfd creation failed");
        tcp_rst = 2;
        goto GT_tcp_server_start;
		// exit(EXIT_FAILURE);
	}
	// 设置套接字选项避免地址使用错误
	int on=1;
	if((setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed");
        tcp_rst = 2;
        goto GT_tcp_server_start;
		// exit(EXIT_FAILURE);
	}
	// 将套接字绑定到服务器的网络地址上 
	if((bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr)))<0)
	{
		perror("server socket bind failed");
        tcp_rst = 2;
        goto GT_tcp_server_start;
		// exit(EXIT_FAILURE);
	}
    printf("server port[%d]\n",8160);
    // 建立监听队列listen，成功返回0，出错返回-1
    if(listen(server_sockfd,20) == -1)
    {
        perror("listen");
        tcp_rst = 2;
        goto GT_tcp_server_start;
        // exit(1);
    }
    
    while (1)
    {
        // 客户端套接字
        struct sockaddr_in source_addr;
        socklen_t length = sizeof(source_addr);
        GT_tcp_server_accept:
        if (tcp_server_sock > 0)
        {
            shutdown(tcp_server_sock, 0);
            close(tcp_server_sock);
            tcp_server_sock = 0;
        }
        temp_num = 0;
        do
        {
            if (wifi_get_local_ip_status(NULL,NULL,NULL) > 0)
            {
                temp_num = 1;
            }
            if (eth_get_local_ip_status(NULL,NULL,NULL) > 0)
            {
                temp_num += 2;
            }
            if (temp_num == 0)
            {
                sleep(1);
            }
        } while (temp_num == 0 || tcp_server_enable == 0);	// 等待网络连接,否则不开启服务器
        printf("wait link ...\n");
        // 等待客户端连接请求到达,成功返回非负描述字，出错返回-1
        int conn = accept(server_sockfd, (struct sockaddr*)&source_addr, &length);
        if(conn<0)
        {
            perror("connect");
            goto GT_tcp_server_start;
            // exit(1);
        }
        tcp_server_sock = conn;
        printf("server linked !\n");
        do{
            if (tcp_server_sock > 0)
            {
                temp_num = recv(tcp_server_sock, data, sizeof(data),0);   // 阻塞
                if (temp_num > 0)
                {
                    if (tcp_server_Callback_Fun != NULL)
                    {
                        for (int i = 0; i < temp_num; i++)
                        {
                            tcp_server_Callback_Fun(data + i);  // pointer
                        }
                    }
                }
                else
                {
                    perror("connect loss");     // net loss
                    break;
                }
            }
            else
            {
                perror("server_sock close");
                break;
            }
        }while (1);
    }
}
~~~



Caven_info  [协议资料](https://kdocs.cn/l/cjMY23q4aET2)

如果还有问题，[请点击我](https://www.baidu.com/) 谢谢！

_____



(See the README.md file in the upper level 'examples' directory for more information about examples.)

FreeRTOS provides the function `vTaskGetRunTimeStats()` to obtain CPU usage statistics of tasks. However, these statistics are with respect to the entire runtime of FreeRTOS (i.e. **run time stats**). Furthermore, statistics of `vTaskGetRunTimeStats()` are only valid whilst the timer for run time statistics has not overflowed.

This example demonstrates how to get CPU usage statistics of tasks with respect to a specified duration (i.e. **real time stats**) rather than over the entire runtime of FreeRTOS. The `print_real_time_stats()` function of this example demonstrates how this can be achieved.

## How to use example

### Hardware Required

This example should be able to run on any commonly available ESP32 development board.

### Configure the project

```
idf.py menuconfig
```

* Select `Enable FreeRTOS to collect run time stats` under `Component Config > FreeRTOS` (this should be enabled in the example by default)

* `Choose the clock source for run time stats` configured under `Component Config > FreeRTOS`. The `esp_timer` should be selected be default. This option will affect the time unit resolution in which the statistics are measured with respect to.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

The example should have the following log output:

```
...
Getting real time stats over 100 ticks
| Task | Run Time | Percentage
| stats | 1304 | 0%
| IDLE0 | 206251 | 10%
| IDLE1 | 464785 | 23%
| spin2 | 225389 | 11%
| spin0 | 227174 | 11%
| spin4 | 225303 | 11%
| spin1 | 207264 | 10%
| spin3 | 225331 | 11%
| spin5 | 225369 | 11%
| Tmr Svc | 0 | 0%
| esp_timer | 0 | 0%
| ipc1 | 0 | 0%
| ipc0 | 0 | 0%
Real time stats obtained
...
```

## Example Breakdown

### Spin tasks

During the examples initialization process, multiple `spin` tasks are created. These tasks will simply spin a certain number of CPU cycles to consume CPU time, then block for a predetermined period.

### Understanding the stats

From the log output, it can be seen that the spin tasks consume nearly an equal amount of time over the specified stats collection period of `print_real_time_stats()`. The real time stats also display the CPU time consumption of other tasks created by default in ESP-IDF (e.g. `IDLE` and `ipc` tasks).