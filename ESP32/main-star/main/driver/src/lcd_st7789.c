#include "lcd_st7789.h"

#include "string.h"

#include "driver/gpio.h" // because define
#include "driver/spi_master.h"

#include "lcdfont.h"	/* 字库	*/
#include "ls_lg.h"	/* 图库	*/

int LCD_PicSize = 0;
U16 LCD_W_Max = 0;
U16 LCD_H_Max = 0;
char lcd_start = 0;

U16 LCD_Back_color = LCD_BLACK; /* 背景色	LCD_BLACK LCD_WHITE*/
U8 LCD_Horizontal = 1;

#ifdef Exist_LCD

spi_device_handle_t LCD_spi = {0};

void SPI_CS_Set(char channel, int Set)
{
#if (PIN_LCD_CS != (-1))
	switch (channel)
	{
	case 1:
		// gpio_set_level(PIN_LCD_CS, Set);
		break;

	default:
		break;
	}
#endif
}

void LCD_RES_H(void) /* 这么写是为了兼容宏 */
{
	gpio_set_level(PIN_LCD_RST, 1);
}

void LCD_RES_L(void)
{
	gpio_set_level(PIN_LCD_RST, 0);
}

void LCD_DC_H(void)
{
	gpio_set_level(PIN_LCD_DC, 1);
}

void LCD_DC_L(void)
{
	gpio_set_level(PIN_LCD_DC, 0);
}

void preposition_DC_Fun(spi_transaction_t *t)
{
	int dc = (int)t->user;
	if (dc)
	{
		LCD_DC_H();
	}
	else
	{
		LCD_DC_L();
	}
}

