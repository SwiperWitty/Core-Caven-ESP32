#ifndef _INFORMATION__H_
#define _INFORMATION__H_

/*
    this is information.h feil    
    这是个信息文件

    2023.9.14  
*/

#include "Precompiling_define.h"
#include"sys_typedef.h"



#if (BOARD_NAME == ESP32_Cavend)
    #include"Items.h"
    
#elif (BOARD_NAME == EY1001)
    #include"Items.h"
    
#endif


int information_init (void);    // 信息 


#endif
