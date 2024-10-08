#include "lcd_st7789.h"
#include "lcdfont.h" /* 字库	*/

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "cat_lcd_1_14.h" 	//
#include "yi_lcd_1_14.h" 	//

U16 BACK_COLOR = LCD_WHITE; /* 背景色	LCD_BLACK */
uint8_t LCD_HORIZONTAL = USE_HORIZONTAL;
spi_device_handle_t LCD_spi = {0};
#ifdef Exist_LCD

void SPI_CS_Set(char channel, int Set)
{
#if (PIN_LCD_CS != 0)
	switch (channel)
	{
	case 1:
		// gpio_set_level(PIN_LCD_CS, Set);
		break;

	default:
		break;
	}
}
#endif

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
#if (PIN_LCD_DC != 0)
		gpio_pad_select_gpio(PIN_LCD_DC);
		gpio_set_direction(PIN_LCD_DC, GPIO_MODE_OUTPUT);
		LCD_DC_H();
#endif

#if (PIN_LCD_RST != 0)
		gpio_pad_select_gpio(PIN_LCD_RST);
		gpio_set_direction(PIN_LCD_RST, GPIO_MODE_OUTPUT);
		LCD_RES_L();
#endif

#if (PIN_LCD_CS != 0)
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

void EPS_SPI_SendData(const uint8_t *data, int num, int cmd)
{
	esp_err_t ret;
	spi_transaction_t t;
	int send_size = 8 * num;

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

void LCD_Writ_String(const void *DATA, int num)
{
	// SPI_Send_String(DATA, num);
}

/******************************************************************************
	  函数说明：LCD写入数据
	  入口数据：dat 写入的数据
	  返回值：  无
******************************************************************************/
void LCD_WR_DATA8(U8 data)
{
	uint8_t spi_tx_buff = data;

	SPI_CS_Set(1, ENABLE);

	EPS_SPI_SendData(&spi_tx_buff, 1, LCD_DATA);

	SPI_CS_Set(1, DISABLE);
}

/*
 *大端先发 [h l]
 */
void LCD_WR_DATA(U16 data)
{
	uint8_t spi_tx_buff[2];
	spi_tx_buff[0] = (data >> 8) & 0xff;
	spi_tx_buff[1] = data & 0xff;

	SPI_CS_Set(1, ENABLE);

	EPS_SPI_SendData(spi_tx_buff, 2, LCD_DATA);

	SPI_CS_Set(1, DISABLE);

	// SPI_CS_Set(1, ENABLE);
	// LCD_Writ_Bus(data >> 8);
	// LCD_Writ_Bus(data & 0X00FF);
	// SPI_CS_Set(1, DISABLE);
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
	uint8_t *spi_tx_buff = malloc(LCD_SPI_BUFF_MAX);

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

	// SPI_CS_Set(1, ENABLE);
	// for (size_t i = 0; i < num; i++)
	// {
	// 	LCD_Writ_Bus(*(data + i));
	// }
	// SPI_CS_Set(1, DISABLE);
}

/******************************************************************************
	  函数说明：LCD写入命令
	  入口数据：dat 写入的命令
	  返回值：  无
******************************************************************************/
void LCD_WR_CMD(U8 data)
{
	uint8_t spi_tx_buff = data;

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
	if (LCD_HORIZONTAL == 0)
	{
		LCD_WR_CMD(0x2a); // 列地址设置
		LCD_WR_DATA(x1 + 52);
		LCD_WR_DATA(x2 + 52);
		LCD_WR_CMD(0x2b); // 行地址设置
		LCD_WR_DATA(y1 + 40);
		LCD_WR_DATA(y2 + 40);
		LCD_WR_CMD(0x2c); // 储存器写
	}
	else if (LCD_HORIZONTAL == 1)
	{
		LCD_WR_CMD(0x2a); // 列地址设置
		LCD_WR_DATA(x1 + 53);
		LCD_WR_DATA(x2 + 53);
		LCD_WR_CMD(0x2b); // 行地址设置
		LCD_WR_DATA(y1 + 40);
		LCD_WR_DATA(y2 + 40);
		LCD_WR_CMD(0x2c); // 储存器写
	}
	else if (LCD_HORIZONTAL == 2)
	{
		LCD_WR_CMD(0x2a); // 列地址设置
		LCD_WR_DATA(x1 + 40);
		LCD_WR_DATA(x2 + 40);
		LCD_WR_CMD(0x2b); // 行地址设置
		LCD_WR_DATA(y1 + 53);
		LCD_WR_DATA(y2 + 53);
		LCD_WR_CMD(0x2c); // 储存器写
	}
	else
	{
		LCD_WR_CMD(0x2a); // 列地址设置
		LCD_WR_DATA(x1 + 40);
		LCD_WR_DATA(x2 + 40);
		LCD_WR_CMD(0x2b); // 行地址设置
		LCD_WR_DATA(y1 + 52);
		LCD_WR_DATA(y2 + 52);
		LCD_WR_CMD(0x2c); // 储存器写
	}
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
	uint8_t *pic_buff = malloc(650); /* 一个y 320 * 2	*/
	uint8_t color_l;
	uint8_t color_h;
	u16 x_len = (x_end - x_sta);
	u16 y_len = (y_end - y_sta);
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
int LCD_Set_Horizontal(uint8_t set)
{
	int retval = 0;
#ifdef Exist_LCD
	if (set < 4)
	{
		LCD_HORIZONTAL = set;
	}
	retval = LCD_HORIZONTAL;
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
	  函数说明：显示单个24x24汉字
	  入口数据：x,y显示坐标
				*s 要显示的汉字
				fc 字的颜色
				bc 字的背景色
				sizey 字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
void LCD_Show_Chinese24x24(U16 x, U16 y, char *s, U16 fc, U16 bc, char sizey, char mode)
{
#ifdef String_Chinese
	char i, j, m = 0;
	U16 k;
	U16 HZnum;		 // 汉字数目
	U16 TypefaceNum; // 一个字符所占字节大小
	U16 x0 = x;
	TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;
	HZnum = sizeof(tfont24) / sizeof(typFNT_GB24); // 统计汉字数目
	for (k = 0; k < HZnum; k++)
	{
		if ((tfont24[k].Index[0] == *(s)) && (tfont24[k].Index[1] == *(s + 1)))
		{
			LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);
			for (i = 0; i < TypefaceNum; i++)
			{
				for (j = 0; j < 8; j++)
				{
					if (!mode) // 非叠加方式
					{
						if (tfont24[k].Msk[i] & (0x01 << j))
							LCD_WR_DATA(fc);
						else
							LCD_WR_DATA(bc);
						m++;
						if (m % sizey == 0)
						{
							m = 0;
							break;
						}
					}
					else // 叠加方式
					{
						if (tfont24[k].Msk[i] & (0x01 << j))
							LCD_Draw_Point(x, y, fc); // 画一个点
						x++;
						if ((x - x0) == sizey)
						{
							x = x0;
							y++;
							break;
						}
					}
				}
			}
		}
		continue; // 查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
#endif
}

/******************************************************************************
	  函数说明：显示单个32x32汉字
	  入口数据：x,y显示坐标
				*s 要显示的汉字
				fc 字的颜色
				bc 字的背景色
				sizey 字号
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
void LCD_Show_Chinese32x32(U16 x, U16 y, char *s, U16 fc, U16 bc, char sizey, char mode)
{
#ifdef String_Chinese
	char i, j, m = 0;
	U16 k;
	U16 HZnum;		 // 汉字数目
	U16 TypefaceNum; // 一个字符所占字节大小
	U16 x0 = x;
	TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;
	HZnum = sizeof(tfont32) / sizeof(typFNT_GB32); // 统计汉字数目
	for (k = 0; k < HZnum; k++)
	{
		if ((tfont32[k].Index[0] == *(s)) && (tfont32[k].Index[1] == *(s + 1)))
		{
			LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);
			for (i = 0; i < TypefaceNum; i++)
			{
				for (j = 0; j < 8; j++)
				{
					if (!mode) // 非叠加方式
					{
						if (tfont32[k].Msk[i] & (0x01 << j))
							LCD_WR_DATA(fc);
						else
							LCD_WR_DATA(bc);
						m++;
						if (m % sizey == 0)
						{
							m = 0;
							break;
						}
					}
					else // 叠加方式
					{
						if (tfont32[k].Msk[i] & (0x01 << j))
							LCD_Draw_Point(x, y, fc); // 画一个点
						x++;
						if ((x - x0) == sizey)
						{
							x = x0;
							y++;
							break;
						}
					}
				}
			}
		}
		continue; // 查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
#endif
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
/*    以上为文字最小单元      */

/******************************************************************************
	  函数说明：显示汉字串
	  入口数据：x,y显示坐标
				*s 要显示的汉字串
				fc 字的颜色
				bc 字的背景色
				sizey 字号 可选 24 32
				mode:  0非叠加模式  1叠加模式
	  返回值：  无
******************************************************************************/
void LCD_Show_Chinese(U16 x, U16 y, char *s, U16 fc, U16 bc, char sizey, char mode)
{
	while (*s != 0)
	{
		if (sizey == 24)
		{
			LCD_Show_Chinese24x24(x, y, s, fc, bc, sizey, mode);
		}
		else if (sizey == 32)
		{
			LCD_Show_Chinese32x32(x, y, s, fc, bc, sizey, mode);
		}
		else
		{
			return;
		}
		s += 2;
		x += sizey;
	}
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
		if (x >= LCD_W)
		{
			x = (LCD_W - (sizey / 2));
		}
		if (y >= LCD_H)
		{
			y = (LCD_H - sizey);
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
void LCD_Show_Picture(U16 x, U16 y, U16 length, U16 width, const U8 pic[])
{
#ifdef Exist_LCD
	u16 i, j;
	u32 k = 0;
	int n = 0;
	uint8_t *pic_buff = malloc(650);
	LCD_Address_Set(x, y, (x + length - 1), (y + width - 1));

	for (i = 0; i < length; i++)
	{
		for (j = 0; j < width; j++)
		{
			pic_buff[n++] = pic[k * 2];
			pic_buff[n++] = pic[k * 2 + 1];
			k++;
		}
		LCD_Send_Data(pic_buff, width * 2);
		n = 0;
	}
	free(pic_buff);
#endif
}

void LCD_Init(int Set)
{
#ifdef Exist_LCD
	LCD_GPIO_Init(Set);
	SPI_Start_Init(Set);
	LCD_WR_DATA8(0x00);

	LCD_Delay(300); // 等待电路复位完成

	LCD_RES_L();
	LCD_Delay(200); //
	LCD_RES_H();
	LCD_Delay(100);

	switch (LCD_HORIZONTAL)
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

	LCD_WR_CMD(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_CMD(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_CMD(0xB7);
	LCD_WR_DATA8(0x35);

	LCD_WR_CMD(0xBB);
	LCD_WR_DATA8(0x19);

	LCD_WR_CMD(0xC0);
	LCD_WR_DATA8(0x2C);

	LCD_WR_CMD(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_CMD(0xC3);
	LCD_WR_DATA8(0x12);

	LCD_WR_CMD(0xC4);
	LCD_WR_DATA8(0x20);

	LCD_WR_CMD(0xC6);
	LCD_WR_DATA8(0x0F);

	LCD_WR_CMD(0xD0);
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_CMD(0xE0);
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

	LCD_WR_CMD(0xE1);
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

	LCD_WR_CMD(0x21);
	LCD_WR_CMD(0x11);
	LCD_WR_CMD(0x29);
	LCD_Delay(100);
	LCD_Fill(0, 0, LCD_W, LCD_H, BACK_COLOR);
#endif
}

void refresh_lcd_task(void *pvParam)
{
	static int refresh_ready = 0;
	uint8_t *array_buff;
	array_buff = malloc(300);
	memset(array_buff, 0, 100);
	memcpy(array_buff, "this is num :    ", strlen("this is num :    "));

	for (int i = 0; i < 300; i++)
	{
		array_buff[i] = (i & 0xff);
	}

	LCD_Init(TURE);
	LCD_Show_Picture(0, 0, cat_lcd_1_14_h, cat_lcd_1_14_w, gImage_cat_lcd_1_14);	// gImage_yi_lcd_1_14 gImage_cat_lcd_1_14
	// LCD_Show_String(2, 0, "1234", LCD_BLUE, LCD_WHITE, 16);
	while (1)
	{
		refresh_ready++;
		// if (refresh_ready % 2)
		// {
		// 	LCD_Fill(0, 0, LCD_W, LCD_H, LCD_RED);
		// }
		// else
		// {
		// 	LCD_Fill(0, 0, LCD_W, LCD_H, LCD_BLUE);
		// }

		LCD_Delay(1000);
		ESP_LOGI("LCD runing ", " time : %d ", refresh_ready);
	}
	free(array_buff);
}