static void LCD_Delay(int time)
{
	for (int i = 0; i < time; i++)
	{
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

void LCD_GPIO_Init(int Set)
{
	if (Set)
	{
#if (PIN_LCD_DC != (-1))
		gpio_pad_select_gpio(PIN_LCD_DC);
		gpio_set_direction(PIN_LCD_DC, GPIO_MODE_OUTPUT);
		LCD_DC_H();
#endif

#if (PIN_LCD_RST != (-1))
		gpio_pad_select_gpio(PIN_LCD_RST);
		gpio_set_direction(PIN_LCD_RST, GPIO_MODE_OUTPUT);
		LCD_RES_L();
#endif

#if (PIN_LCD_CS != (-1))
		gpio_pad_select_gpio(PIN_LCD_CS);
		gpio_set_direction(PIN_LCD_CS, GPIO_MODE_OUTPUT);
		gpio_set_level(PIN_LCD_CS, 0);
#endif
	}
	else
	{
	}
}

int SPI_Start_Init(int Set)
{
	int retval = 0;
	if (Set)
	{
		esp_err_t ret;
		spi_bus_config_t buscfg = {
			.miso_io_num = PIN_LCD_MISO,
			.mosi_io_num = PIN_LCD_MOSI,
			.sclk_io_num = PIN_LCD_CLK,
			.quadwp_io_num = -1,
			.quadhd_io_num = -1,
			.max_transfer_sz = (16 * 320 * 2 + 8)};
		spi_device_interface_config_t devcfg = {
			.clock_speed_hz = 24 * 1000 * 1000, // Clock out at 26 MHz
			.mode = 3,							// SPI mode 0-3
			.spics_io_num = -1,					// CS pin
			.queue_size = 7,					// We want to be able to queue 7 transactions at a time
			.pre_cb = preposition_DC_Fun,		// 传输开始时的回调函数
		};
		// Initialize the SPI bus
		ret = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
		ESP_ERROR_CHECK(ret);
		// Attach the LCD to the SPI bus
		ret = spi_bus_add_device(LCD_HOST, &devcfg, &LCD_spi);
		ESP_ERROR_CHECK(ret);
	}
	else
	{
	}
	return retval;
}

void EPS_SPI_SendData(const U8 *data, int num, int cmd)
{
	esp_err_t ret;
	spi_transaction_t t;
	int send_size = 0;

	if (data == 0)
	{
		return;
	}

	memset(&t, 0, sizeof(t)); // Zero out the transaction
	if (cmd)
	{
		t.user = (void *)1;
	}
	else
	{
		t.user = (void *)0;
	}
	send_size = 8 * num;
	t.length = send_size;							// Len is in bytes, transaction length is in bits.
	t.tx_buffer = data;								// Data
	ret = spi_device_polling_transmit(LCD_spi, &t); // Transmit!
	// ret = spi_device_transmit(LCD_spi, &t);			// Transmit!

	assert(ret == ESP_OK);
	if (ret == ESP_OK)
	{
		// printf("spi send run : %d \n",num);
	}
}

void LCD_Writ_Bus(U8 data)
{
	// SPI_Send_DATA(data);
	esp_err_t ret;
	spi_transaction_t t;

	memset(&t, 0, sizeof(t));						// Zero out the transaction
	t.length = 1 * 8;								// Len is in bytes, transaction length is in bits.
	t.tx_buffer = &data;							// Data
	ret = spi_device_polling_transmit(LCD_spi, &t); // Transmit!
	assert(ret == ESP_OK);							// Should have had no issues.
}

/******************************************************************************
	  函数说明：LCD写入数据
	  入口数据：dat 写入的数据
	  返回值：  无
******************************************************************************/
void LCD_WR_DATA8(U8 data)
{
	U8 spi_tx_buff = data;

	SPI_CS_Set(1, ENABLE);

	EPS_SPI_SendData(&spi_tx_buff, 1, LCD_DATA);

	SPI_CS_Set(1, DISABLE);
}

/*
 * 大端先发 [h l]
 */
void LCD_WR_DATA(U16 data)
{
	U8 spi_tx_buff[2];
	spi_tx_buff[0] = (data >> 8) & 0xff;
	spi_tx_buff[1] = data & 0xff;

	SPI_CS_Set(1, ENABLE);

	EPS_SPI_SendData(spi_tx_buff, 2, LCD_DATA);

	SPI_CS_Set(1, DISABLE);
}

/******************************************************************************
	  函数说明：LCD写入大量数据
	  入口数据：dat 写入的数据
	  返回值：  无
******************************************************************************/
void LCD_Send_Data(const U8 *data, int num)
{
	int all_size = num;
	int point_move = 0;
	int size_data = 0;
	U8 *spi_tx_buff = malloc(LCD_SPI_BUFF_MAX);

	for (int i = 0; i < all_size; i += size_data)
	{
		if ((all_size - i) > LCD_SPI_BUFF_MAX)
		{
			size_data = LCD_SPI_BUFF_MAX;
		}
		else
		{
			size_data = all_size - i;
		}
		if (size_data > 0)
		{
			memset(spi_tx_buff, 0, LCD_SPI_BUFF_MAX);
			memcpy(spi_tx_buff, data + point_move, size_data);
			EPS_SPI_SendData(spi_tx_buff, size_data, LCD_DATA);
			point_move += size_data;
			// printf("spi send run : %d , all : %d \n",size_data,all_size);
		}
	}
	free(spi_tx_buff);
}

/******************************************************************************
	  函数说明：LCD写入命令
	  入口数据：dat 写入的命令
	  返回值：  无
******************************************************************************/
void LCD_WR_CMD(U8 data)
{
	U8 spi_tx_buff = data;

	SPI_CS_Set(1, ENABLE);

	EPS_SPI_SendData(&spi_tx_buff, 1, LCD_CMD);

	SPI_CS_Set(1, DISABLE);
}

#endif
// 以上不提供到其他文件

/******************************************************************************
	  函数说明：设置起始和结束地址
	  入口数据：x1,x2 设置列的起始和结束地址
				y1,y2 设置行的起始和结束地址
	  返回值：  无
******************************************************************************/
void LCD_Address_Set(U16 x1, U16 y1, U16 x2, U16 y2)
{
	U16 x_sta, y_sta, x_end, y_end;
#if (USE_LCD_TYPE == LCD_TYPE_1_30)
	if (LCD_Horizontal == 0)
	{
		x_sta = (x1);
		x_end = (x2);
		y_sta = (y1);
		y_end = (y2);
	}
	else if (LCD_Horizontal == 1)
	{
		x_sta = (x1);
		x_end = (x2);
		y_sta = (y1 + 80);
		y_end = (y2 + 80);
	}
	else if (LCD_Horizontal == 2)
	{
		x_sta = (x1);
		x_end = (x2);
		y_sta = (y1);
		y_end = (y2);
	}
	else
	{
		x_sta = (x1 + 80);
		x_end = (x2 + 80);
		y_sta = (y1);
		y_end = (y2);
	}
#elif (USE_LCD_TYPE == LCD_TYPE_1_69)
	if (LCD_Horizontal == 1)
	{
		x_sta = (x1);
		x_end = (x2);
		y_sta = (y1 + 20);
		y_end = (y2 + 20);
	}
	else
	{
		x_sta = (x1 + 20);
		x_end = (x2 + 20);
		y_sta = (y1);
		y_end = (y2);
	}
#elif (USE_LCD_TYPE == LCD_TYPE_1_90)
	if (LCD_Horizontal == 0 || LCD_Horizontal == 1)
	{
		x_sta = (x1 + 35);
		x_end = (x2 + 35);
		y_sta = (y1);
		y_end = (y2);
	}
	else
	{
		x_sta = (x1);
		x_end = (x2);
		y_sta = (y1 + 35);
		y_end = (y2 + 35);
	}
#elif (USE_LCD_TYPE == LCD_TYPE_2_40)

#endif

	LCD_WR_CMD(0x2a); // 列地址设置
	LCD_WR_DATA(x_sta);
	LCD_WR_DATA(x_end);
	LCD_WR_CMD(0x2b); // 行地址设置
	LCD_WR_DATA(y_sta);
	LCD_WR_DATA(y_end);
	LCD_WR_CMD(0x2c); // 储存器写
}

/******************************************************************************
	  函数说明：在指定区域填充颜色
	  入口数据：xsta,ysta   起始坐标
				xend,yend   终止坐标
								color       要填充的颜色
	  返回值：  无
******************************************************************************/
void LCD_Fill(U16 x_sta, U16 y_sta, U16 x_end, U16 y_end, U16 color)
{
#ifdef Exist_LCD
	U8 *pic_buff = malloc(650); /* 一个y 320 * 2	*/
	U8 color_l;
	U8 color_h;
	U16 x_len = (x_end - x_sta);
	U16 y_len = (y_end - y_sta);
	int i = x_len * y_len;

	color_l = 0xff & color;
	color_h = (color >> 8) & 0xff;
	// memset(pic_buff,0,sizeof(pic_buff));
	LCD_Address_Set(x_sta, y_sta, x_end - 1, y_end - 1); // 设置显示范围

	for (i = 0; i < y_len; i++)
	{
		pic_buff[2 * i] = color_h;
		pic_buff[2 * i + 1] = color_l;
	}

	for (; x_len > 0; x_len--)
	{
		LCD_Send_Data(pic_buff, y_len * 2);
	}
	free(pic_buff);
#endif
}

/******************************************************************************
	  函数说明：设置屏幕显示方向
	  入口数据：0或1为竖屏 2或3为横屏
				如果是其他值，则理解为查询当前屏幕方向
	  返回值：  返回当前屏幕显示方向
******************************************************************************/
int LCD_Set_Horizontal(U8 set)
{
	int retval = 0;
#ifdef Exist_LCD
	if (set < 4)
	{
		LCD_Horizontal = set;
	}
	retval = LCD_Horizontal;
#endif
	return retval;
}

/******************************************************************************
	  函数说明：在指定位置画点
	  入口数据：x,y 画点坐标
				color 点的颜色
	  返回值：  无
******************************************************************************/
void LCD_Draw_Point(U16 x, U16 y, U16 color)
{
#ifdef Exist_LCD
	LCD_Address_Set(x, y, x, y); // 设置光标位置
	LCD_WR_DATA(color);
#endif
}

/******************************************************************************
	  函数说明：画线
	  入口数据：x1,y1   起始坐标
				x2,y2   终止坐标
				color   线的颜色
	  返回值：  无
******************************************************************************/
void LCD_Draw_Line(U16 x1, U16 y1, U16 x2, U16 y2, U16 color)
{
#ifdef Exist_LCD
	U16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // 计算坐标增量
	delta_y = y2 - y1;
	uRow = x1; // 画线起点坐标
	uCol = y1;
	if (delta_x > 0)
		incx = 1; // 设置单步方向
	else if (delta_x == 0)
		incx = 0; // 垂直线
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; // 水平线
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; // 选取基本增量坐标轴
	else
		distance = delta_y;
	for (t = 0; t < distance + 1; t++)
	{
		LCD_Draw_Point(uRow, uCol, color); // 画点
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
#endif
}

/******************************************************************************
	  函数说明：画矩形
	  入口数据：x1,y1   起始坐标
				x2,y2   终止坐标
				color   矩形的颜色
	  返回值：  无
******************************************************************************/
void LCD_Draw_Rectangle(U16 x1, U16 y1, U16 x2, U16 y2, U16 color)
{
	LCD_Draw_Line(x1, y1, x2, y1, color);
	LCD_Draw_Line(x1, y1, x1, y2, color);
	LCD_Draw_Line(x1, y2, x2, y2, color);
	LCD_Draw_Line(x2, y1, x2, y2, color);
}

/******************************************************************************
	  函数说明：画圆
	  入口数据：x0,y0   圆心坐标
				r       半径
				color   圆的颜色
	  返回值：  无
******************************************************************************/
void LCD_Draw_Circle(U16 x0, U16 y0, char r, U16 color)
{
	int a, b;
	a = 0;
	b = r;
	while (a <= b)
	{
		LCD_Draw_Point(x0 - b, y0 - a, color); // 3
		LCD_Draw_Point(x0 + b, y0 - a, color); // 0
		LCD_Draw_Point(x0 - a, y0 + b, color); // 1
		LCD_Draw_Point(x0 - a, y0 - b, color); // 2
		LCD_Draw_Point(x0 + b, y0 + a, color); // 4
		LCD_Draw_Point(x0 + a, y0 - b, color); // 5
		LCD_Draw_Point(x0 + a, y0 + b, color); // 6
		LCD_Draw_Point(x0 - b, y0 + a, color); // 7
		a++;
		if ((a * a + b * b) > (r * r)) // 判断要画的点是否过远
		{
			b--;
		}
	}
}

/******************************************************************************
	  函数说明：显示单个字符
	  入口数据：x,y显示坐标
				num 要显示的字符
				fc 字的颜色
				bc 字的背景色
				sizey 字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
void LCD_Show_Char(U16 x, U16 y, char num, U16 fc, U16 bc, char sizey, char mode)
{
#ifdef String_Lib
	U16 lcd_temp, sizex, t, m = 0;
	U16 i, TypefaceNum; // 一个字符所占字节大小
	U16 x0 = x;
	sizex = sizey / 2;
	TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
	int num_str = num - ' ';							 // 得到偏移后的值
	LCD_Address_Set(x, y, x + sizex - 1, y + sizey - 1); // 设置光标位置
	for (i = 0; i < TypefaceNum; i++)
	{
		if (sizey == 16)
		{
			lcd_temp = ascii_1608[num_str][i];
		} // 调用8x16字体
		else if (sizey == 24)
		{
			lcd_temp = ascii_2412[num_str][i];
		} // 调用12x24字体
		else if (sizey == 32)
		{
			lcd_temp = ascii_3216[num_str][i];
		} // 调用16x32字体
		else
		{
			return;
		}
		for (t = 0; t < 8; t++)
		{
			if (!mode) // 非叠加模式
			{
				if (lcd_temp & (0x01 << t))
				{
					LCD_WR_DATA(fc);
				}
				else
				{
					LCD_WR_DATA(bc);
				}
				m++;
				if (m % sizex == 0)
				{
					m = 0;
					break;
				}
			}
			else // 叠加模式
			{
				if (lcd_temp & (0x01 << t))
				{
					LCD_Draw_Point(x, y, fc);
				} // 画一个点
				x++;
				if ((x - x0) == sizex)
				{
					x = x0;
					y++;
					break;
				}
			}
		}
	}
#endif
}

/******************************************************************************
	  函数说明：显示字符串
	  入口数据：x(0-29),y(0-14)显示坐标
				*p 要显示的字符串
				fc 字的颜色 BLUE
				bc 字的背景色 BLACK
				sizey (16/24/32/)字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
void LCD_Show_String(U16 x, U16 y, const char *p, U16 fc, U16 bc, char sizey)
{
	if (sizey == 16 || sizey == 24 || sizey == 32)
	{
		x *= (sizey / 2);
		y *= (sizey);
		if (x >= LCD_W_Max)
		{
			x = (LCD_W_Max - (sizey / 2));
		}
		if (y >= LCD_H_Max)
		{
			y = (LCD_H_Max - sizey);
		}
	}
	else
		return;

	while (*p != '\0')
	{
		LCD_Show_Char(x, y, *p, fc, bc, sizey, 0);
		x += sizey / 2;
		p++;
	}
}

/******************************************************************************
	  函数说明：显示图片
	  入口数据：x,y起点坐标
				length 图片长度
				width  图片宽度
				pic[]  图片数组
	  返回值：  无
******************************************************************************/
void LCD_Show_Picture(U16 x, U16 y, U16 length, U16 width, U8 pic[])
{
#ifdef Exist_LCD
	U16 i, j;
	int temp_num = 0;
	int temp_run = 0;
	U8 *pic_buff = malloc(650);
	LCD_Address_Set(x, y, (x + length - 1), (y + width - 1));

	for (i = 0; i < length; i++)
	{
		for (j = 0; j < width; j++)
		{
			pic_buff[temp_num++] = pic[temp_run++];
			pic_buff[temp_num++] = pic[temp_run++];
		}
		LCD_Send_Data(pic_buff, width * 2);
		temp_num = 0;
	}
	free(pic_buff);
#endif
}

void LCD_Init(int Set)
{
#ifdef Exist_LCD
	U8 temp_data = 0;
	LCD_GPIO_Init(Set);
	SPI_Start_Init(Set);
	LCD_WR_DATA8(0x00);

	LCD_Delay(300); // 等待电路复位完成

	LCD_RES_L();
	LCD_Delay(200); //
	LCD_RES_H();
	LCD_Delay(100);

	//************* Start Initial Sequence **********//
	LCD_WR_CMD(0x36); // res
	switch (LCD_Horizontal)
	{
	case 0:
		LCD_WR_DATA8(0x00);
		break;
	case 1:
		LCD_WR_DATA8(0xC0);
		break;
	case 2:
		LCD_WR_DATA8(0x70);
		break;
	default:
		LCD_WR_DATA8(0xA0);
		break;
	}
//************* InitReg **********//
#if (USE_LCD_TYPE == LCD_TYPE_1_30)
	LCD_WR_CMD(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_CMD(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_CMD(0xB7); // Gate Control
	LCD_WR_DATA8(0x35);
	LCD_WR_CMD(0xBB); // VCOM Setting
	LCD_WR_DATA8(0x19);

	LCD_WR_CMD(0xC0); // LCM Control
	LCD_WR_DATA8(0x2C);

	LCD_WR_CMD(0xC2); // VDV and VRH Command Enable
	LCD_WR_DATA8(0x01);
	LCD_WR_CMD(0xC3); // VRH Set
	LCD_WR_DATA8(0x12);
	LCD_WR_CMD(0xC4); // VDV Set
	LCD_WR_DATA8(0x20);

	LCD_WR_CMD(0xC6); // Frame Rate Control in Normal Mode
	LCD_WR_DATA8(0x0F);

	LCD_WR_CMD(0xD0); // Power Control 1
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_CMD(0xE0); // Positive Voltage Gamma Control
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);

	LCD_WR_CMD(0xE1); // Negative Voltage Gamma Control
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);
	LCD_W_Max = 240;
	LCD_H_Max = 240;
#elif (USE_LCD_TYPE == LCD_TYPE_1_69)
	LCD_WR_CMD(0x3A); /* RGB 5-6-5-bit  */
	LCD_WR_DATA8(0x05);

	LCD_WR_CMD(0xB2); /* Porch Setting */
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x35);

	LCD_WR_CMD(0xB7); // Gate Control
	LCD_WR_DATA8(0x11);
	LCD_WR_CMD(0xBB); // VCOM Setting
	LCD_WR_DATA8(0x35);

	LCD_WR_CMD(0xC0); // LCM Control
	LCD_WR_DATA8(0x2C);

	LCD_WR_CMD(0xC2); // VDV and VRH Command Enable
	LCD_WR_DATA8(0x01);
	LCD_WR_CMD(0xC3); // VRH Set
	LCD_WR_DATA8(0x0D);
	LCD_WR_CMD(0xC4); // VDV Set
	LCD_WR_DATA8(0x20);

	LCD_WR_CMD(0xC6); // Frame Rate Control in Normal Mode
	LCD_WR_DATA8(0x13);

	LCD_WR_CMD(0xD0); // Power Control 1
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_CMD(0xD6);
	LCD_WR_DATA8(0xA1);

	LCD_WR_CMD(0xE0); // Positive Voltage Gamma Control
	LCD_WR_DATA8(0xF0);
	LCD_WR_DATA8(0x06);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x0A);
	LCD_WR_DATA8(0x09);
	LCD_WR_DATA8(0x26);
	LCD_WR_DATA8(0x29);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x41);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x16);
	LCD_WR_DATA8(0x15);
	LCD_WR_DATA8(0x29);
	LCD_WR_DATA8(0x2D);

	LCD_WR_CMD(0xE1); // Negative Voltage Gamma Control
	LCD_WR_DATA8(0xF0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x07);
	LCD_WR_DATA8(0x03);
	LCD_WR_DATA8(0x28);
	LCD_WR_DATA8(0x32);
	LCD_WR_DATA8(0x40);
	LCD_WR_DATA8(0x3B);
	LCD_WR_DATA8(0x19);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x2A);
	LCD_WR_DATA8(0x2E);

	LCD_WR_CMD(0xE4);
	LCD_WR_DATA8(0x25);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_W_Max = 240;
	LCD_H_Max = 280;
