#ifndef _CAVEN_APP_H_
#define _CAVEN_APP_H_

#include "system_app.h"

#include "Caven_info_frame.h"
#include "caven_at_info_frame.h"

int Caven_app_State_machine(Caven_BaseTIME_Type time);
int Caven_app_Make_pack (uint8_t data,int way,Caven_BaseTIME_Type time);
int Caven_app_JSON_Make_pack (char *data,int way);

int Caven_app_BOOT_check (void);
int Caven_app_send_heart_packet(int num,int way);

void Caven_app_Init (void);
void Caven_app_Exit (void);

#endif 
