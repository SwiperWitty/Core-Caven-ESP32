#ifndef __BASE_GPIO__H_
#define __BASE_GPIO__H_

#include "Items.h"          // 默认功能


/*
                GPIO_Init(Exist_GPIO)->
    SDK->Items->                       BASE->
                Peripheral(UART、ADC)->      \\
                                              -->[XXX]->MODE
                                             //
                        C(Lib)->Caven->API->
*/


#define READ_Config		0
#define WRITE_Config	1


/*  Init-Function    */
void User_GPIO_config(int gpiox,int pin,int set);
void User_GPIO_set(int gpiox,int pin,int set);
char User_GPIO_get(int gpiox,int pin);

// end 
#endif