#elif (USE_LCD_TYPE == LCD_TYPE_1_90)
	LCD_WR_CMD(0x3A); /* RGB 5-6-5-bit  */
	LCD_WR_DATA8(0x05);

	LCD_WR_CMD(0xB2); /* Porch Setting */
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x35);

	LCD_WR_CMD(0xB7); // Gate Control
	LCD_WR_DATA8(0x35);
	LCD_WR_CMD(0xBB); // VCOM Setting
	LCD_WR_DATA8(0x1A);

	LCD_WR_CMD(0xC0); // LCM Control
	LCD_WR_DATA8(0x2C);
	LCD_WR_CMD(0xC2); // VDV and VRH Command Enable
	LCD_WR_DATA8(0x01);
	LCD_WR_CMD(0xC3); // VRH Set
	LCD_WR_DATA8(0x0B);
	LCD_WR_CMD(0xC4); // VDV Set
	LCD_WR_DATA8(0x20);
	LCD_WR_CMD(0xC6); // Frame Rate Control in Normal Mode
	LCD_WR_DATA8(0x0F);

	LCD_WR_CMD(0xD0); // Power Control 1
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_CMD(0x21);
	LCD_WR_CMD(0xE0); // Positive Voltage Gamma Control
	LCD_WR_DATA8(0xF0);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x29);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x3E);
	LCD_WR_DATA8(0x38);
	LCD_WR_DATA8(0x12);
	LCD_WR_DATA8(0x12);
	LCD_WR_DATA8(0x28);
	LCD_WR_DATA8(0x30);

	LCD_WR_CMD(0xE1); // Negative Voltage Gamma Control
	LCD_WR_DATA8(0xF0);
	LCD_WR_DATA8(0x07);
	LCD_WR_DATA8(0x0A);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x07);
	LCD_WR_DATA8(0x28);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x3E);
	LCD_WR_DATA8(0x36);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x29);
	LCD_WR_DATA8(0x32);
	LCD_W_Max = 170;
	LCD_H_Max = 320;
