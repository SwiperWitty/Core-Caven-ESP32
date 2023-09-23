#ifndef _MAIN__H_
#define _MAIN__H_


/*
    this is main.h feil    

    2023.9.12   
*/

#include"Precompiling_define.h"

#include "nvs_flash.h"
#include "driver/gpio.h"

#include"information.h"

#if (Board_Name == ESP32_Cavend)
    #include"Items.h"
#elif (Board_Name == EY1001)
    #include"Items.h"
#endif


#ifdef Exist_RTC_Clock
    #include"rtc_time.h"
#endif 
#ifdef Exist_GPIO
    #include"Custom_gpio.h"
#endif 

#include"draw.h"



#endif // !_MAIN__H_
