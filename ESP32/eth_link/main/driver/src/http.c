#include "http.h"

#include <esp_system.h>
#include <esp_event.h>

#include <nvs_flash.h>
#include <esp_netif.h>

#include "esp_http_client.h"

#include "cJSON.h"

static char *TAG = "http_client";

static int http_enable = 0;

static char response_data[1024*4];  // 自定义缓存空间储存一次响应数据
static int recived_len = 0;         // 自定义变量储存一次响应中接收到分片数据的累计偏移

static char http_url_str[100];
static char http_port_str[20];
static uint32_t http_port_value = 9090;

static char http_post_buff[1024*4];
static int http_post_len = 0;

// http客户端的事件处理回调函数
static esp_err_t http_client_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "connected to web-server");
        recived_len = 0;
        break;
    case HTTP_EVENT_ON_DATA:
        if (evt->user_data)
        {
            memcpy(evt->user_data + recived_len, evt->data, evt->data_len); // 将分片的每一片数据都复制到user_data
            recived_len += evt->data_len;   // 
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "finished a request and response!");
        recived_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "disconnected to web-server");
        recived_len = 0;
        break;
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG, "error");
        recived_len = 0;
        break;
    default:
        break;
    }

    return ESP_OK;
}

int http_client_config_init (char *url_str,char *port_str,int enable)
{
    int retval = 0;
    int temp_num = 0;
    int temp_run = 0;
    char temp_str[300];
    http_enable = enable;

    if (url_str == NULL)
    {
        retval = 1;
        return retval;
    }
    memset(temp_str,0,sizeof(temp_str));

    temp_num = strlen(url_str);
    if (temp_num)
    {
        memset(http_url_str,0,sizeof(http_url_str));
        memcpy(http_url_str,url_str,temp_num);
        memcpy(&temp_str[temp_run],http_url_str,temp_num);
        temp_run += temp_num;
    }
    else
    {
        retval = 1;
        return retval;
    }
    if (port_str != NULL)
    {
        temp_num = strlen(port_str);
        if (temp_num && temp_num < sizeof(http_port_str))
        {
            memset(http_port_str,0,sizeof(http_port_str));
            memcpy(http_port_str,port_str,temp_num);
            ESP_LOGI(TAG, "port:[%s]",port_str);
        }
    }
    else
    {
        memset(http_port_str,0,sizeof(http_port_str));
        strcpy(http_port_str,"9090");
        temp_num = strlen(http_port_str);
        ESP_LOGI(TAG, "default port:[%s]",port_str);
    }
    http_port_value = atoi(http_port_str);
    if (retval == 0)
    {
        
    }
    return retval;
}

/*
    接收回调函数执行指针
*/
static D_Callback_pFun http_Callback_Fun = NULL;
/*
    接收回调函数绑定
*/
void http_receive_Callback_Bind (D_Callback_pFun Callback_pFun)
{
    http_Callback_Fun = Callback_pFun;
}

/*
    收发本体都在其中
*/
int http_port_data_Fun (char *data)
{
    esp_err_t ret;
    int temp_num = 0;
    static esp_http_client_handle_t httpclient;

    if (http_url_str == NULL || data == NULL)
    {
        return (-1);
    }
    do
    {
        temp_num = Network_manage_get_status();
        if (temp_num == 0)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    } while (temp_num == 0);        // 等待网络连接
    if (temp_num)
    {
        ESP_LOGW(TAG, "get network ID [%d]",temp_num);
    }
    ESP_LOGI(TAG, "url:[%s]",http_url_str);
    esp_http_client_config_t cfg = {
        .url = http_url_str,
        .port = http_port_value,
        .event_handler = http_client_event_handler,
        .user_data = response_data,
    };
    // 使用http服务器配置参数对http客户端初始化
    httpclient = esp_http_client_init(&cfg);
/*
    // GET方法请求
    esp_http_client_set_method(httpclient, HTTP_METHOD_GET);
    esp_http_client_set_url(httpclient, "/add?x=10&y=11");//url传递发送数据x=10，y=11
    ret = esp_http_client_perform(httpclient);//启动并阻塞等待一次响应完成
    if (ret == ESP_OK)
    {
		//一次响应完成，打印完整响应数据，即得到X+y的值
        printf("GET:%s\n", response_data);
    }
    // POST方法请求
    esp_http_client_set_method(httpclient, HTTP_METHOD_POST);
    esp_http_client_set_url(httpclient, "/add/");
*/
    esp_http_client_set_method(httpclient, HTTP_METHOD_POST);
    esp_http_client_set_url(httpclient, http_url_str);
    // 设置请求头
    esp_http_client_set_header(httpclient, "Content-Type", "application/json");
    esp_http_client_set_header(httpclient, "User-Agent", "=ESP32 HTTP Client/1.0");
    ret = esp_http_client_set_post_field(httpclient, data, strlen(data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set_post %s", esp_err_to_name(ret));
    }

    ret = esp_http_client_perform(httpclient);
    if (ret == ESP_OK)  // 响应完成
    {
        temp_num = strlen(response_data);
        ESP_LOGI(TAG,"POST: len[%d]byte \n%s\n",temp_num,response_data);
        if (http_Callback_Fun != NULL)
        {
            http_Callback_Fun (response_data);
        }
    }
    if (httpclient != NULL)
    {
        esp_http_client_close(httpclient);
        esp_http_client_cleanup(httpclient); // 清空http客户端描述符
        httpclient = NULL;
    }
    return (ret);
}

/*
    一般使用这个，配合[http_cache_port_State_machine]使用
*/
void http_cache_port_data_Fun (char *data)
{
    if (data != NULL && http_enable)
    {
        int temp_num = strlen(data);
        if ((temp_num + http_post_len) < sizeof(http_post_buff))
        {
            memcpy(&http_post_buff[http_post_len],data,temp_num);
            http_post_len += temp_num;
        }
    }
}

void http_cache_clean (void)
{
    memset(http_post_buff,0,sizeof(http_post_buff));
    http_post_len = 0;
}

void http_cache_port_State_machine (void *empty)
{
    if (http_post_len && http_enable)
    {
        http_port_data_Fun (http_post_buff);
        http_cache_clean();
    }
}
