#include "show_app.h"

//2bit
#define black   0x00  /// 00
#define white   0x01  /// 01
#define yellow  0x02  /// 10
#define red     0x03  /// 11


#define Source_BITS     152
#define Gate_BITS       152
#define ALLSCREEN_BYTES 5776 //Source_BITS*Gate_BITS/4

void delay (int n)
{
    Mode_Use.TIME.Delay_Ms(n);
}

//SPI write byte
void SPI_Write(unsigned char value)
{				   			 
   Base_SPI_Send_Data(1,value);
}

//SPI write command
void EPD_W21_WriteCMD(unsigned char command)
{
	// EPD_W21_CS_0;
	User_GPIO_set(0,25,0);  // D/C#   0:command  1:data  
	SPI_Write(command);
    // User_GPIO_set(0,25,1);
	// EPD_W21_CS_1;
}
//SPI write data
void EPD_W21_WriteDATA(unsigned char datas)
{
    User_GPIO_set(0,25,1);
	SPI_Write(datas);
}

//
void lcd_chkstatus(void)
{ 
    while(1)
    {  //=0 BUSY
        if(User_GPIO_get(0,32)==1) break;
        Mode_Use.TIME.Delay_Ms(1);
    }
}
  
void EPD_sleep(void)
{   
  EPD_W21_WriteCMD(0X02);   //power off
  lcd_chkstatus();          //waiting for the electronic paper IC to release the idle signal
  delay(100);   //!!!The delay here is necessary,100mS at least!!!     
 
  EPD_W21_WriteCMD(0X07);   //deep sleep
  EPD_W21_WriteDATA(0xA5);
}
void EPD_refresh(void)
{   
  EPD_W21_WriteCMD(0x12); //Display Update Control
  EPD_W21_WriteDATA(0x00);
  lcd_chkstatus();   
}

void Display_All_Black(void)
{
  unsigned long i; 

  EPD_W21_WriteCMD(0x10);
  {
    for(i=0;i<ALLSCREEN_BYTES;i++)
    {
      EPD_W21_WriteDATA(0x00);
    }
  } 
  EPD_refresh();  
  
}

void Display_All_White(void)
{
  unsigned long i;
 
  EPD_W21_WriteCMD(0x10);
  {
    for(i=0;i<ALLSCREEN_BYTES;i++)
    {
      EPD_W21_WriteDATA(0x55);
    }
  } 
   EPD_refresh(); 
}

void Display_All_Yellow(void)
{
  unsigned long i;
 
  EPD_W21_WriteCMD(0x10);
  {
    for(i=0;i<ALLSCREEN_BYTES;i++)
    {
      EPD_W21_WriteDATA(0xaa);
    }
  }
   EPD_refresh(); 
}


void Display_All_Red(void)
{
  unsigned long i;
 
  EPD_W21_WriteCMD(0x10);
  {
    for(i=0;i<ALLSCREEN_BYTES;i++)
    {
      EPD_W21_WriteDATA(0xff);
    }
  } 
   EPD_refresh(); 
}



unsigned char Color_get(unsigned char color)
{
  unsigned char datas = 0;
  switch(color)
  {
    case 0x00:
      datas=white;  
      break;    
    case 0x01:
      datas=yellow;
      break;
    case 0x02:
      datas=red;
      break;    
    case 0x03:
      datas=black;
      break;      
    default:
      break;      
  }
   return datas;
}



void PIC_display(const unsigned char* picData)
{
  unsigned int i,j;
  unsigned char temp1;
  unsigned char data_H1,data_H2,data_L1,data_L2,data;
   
  EPD_W21_WriteCMD(0x10);        
  for(i=0;i<Gate_BITS;i++)  //Source_BITS*Gate_BITS/4
  { 
    for(j=0;j<Source_BITS/4;j++)
    {   
      temp1=picData[i*Source_BITS/4+j]; 

      data_H1=Color_get(temp1>>6&0x03)<<6;      
      data_H2=Color_get(temp1>>4&0x03)<<4;
      data_L1=Color_get(temp1>>2&0x03)<<2;
      data_L2=Color_get(temp1&0x03);
      
      data=data_H1|data_H2|data_L1|data_L2;
      EPD_W21_WriteDATA(data);
    }
  } 
  
   //Refresh
    EPD_refresh();  
}

