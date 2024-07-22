#ifndef MAIN_PCF8563_H_
#define MAIN_PCF8563_H_

#include <time.h>
#include <stdbool.h>
#include "driver/i2c.h"

//#include "i2cdev.h"

#define PCF8563_ADDR 0x51 //!< I2C address      读地址0xA3  写地址0xA2  应为程序中需要左移1bit，所以这里定义需要改写为0x51

#define PCF8563_ADDR_STATUS1 0x00
#define PCF8563_ADDR_STATUS2 0x01
#define PCF8563_ADDR_TIME    0x02
#define PCF8563_ADDR_ALARM   0x09
#define PCF8563_ADDR_CONTROL 0x0d
#define PCF8563_ADDR_TIMER   0x0e
typedef struct {
    i2c_port_t port;            // I2C port number
    uint8_t addr;               // I2C address
    gpio_num_t sda_io_num;      // GPIO number for I2C sda signal
    gpio_num_t scl_io_num;      // GPIO number for I2C scl signal
        uint32_t clk_speed;             // I2C clock frequency for master mode
} pcf_i2c_dev_t;

uint8_t bcd2dec(uint8_t val);
uint8_t dec2bcd(uint8_t val);
void pcf8563_get_timer_init(void);
esp_err_t pcf8563_set_timer_my_self(time_t utc_second ,time_t utc_ms);
esp_err_t pcf8563_get_timer_my_self(uint32_t *utc_second);
// esp_err_t pcf8563_init_desc(i2c_config_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);
// esp_err_t pcf8563_reset(i2c_config_t *dev);
// esp_err_t pcf8563_set_time(i2c_config_t *dev, struct tm *time);
// esp_err_t pcf8563_get_time(i2c_config_t *dev, struct tm *time);
#endif /* MAIN_PCF8563_H_ */
