#ifndef BASE_FLASH_H__
#define BASE_FLASH_H__

#include "Items.h"          // 默认功能

#if Exist_FLASH 


#endif

int Base_Flash_Erase (int addr,int len);
int Base_Flash_Read (void *data,int addr,int len);
int Base_Flash_Write (void *data,int addr,int len);
int Base_Flash_Demarcation (int addr);
#endif
