#ifndef _USER_MQTT__H_
#define _USER_MQTT__H_

#ifdef CAVEN    /* 预编译 */
#include "Caven_Type.h"
#else
#include "stdint.h"
#include "string.h"
#include "stddef.h"
#include "stdio.h"
#endif

void TCP_MQTT_task(void);
void TCP_MQTT_send (uint8_t *data,int len);

#endif
