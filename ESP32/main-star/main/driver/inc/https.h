#ifndef _HTTPS_H_
#define _HTTPS_H_

#include "stdint.h"
#include "string.h"

/*

*/

void eps32_HTTPS_task (void *empty);
int https_request_config_init (char *way_str,char *Host_str,char *URL_str,char *port_str);
int https_request_add_header (char *type,char *data);
int https_request_Fun (char type,char *data_str);


#endif
