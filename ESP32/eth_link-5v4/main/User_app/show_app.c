#include "show_app.h"
#include "ui.h"
#ifdef Exist_OLED
    #include "MODE_OLED.h"
#else
    #include "MODE_LCD.h"
#endif 

//
#define BUFF_LEN 5

static lv_disp_draw_buf_t draw_buf_dsc_1;
static lv_color_t buf_1[MY_DISP_HOR_RES * BUFF_LEN];
static lv_disp_drv_t disp_drv;
static int s_gui_flag = 0;

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
	uint32_t w = (area->x2 - area->x1 + 1);
	uint32_t h = (area->y2 - area->y1 + 1);
	// M5.Lcd.drawBitmap(area->x1, area->y1, w, h, &color_p->full);
	// lcd_show_image(area->x1, area->y1, w, h, &color_p->full);
    LCD_Show_Picture(area->x1, area->y1, w, h, (uint8_t*)&color_p->full); // !!!!
	lv_disp_flush_ready(disp_drv);
}

uint32_t custom_millis(void)
{
	uint32_t retval;
	retval = xTaskGetTickCount();
	return retval;
}

/**
 * Initialize the Hardware Abstraction Layer (HAL) for LVGL
 */
static void hal_init(void)
{

}

void gui_init (int hor,int ver)
{
	/*Initialize LVGL*/
	lv_init();
	/*Initialize the HAL (display, input devices, tick) for LVGL*/
	hal_init();
	lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, hor * BUFF_LEN);
	lv_disp_drv_init(&disp_drv);
	disp_drv.hor_res = hor;
	disp_drv.ver_res = ver;
	disp_drv.flush_cb = disp_flush;
	disp_drv.draw_buf = &draw_buf_dsc_1;
	lv_disp_drv_register(&disp_drv);
	ui_init();
}
// lv_timer_handler();
//

char net_str[30];
void show_set_net_information (char *str)
{
    if (str != NULL && strlen(str) && s_gui_flag)
    {
        if (strcmp(net_str,str) != 0)
        {
            strcpy(net_str,str);
            lv_label_set_text(ui_Label2, net_str);
        }
    }
}

char txt_str[100];
void show_txt_str_information (char *str)
{
    int temp_num = 0;
    temp_num = strlen(str);
    if (str != NULL && temp_num && s_gui_flag)
    {
        if (strcmp(txt_str,str) != 0 && temp_num < 50)
        {
            strcpy(txt_str,str);
            lv_textarea_set_text(ui_TextArea2, txt_str);
        }
    }
}

#include "hongshu.h"	// 图库
void show_app_task (void * empty)
{
    char type = m_LCD_TYPE_1_69;    // m_LCD_TYPE_1_28 m_LCD_TYPE_1_69
    LCD_Set_TargetModel(type);
    if (type == m_LCD_TYPE_1_69)
    {
        LCD_Set_Horizontal(1);
    }
    else
    {
        LCD_Set_Horizontal(0);
    }
    MODE_LCD_Init(1);
    LCD_Show_Picture(0, 0, 240, 240, gImage_hongshu);
    ESP_LOGI("LCD Init","Model[%d],x:%d y:%d ",type,LCD_W_Max,LCD_H_Max);
    //
    gui_init (LCD_W_Max,LCD_H_Max);
    if (type == m_LCD_TYPE_1_69)
    {
        s_gui_flag = 1;
    }
    
    while(1)
    {
        if (s_gui_flag)
        {
            lv_timer_handler();
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