#elif (USE_LCD_TYPE == LCD_TYPE_2_40)
	LCD_W_Max = 320;
	LCD_H_Max = 240;
#endif

	LCD_WR_CMD(0x21); // Display Inversion On
	LCD_WR_CMD(0x11); // Sleep Out
	LCD_WR_CMD(0x29); // Display On
	LCD_Delay(100);
	LCD_Fill(0, 0, LCD_W_Max, LCD_H_Max, LCD_Back_color);
	LCD_PicSize = LCD_W_Max * LCD_H_Max * 2;
#endif
}

#include "ui.h"
#define BUFF_LEN 5
static lv_disp_draw_buf_t draw_buf_dsc_1;
static lv_color_t buf_1[MY_DISP_HOR_RES * BUFF_LEN];
static lv_disp_drv_t disp_drv;
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
	uint32_t w = (area->x2 - area->x1 + 1);
	uint32_t h = (area->y2 - area->y1 + 1);
	// M5.Lcd.drawBitmap(area->x1, area->y1, w, h, &color_p->full);
	// lcd_show_image(area->x1, area->y1, w, h, &color_p->full);
	// Mode_Use.LCD.Show_Picture_pFun(area->x1, area->y1, w, h, (uint8_t*)&color_p->full); // Photo
	LCD_Show_Picture(area->x1, area->y1, w, h, (uint8_t *)&color_p->full);
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
void gui_init(void)
{
	/*Initialize LVGL*/
	lv_init();
	/*Initialize the HAL (display, input devices, tick) for LVGL*/
	hal_init();
	lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * BUFF_LEN);
	lv_disp_drv_init(&disp_drv);
	disp_drv.hor_res = MY_DISP_HOR_RES;
	disp_drv.ver_res = MY_DISP_VER_RES;
	disp_drv.flush_cb = disp_flush;
	disp_drv.draw_buf = &draw_buf_dsc_1;
	lv_disp_drv_register(&disp_drv);
	ui_init();
}


