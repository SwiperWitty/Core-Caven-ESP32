#ifndef _MESSAGE_INFO_APP__H_
#define _MESSAGE_INFO_APP__H_

#include"system_app.h"
#include"Caven_Type.h"

#include "caven_at_info_frame.h"

typedef struct
{
    char flag;
    int len_a;
    int len_b;
    void *p_buff_a;
    void *p_buff_b;
    int time;       //ms
}standby_Date_Type;

void Message_info_task (void * empty);

#endif
