#ifndef _DEFAULT__H_
#define _DEFAULT__H_
/*
    this is Custom_bluetooth.h feil    
    

    2023.9.13   
*/
#include "Items.h"
#include "Caven_Type.h"


#ifdef Exist_UART
    #if (BOARD_NAME == ESP32_CAVEND)
        
    #elif (BOARD_NAME == EY1001)
        
    #endif
#endif

int custom_bluetooth_init (int set);    // 示例 


#endif
