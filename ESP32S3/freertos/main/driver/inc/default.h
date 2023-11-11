#ifndef _DEFAULT__H_
#define _DEFAULT__H_

/*
    this is default.h feil    
    这是个模板文件

    2023.9.13   
*/

#include "Precompiling_define.h"
#include"sys_typedef.h"



#if (BOARD_NAME == ESP32_Cavend)
    #include"Items.h"
    
#elif (BOARD_NAME == EY1001)
    #include"Items.h"
    
#endif


int default_init (int set);    // 示例 


#endif
