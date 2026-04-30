#include "center_app.h"
#include "show_app.h"
#include "Mode.h"

void Main_Init(void);
#include "driver/i2c.h"
#define QMI8658_SENSOR_ADDR	0x6a

enum qmi8658_reg
{
    QMI8658_WHO_AM_I = 0,
    QMI8658_REVISION_ID,
    QMI8658_CTRL1,
    QMI8658_CTRL2,
    QMI8658_CTRL3,
    QMI8658_CTRL4,
    QMI8658_CTRL5,
    QMI8658_CTRL6,
    QMI8658_CTRL7,
    QMI8658_CTRL8,
    QMI8658_CTRL9,
    QMI8658_CATL1_L,
    QMI8658_CATL1_H,
    QMI8658_CATL2_L,
    QMI8658_CATL2_H,
    QMI8658_CATL3_L,
    QMI8658_CATL3_H,
    QMI8658_CATL4_L,
    QMI8658_CATL4_H,
    QMI8658_FIFO_WTM_TH,
    QMI8658_FIFO_CTRL,
    QMI8658_FIFO_SMPL_CNT,
    QMI8658_FIFO_STATUS,
    QMI8658_FIFO_DATA,
    QMI8658_I2CM_STATUS = 44,
    QMI8658_STATUSINT,
    QMI8658_STATUS0,
    QMI8658_STATUS1,
    QMI8658_TIMESTAMP_LOW,
    QMI8658_TIMESTAMP_MID,
    QMI8658_TIMESTAMP_HIGH,
    QMI8658_TEMP_L,
    QMI8658_TEMP_H,
    QMI8658_AX_L,
    QMI8658_AX_H,
    QMI8658_AY_L,
    QMI8658_AY_H,
    QMI8658_AZ_L,
    QMI8658_AZ_H,
    QMI8658_GX_L,
    QMI8658_GX_H,
    QMI8658_GY_L,
    QMI8658_GY_H,
    QMI8658_GZ_L,
    QMI8658_GZ_H,
    QMI8658_MX_L,
    QMI8658_MX_H,
    QMI8658_MY_L,
    QMI8658_MY_H,
    QMI8658_MZ_L,
    QMI8658_MZ_H,
    QMI8658_dQW_L = 73,
    QMI8658_dQW_H,
    QMI8658_dQX_L,
    QMI8658_dQX_H,
    QMI8658_dQY_L,
    QMI8658_dQY_H,
    QMI8658_dQZ_L,
    QMI8658_dQZ_H,
    QMI8658_dVX_L,
    QMI8658_dVX_H,
    QMI8658_dVY_L,
    QMI8658_dVY_H,
    QMI8658_dVZ_L,
    QMI8658_dVZ_H,
    QMI8658_AE_REG1,
    QMI8658_AE_REG2,
    QMI8658_RESET = 96
};

// 读取QMI8658寄存器的值
esp_err_t qmi8658_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
	int retval = 0;
	retval = Base_IIC_Send_DATA(QMI8658_SENSOR_ADDR,&reg_addr,1,sizeof(reg_addr),10,1);
	retval |= Base_IIC_Receive_DATA(QMI8658_SENSOR_ADDR,data,1,len,10);
	Debug_printf("qmi8658 read %d flag",retval);
	return 0;
    // return i2c_master_write_read_device(I2C_NUM_0, QMI8658_SENSOR_ADDR,  &reg_addr, 1, data, len, 10 / portTICK_PERIOD_MS);
}

// 给QMI8658的寄存器写值
esp_err_t qmi8658_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
	Base_IIC_Send_DATA(QMI8658_SENSOR_ADDR,write_buf,1,sizeof(write_buf),10,0);
	return 0;
    // return i2c_master_write_to_device(I2C_NUM_0, QMI8658_SENSOR_ADDR, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS);
}

