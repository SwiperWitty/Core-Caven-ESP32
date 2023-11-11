#ifndef _DRAW__H_
#define _DRAW__H_

/*
    this is draw.h feil    
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

#define ROUND_OFF(X)    (int)((X) + 0.5)        // 约等于

char draw_coordinate_line_handle(int x0,int y0,int x1,int y1);
void draw_coordinate_show (int x_len,int y_len);

#endif
