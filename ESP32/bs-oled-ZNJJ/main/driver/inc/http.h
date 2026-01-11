#ifndef _HTTP_H_
#define _HTTP_H_

#include "stdint.h"
#include "Network_manage.h"

/*
    http_client_config_init 初始化配置
    http_cache_port_task 任务检测是否有数据要发送

    http_cache_port_data_Fun 发送缓存
    http_receive_Callback_Bind 接收数据函数绑定，收到的数据是ASCII，支架strlen访问长度

    http_cache_port_State_machine 寄生在别的任务里面使用，发送方式和http_cache_port_data_Fun一致
*/

int http_client_config_init (char *url_str,char *port_str,int enable);

void http_receive_Callback_Bind (D_Callback_pFun Callback_pFun);
void http_cache_port_data_Fun (char *data);

void http_cache_port_State_machine (void *empty);
void http_cache_port_task (void *empty);

#endif