void qmi8658_init(void)
{
    uint8_t id = 0; // 芯片的ID号
	ESP_LOGI("TAG", "QMI8658 go!");  // 打印信息
    qmi8658_register_read(QMI8658_WHO_AM_I, &id ,1); // 读芯片的ID号
    while (id != 0x05)  // 判断读到的ID号是否是0x05
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);  // 延时1秒
        qmi8658_register_read(QMI8658_WHO_AM_I, &id ,1); // 读取ID号
		ESP_LOGE("ERROR", "QMI8658 IC ERROR !");  // 打印信息
    }
    ESP_LOGI("TAG", "QMI8658 OK!");  // 打印信息

    qmi8658_register_write_byte(QMI8658_RESET, 0xb0);  // 复位
    vTaskDelay(10 / portTICK_PERIOD_MS);  // 延时10ms
    qmi8658_register_write_byte(QMI8658_CTRL1, 0x40); // CTRL1 设置地址自动增加
    qmi8658_register_write_byte(QMI8658_CTRL7, 0x03); // CTRL7 允许加速度和陀螺仪
    qmi8658_register_write_byte(QMI8658_CTRL2, 0x95); // CTRL2 设置ACC 4g 250Hz
    qmi8658_register_write_byte(QMI8658_CTRL3, 0xd5); // CTRL3 设置GRY 512dps 250Hz
}

uint8_t iic_buf[10] = {0xff,6,7};
uint8_t iic_array[10];
struct tm date;

int app_main(void)
{
    Caven_BaseTIME_Type now_time;
	Task_Overtime_Type led_task;
	
    Main_Init();
    now_time.SYS_Sec = 1742299486;
    Mode_Use.TIME.Set_BaseTIME_pFun(now_time);
	Debug_printf("Hello world !");
	int led = 0;
	led_task.Switch = 1;
	led_task.Set_time.SYS_Sec = 0;
	led_task.Set_time.SYS_Us = 500000;

	memset(iic_array,0,sizeof(iic_array));
        qmi8658_init();
	while(1)
    {
		now_time = Mode_Use.TIME.Get_BaseTIME_pFun();
		date = API_UTC_Get_Date (now_time.SYS_Sec,8);
		API_Task_Timer (&led_task,now_time);
		User_GPIO_set(0,2,now_time.SYS_Sec % 2);
		if (led_task.Trigger_flag)
		{
			led ++;
			// Base_IIC_Send_DATA(0x5a,iic_buf,1,1,10,1);
			// Base_IIC_Receive_DATA(0x5a,iic_array,1,1,10);
			// User_GPIO_set(0,22,led % 2);
			// Debug_printf("Begin_time %d:%d us,code:%d",led_task.Begin_time.SYS_Sec,led_task.Begin_time.SYS_Us,led_task.error_code);
			// Debug_printf("SET time %d:%d us",led_task.Set_time.SYS_Sec,led_task.Set_time.SYS_Us);
			// Debug_printf("utc %d:%d us",now_time.SYS_Sec,now_time.SYS_Us);
			led_task.Trigger_flag = 0;
		}
		Mode_Use.TIME.Delay_Ms(1);
    }
	Mode_Use.TIME.Delay_Ms (250);
    SYS_RESET();
}

TaskHandle_t show_app_taskhanlde = NULL;

void Main_Init(void)
{
    Mode_Index();
	Mode_Init.TIME(ENABLE);
	Mode_Use.TIME.Delay_Ms (1);

	User_GPIO_config(0,2,WRITE_Config);
	User_GPIO_config(0,22,WRITE_Config);	// out1
	User_GPIO_config(0,25,READ_Config);		// ds18b20
	User_GPIO_config(0,36,READ_Config);		// key
	User_GPIO_set(0,2,0);
	User_GPIO_set(0,22,0);
	xTaskCreate(Show_app_Task, "task-[show_app]", 1024*8, NULL, SHOW_TASK_PRIORITY, &show_app_taskhanlde);
	Debug_printf("ESP_Hello !");

	Base_IIC_Init(ENABLE);
	// Center_app_Init ();
	// System_app_Init ();
}
