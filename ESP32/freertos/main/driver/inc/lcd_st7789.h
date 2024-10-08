#ifndef __LCD_H_
#define __LCD_H_
/*
    ESP32用的lcd驱动，使用硬件spi。
    Logs:
    -调通eps32驱动st7789v的1.3寸 240 * 240屏幕   --2023.10.29
    还需要完成：
    -高速刷新
    -自动完成D/C命令
    -等
*/

#include "Items.h"
#include "sys_typedef.h"
#include "Caven_Type.h"

#include "driver/gpio.h"        // because define 
#include "driver/spi_master.h"

/*  LCD_TYPE    */
#define LCD_TYPE_1_14   0
#define LCD_TYPE_1_30   1
#define LCD_TYPE_1_69   2
#define LCD_TYPE_2_40   3


#ifdef Exist_LCD 
    #define PIN_LCD_MOSI    (13) 
    #define PIN_LCD_MISO    (12)
    #define PIN_LCD_CLK     (14)
    #define PIN_LCD_CS      (-1)        /* -1 是不使用	*/

    #define PIN_LCD_DC      (33)
    #define PIN_LCD_RST     (32)
    #define PIN_LCD_BCKL    NULL        /* 直接接到vdd */

    #ifdef CONFIG_IDF_TARGET_ESP32
        #define LCD_HOST    SPI2_HOST 
    #else
        #define LCD_HOST    SPI3_HOST 
    #endif
#endif

#define LCD_SPI_BUFF_MAX 500

/*  LCD config  */
#define USE_HORIZONTAL  1   // 设置默认横屏或者竖屏显示 0或1为竖屏 2或3为横屏
#define USE_LCD_TYPE    LCD_TYPE_1_69   // 设置
#if (USE_LCD_TYPE == LCD_TYPE_1_30)
    #define LCD_W 240        
    #define LCD_H 240    
    #define LCD_PICSIZE 115200    
#elif (USE_LCD_TYPE == LCD_TYPE_1_14)
    #define LCD_W 135
    #define LCD_H 240
    #define LCD_PICSIZE 64800
#elif (USE_LCD_TYPE == LCD_TYPE_1_69)
    #define LCD_W 240
    #define LCD_H 280
    #define LCD_PICSIZE 134400
#elif (USE_LCD_TYPE == LCD_TYPE_2_40)
    #define LCD_W 320
    #define LCD_H 240
    #define LCD_PICSIZE 153600
#endif //



#define LCD_CMD     0   // 写命令
#define LCD_DATA    1   // 写数据

typedef struct
{
    char flag;
    char refresh;
    char direction;
    uint8_t str_place[2];
    uint8_t *string;
    uint8_t pic_place[4];
    // uint8_t pic[LCD_PICSIZE];
} LCD_data_Type;

// 画笔颜色
#define LCD_WHITE       0xFFFF
#define LCD_BLACK       0x0000
#define LCD_BLUE        0x001F
#define LCD_BRED        0XF81F
#define LCD_GRED        0XFFE0
#define LCD_GBLUE       0X07FF
#define LCD_RED         0xF800
#define LCD_MAGENTA     0xF81F
#define LCD_GREEN       0x07E0
#define LCD_CYAN        0x7FFF
#define LCD_YELLOW      0xFFE0
#define LCD_BROWN       0XBC40  // 棕色
#define LCD_BRRED       0XFC07  // 棕红色
#define LCD_GRAY        0X8430  // 灰色
#define LCD_DARKBLUE    0X01CF  // 深蓝色
#define LCD_LIGHTBLUE   0X7D7C  // 浅蓝色
#define LCD_GRAYBLUE    0X5458  // 灰蓝色
#define LCD_LIGHTGREEN  0X841F  // 浅绿色
#define LCD_LGRAY       0XC618  // 浅灰色(PANNEL),窗体背景色
#define LCD_LGRAYBLUE   0XA651  // 浅灰蓝色(中间层颜色)
#define LCD_LBBLUE      0X2B12  // 浅棕蓝色(选择条目的反色)

struct LCD_
{
    void (*Fill)(U16 x_sta, U16 y_sta, U16 x_end, U16 y_end, U16 color); // 指定区域填充颜色

    void (*Draw_Point)(U16 x, U16 y, U16 color);                       // 在指定位置画一个点
    void (*Draw_Line)(U16 x1, U16 y1, U16 x2, U16 y2, U16 color);      // 在指定位置画一条线
    void (*Draw_Circle)(U16 x0, U16 y0, char r, U16 color);            // 在指定位置画一个圆
    void (*Draw_Rectangle)(U16 x1, U16 y1, U16 x2, U16 y2, U16 color); // 在指定位置画一个矩形

    void (*Show_String)(U16 x, U16 y, const char *p, U16 coloer, U16 b_coloer, char Size);       // 显示字符串
    void (*Show_Chinese)(U16 x, U16 y, char *s, U16 coloer, U16 b_coloer, char Size, char mode); // 显示汉字串
    void (*Show_Picture)(U16 x, U16 y, U16 length, U16 width, const unsigned char pic[]);        // 显示图片

    int (*Set_HORIZONTAL)(char set);
};

void LCD_Fill(U16 x_sta, U16 y_sta, U16 x_end, U16 y_end, U16 color); // 指定区域填充颜色
void LCD_Draw_Point(U16 x, U16 y, U16 color);                         // 在指定位置画一个点
void LCD_Draw_Line(U16 x1, U16 y1, U16 x2, U16 y2, U16 color);        // 在指定位置画一条线
void LCD_Draw_Circle(U16 x0, U16 y0, char r, U16 color);              // 在指定位置画一个圆
void LCD_Draw_Rectangle(U16 x1, U16 y1, U16 x2, U16 y2, U16 color);   // 在指定位置画一个矩形

void LCD_Show_String(U16 x, U16 y, const char *p, U16 coloer, U16 b_coloer, char Size);       // 显示字符串
void LCD_Show_Chinese(U16 x, U16 y, char *s, U16 coloer, U16 b_coloer, char Size, char mode); // 显示汉字串
void LCD_Show_Picture(U16 x, U16 y, U16 length, U16 width, const unsigned char pic[]);        // 显示图片

int LCD_Set_Horizontal(uint8_t set);

void LCD_Init(int SET); // LCD初始化

void refresh_lcd_task(void *pvParam);

#endif