void EPD_init(void)
{
//   delay(20);//At least 20ms delay   
//   EPD_W21_RST_0;    // Module reset
//   delay(40);//At least 40ms delay 
//   EPD_W21_RST_1;
//   delay(50);//At least 50ms delay 
  
  lcd_chkstatus();
  EPD_W21_WriteCMD(0x4D);
  EPD_W21_WriteDATA(0x78);

  EPD_W21_WriteCMD(0x00); //PSR
  EPD_W21_WriteDATA(0x0F);
  EPD_W21_WriteDATA(0x29);

  EPD_W21_WriteCMD(0x01); //PWRR
  EPD_W21_WriteDATA(0x07);
  EPD_W21_WriteDATA(0x00);
  
  EPD_W21_WriteCMD(0x03); //POFS
  EPD_W21_WriteDATA(0x10);
  EPD_W21_WriteDATA(0x54);
  EPD_W21_WriteDATA(0x44);
  
  EPD_W21_WriteCMD(0x06); //BTST_P
  EPD_W21_WriteDATA(0x05);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x3F);
  EPD_W21_WriteDATA(0x0A);
  EPD_W21_WriteDATA(0x25);
  EPD_W21_WriteDATA(0x12);
  EPD_W21_WriteDATA(0x1A); 

  EPD_W21_WriteCMD(0x50); //CDI
  EPD_W21_WriteDATA(0x37);
  
  EPD_W21_WriteCMD(0x60); //TCON
  EPD_W21_WriteDATA(0x02);
  EPD_W21_WriteDATA(0x02);
  
  EPD_W21_WriteCMD(0x61); //TRES
  EPD_W21_WriteDATA(Source_BITS/256);   // Source_BITS_H
  EPD_W21_WriteDATA(Source_BITS%256);   // Source_BITS_L
  EPD_W21_WriteDATA(Gate_BITS/256);     // Gate_BITS_H
  EPD_W21_WriteDATA(Gate_BITS%256);     // Gate_BITS_L  
  
  EPD_W21_WriteCMD(0xE7);
  EPD_W21_WriteDATA(0x1C);
  
  EPD_W21_WriteCMD(0xE3); 
  EPD_W21_WriteDATA(0x22);
  
  EPD_W21_WriteCMD(0xB4);
  EPD_W21_WriteDATA(0xD0);
  EPD_W21_WriteCMD(0xB5);
  EPD_W21_WriteDATA(0x03);
  
  EPD_W21_WriteCMD(0xE9);
  EPD_W21_WriteDATA(0x01); 

  EPD_W21_WriteCMD(0x30);
  EPD_W21_WriteDATA(0x08);  
  
  EPD_W21_WriteCMD(0x04);
  lcd_chkstatus();   
}

#include "Ap_29demo.h"

void Show_app_Task (void * empty)
{
    Show_app_Init();
    Debug_printf("Show_app_Task init ");
    EPD_init(); //Full screen refresh initialization.
    Debug_printf("Show_app_Task EPD_init ");
    PIC_display(gImage_1);//To Display one image using full screen refresh.
    Debug_printf("Show_app_Task Image ");
    while (1)
    {
        Mode_Use.TIME.Delay_Ms(1);
    }
}

void Show_app_Init (void)
{
    User_GPIO_config(0,25,WRITE_Config);
    User_GPIO_config(0,26,WRITE_Config);
    User_GPIO_config(0,32,READ_Config);
    Base_SPI_Init(1,8,ENABLE);
    Base_SPI_Send_Data(1,0X5A);

    Mode_Use.TIME.Delay_Ms(50);
    User_GPIO_set(0,25,1);      // DC
    User_GPIO_set(0,26,0);      // RES
    Mode_Use.TIME.Delay_Ms(40);
    User_GPIO_set(0,26,1);
    Mode_Use.TIME.Delay_Ms(50);
}

void Show_app_Exit (void)
{

}