void refresh_lcd_task(void *pvParam)
{
	static int refresh_ready = 0;
	int colour = 0;
	SemaphoreHandle_t XSema_LCDpt = NULL;
	uint8_t *array_buff;
	array_buff = malloc(300);
	memset(array_buff, 0, 100);
	memcpy(array_buff, "this is num :    ", strlen("this is num :    "));

	for (int i = 0; i < 300; i++)
	{
		array_buff[i] = (i & 0xff);
	}
	XSema_LCDpt = xSemaphoreCreateCounting(6,1);
	LCD_Set_Horizontal(0);
	LCD_Init(TURE);
	
	LCD_Show_Picture(0, 0, 240, 240, gImage_ls);
	ESP_LOGI("[LCD]", "init TYPE %d", USE_LCD_TYPE);

	LCD_Delay(1000);
	while (1)
	{
		LCD_Delay(1000);
	};
	
	gui_init();
	lv_label_set_text(ui_NUM, "0000");
	lv_label_set_text(ui_NUMCT, "    ");
	lv_slider_set_value(ui_NUMC, 0, LV_ANIM_OFF);
	while (1)
	{
		// LCD_Fill(0, 0, LCD_W, LCD_H, LCD_RED);
		// LCD_Fill(0, 0, LCD_W, LCD_H, LCD_GREEN);
		// LCD_Fill(0, 0, LCD_W, LCD_H, LCD_BLUE);
		// if (xSemaphoreTake(XSema_LCDpt,portMAX_DELAY) == pdTRUE)
		// {
		// 	ESP_LOGI("[LCD]","run again");
		// 	colour = 0xffff;
		// 	// LCD_Fill(0, 0, LCD_W, LCD_H, colour);
		// 	colour += 500;
		// 	if (colour > 0x0fff)
		// 	{
		// 		colour = 0;
		// 	}
		// 	LCD_Delay(100);
		// }
		lv_timer_handler();
		LCD_Delay(10);
		
	}
	free(array_buff);
}
