#include "Base_Flash.h"
#include "string.h"

static int Flash_Demarcation = 0;

int Base_addr_check (int addr,int len)
{
	int retval = 0;
#if Exist_FLASH 

#endif
	return retval;
}

/*
    符合地址大小 ≥ 0,且返回扇区
    不符合 (-1)
*/
int Base_Addr_Get_Area(int addr)
{
    int retval = 0;
#if Exist_FLASH 

#endif
    return retval;
}

int Base_Flash_Demarcation (int addr)
{
    int retval = 0;
#if Exist_FLASH 

#endif
    return retval;
}

/*
retval = 0,COMPLETE
retval = x,error
*/
int Base_Flash_Erase (int addr,int len)
{
	int retval = 0;
#if Exist_FLASH 

#endif

	return retval;
}

/*
retval = 0,COMPLETE
retval = x,error
*/
int Base_Flash_Read (void *data,int addr,int len)
{
    int retval = 0;
#if Exist_FLASH 

#endif
    return retval;
}

/*
retval = 0,COMPLETE
retval = x,error
*/
int Base_Flash_Write (void *data,int addr,int len)
{
	int retval = 0;
#if Exist_FLASH 

#endif

	return retval;
}
