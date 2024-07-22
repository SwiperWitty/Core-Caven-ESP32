#ifndef _DATA_FILESYSTEM__H_
#define _DATA_FILESYSTEM__H_

#include <stdlib.h>
#include "reader_typedef.h"

void filesystem_init (void);
size_t filesystem_get_space_size (int *addr);
int filesystem_read_data (int addr,void *data,int size);
int filesystem_write_data (int addr,void *data,int size);


int filesystem_read_mode (int set);
int filesystem_write_mode (int set);

#endif // 
