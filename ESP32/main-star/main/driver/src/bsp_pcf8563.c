#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "bsp_pcf8563.h"
#include "reader_typedef.h"
#define CHECK_ARG(ARG)                  \
    do                                  \
    {                                   \
        if (!ARG)                       \
            return ESP_ERR_INVALID_ARG; \
    } while (0)
#define TAG "BM8563_I2C"

struct tm timeinfo;
uint8_t bcd2dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0f);
}

uint8_t dec2bcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}

esp_err_t pcf8563_init_desc(pcf_i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    esp_err_t retval;
    CHECK_ARG(dev);
    dev->port = port;
    dev->addr = PCF8563_ADDR;
    dev->sda_io_num = sda_gpio;
    dev->scl_io_num = scl_gpio;
    dev->clk_speed = PCF8563_I2C_MASTER_FREQ_HZ;

    int i2c_master_port = port;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_gpio,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl_gpio,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = PCF8563_I2C_MASTER_FREQ_HZ, /* 标准模式(100 kbit/s) */
    };

    i2c_param_config(i2c_master_port, &conf);
    retval = i2c_driver_install(i2c_master_port, conf.mode, PCA9535_I2C_MASTER_RX_BUF_DISABLE, PCA9535_I2C_MASTER_TX_BUF_DISABLE, 0);
    return retval;
}

esp_err_t i2c_dev_read(const pcf_i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size)
{
    if (!dev || !in_data || !in_size)
        return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (out_data && out_size)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, (void *)out_data, out_size, true);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | 1, true);
    i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t res = i2c_master_cmd_begin(dev->port, cmd, PCF8563_I2C_MASTER_TIMEOUT / portTICK_PERIOD_MS);
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not read from device [0x%02x at %d]: %d", dev->addr, dev->port, res);
    i2c_cmd_link_delete(cmd);

    return res;
}

esp_err_t i2c_dev_write(const pcf_i2c_dev_t *dev, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size)
{
    if (!dev || !out_data || !out_size)
        return ESP_ERR_INVALID_ARG;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
    if (out_reg && out_reg_size)
        i2c_master_write(cmd, (void *)out_reg, out_reg_size, true);
    i2c_master_write(cmd, (void *)out_data, out_size, true);
    i2c_master_stop(cmd);
    esp_err_t res = i2c_master_cmd_begin(dev->port, cmd, PCF8563_I2C_MASTER_TIMEOUT / portTICK_PERIOD_MS);
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d", dev->addr, dev->port, res);
    i2c_cmd_link_delete(cmd);
    return res;
}

inline esp_err_t i2c_dev_read_reg(const pcf_i2c_dev_t *dev, uint8_t reg,
                                  void *in_data, size_t in_size)
{
    return i2c_dev_read(dev, &reg, 1, in_data, in_size);
}

inline esp_err_t i2c_dev_write_reg(const pcf_i2c_dev_t *dev, uint8_t reg,
                                   const void *out_data, size_t out_size)
{
    return i2c_dev_write(dev, &reg, 1, out_data, out_size);
}


esp_err_t pcf8563_reset(pcf_i2c_dev_t *dev)
{
    CHECK_ARG(dev);

    uint8_t data[2];
    data[0] = 0;
    data[1] = 0;

    return i2c_dev_write_reg(dev, PCF8563_ADDR_STATUS1, data, 2);
}

esp_err_t pcf8563_set_time(pcf_i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);
    esp_err_t res = 0;
    uint8_t data[7];
    if (time->tm_year >= 100)
        time->tm_year = time->tm_year - 100;
    //  ESP_LOGI("pcf8563_set_time", "time->tm_year - 2000=%d",time->tm_year - 2000);
    /* time/date data */
    data[0] = dec2bcd(time->tm_sec);
    data[1] = dec2bcd(time->tm_min);
    data[2] = dec2bcd(time->tm_hour + 8);
    data[3] = dec2bcd(time->tm_mday);
    data[4] = dec2bcd(time->tm_wday);    // tm_wday is 0 to 6
    data[5] = dec2bcd(time->tm_mon + 1); // tm_mon is 0 to 11
    data[6] = dec2bcd(time->tm_year);
    ESP_LOGI("pcf8563_set_time", "time->tm_year=%d", time->tm_year);
    // return i2c_dev_write_reg(dev, PCF8563_ADDR_TIME, data, 7);
    ESP_LOGI("i2c_dev_write_reg", "data=sec:%02x min:%02x hour:%02x mday:%02x month:%02x year:%02x",
             data[0], data[1], data[2], data[3], data[5], data[6]);
    res = i2c_dev_write_reg(dev, PCF8563_ADDR_TIME, data, 7);

    i2c_dev_read_reg(dev, PCF8563_ADDR_TIME, data, 7);
    data[0] = bcd2dec(data[0] & 0x7F);
    data[1] = bcd2dec(data[1] & 0x7F);
    data[2] = bcd2dec(data[2] & 0x3F);
    data[3] = bcd2dec(data[3] & 0x3F);
    data[4] = bcd2dec(data[4] & 0x07); // tm_wday is 0 to 6
    data[5] = bcd2dec(data[5] & 0x1F); // tm_mon is 0 to 11
    data[6] = bcd2dec(data[6]);
    ESP_LOGI("i2c_dev_read_reg", "data=sec:%02d min:%02d hour:%0d mday:%02d  month:%02d year:%02d",
             data[0], data[1], data[2], data[3], data[5], data[6]);
    return res;
}

esp_err_t pcf8563_get_time(pcf_i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t data[7];

    /* read time */
    esp_err_t res = i2c_dev_read_reg(dev, PCF8563_ADDR_TIME, data, 7);
    if (res != ESP_OK)
        return res;

    /* convert to unix time structure */
    ESP_LOGI("", "data=sec:%02x min:%02x hour:%02x mday:%02x wday:%02x month:%02x year:%02x",
             data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
    time->tm_sec = bcd2dec(data[0] & 0x7F);
    time->tm_min = bcd2dec(data[1] & 0x7F);
    time->tm_hour = bcd2dec(data[2] & 0x3F);
    time->tm_mday = bcd2dec(data[3] & 0x3F);
    time->tm_wday = bcd2dec(data[4] & 0x07);    // tm_wday is 0 to 6
    time->tm_mon = bcd2dec(data[5] & 0x1F) - 1; // tm_mon is 0 to 11
    time->tm_year = bcd2dec(data[6]) + 2000;
    time->tm_isdst = 0;

    return ESP_OK;
}

pcf_i2c_dev_t pcf8563_i2c_device;

void pcf8563_init(void)
{
    pcf8563_init_desc(&pcf8563_i2c_device, PCF8563_I2C_MASTER_NUMBER, EEPROM_I2C_MASTER_SDA_IO, EEPROM_I2C_MASTER_SCL_IO);
}

void pcf8563_test(void)
{
    // update 'now' variable with current time
    time_t now;

    char strftime_buf[64];
    time(&now);
    now = now + (9 * 60 * 60);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo); // 根据format中定义的格式化规则,格式化timeinfo表示的时间,并将它存储在strtime_buf中  %c 表示格式化为ASCII码
    ESP_LOGI("rtc_init", "The current date/time is: %s", strftime_buf);
    ESP_LOGI("rtc_init", "timeinfo.tm_sec=%d", timeinfo.tm_sec);
    ESP_LOGI("rtc_init", "timeinfo.tm_min=%d", timeinfo.tm_min);
    ESP_LOGI("rtc_init", "timeinfo.tm_hour=%d", timeinfo.tm_hour);
    ESP_LOGI("rtc_init", "timeinfo.tm_wday=%d", timeinfo.tm_wday);
    ESP_LOGI("rtc_init", "timeinfo.tm_mday=%d", timeinfo.tm_mday);
    ESP_LOGI("rtc_init", "timeinfo.tm_mon=%d", timeinfo.tm_mon);
    ESP_LOGI("rtc_init", "timeinfo.tm_year=%d", timeinfo.tm_year);
    struct tm time = {
        .tm_year = timeinfo.tm_year + 1900,
        .tm_mon = timeinfo.tm_mon, // 0--1月份  1---2月份   2---3月份  依次类推
        .tm_mday = timeinfo.tm_mday,
        .tm_hour = timeinfo.tm_hour,
        .tm_min = timeinfo.tm_min,
        .tm_sec = timeinfo.tm_sec};
    pcf8563_init();
    if (pcf8563_set_time(&pcf8563_i2c_device, &time) != ESP_OK)
    {
        ESP_LOGE("rtc_init", "Could not set time.");
        // while (1) { vTaskDelay(1); }
    }
    else
        ESP_LOGI("rtc_init", "Set initial date time done");
}

void pcf8563_get_timer_init(void)
{
#if RTC_BM8563
    pcf8563_init();
    pcf8563_get_time(&pcf8563_i2c_device, &timeinfo);
    ESP_LOGI("rtc_init", "timeinfo.tm_sec=%d", timeinfo.tm_sec);
    ESP_LOGI("rtc_init", "timeinfo.tm_min=%d", timeinfo.tm_min);
    ESP_LOGI("rtc_init", "timeinfo.tm_hour=%d", timeinfo.tm_hour);
    ESP_LOGI("rtc_init", "timeinfo.tm_wday=%d", timeinfo.tm_wday);
    ESP_LOGI("rtc_init", "timeinfo.tm_mday=%d", timeinfo.tm_mday);
    ESP_LOGI("rtc_init", "timeinfo.tm_mon=%d", timeinfo.tm_mon);
    ESP_LOGI("rtc_init", "timeinfo.tm_year=%d", timeinfo.tm_year);
#endif // 
}

esp_err_t pcf8563_set_timer_my_self(time_t utc_second, time_t utc_ms)
{
    char strftime_buf[64];
    esp_err_t res = 0;
#if RTC_BM8563
    int a = 0;
    localtime_r(&utc_second, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo); // 根据format中定义的格式化规则,格式化timeinfo表示的时间,并将它存储在strtime_buf中  %c 表示格式化为ASCII码
    ESP_LOGI("RTC8563_set_timer", "The current date/time is: %s", strftime_buf);
    ESP_LOGI("RTC8563_set_timer", "timeinfo.tm_wday=%d", timeinfo.tm_wday + 1);
    ESP_LOGI("RTC8563_set_timer", "timeinfo.tm_year=%d", timeinfo.tm_year + 1900);
    ESP_LOGI("RTC8563_set_timer", "timeinfo.tm_mon=%d", timeinfo.tm_mon + 1);
    ESP_LOGI("RTC8563_set_timer", "timeinfo.tm_mday=%d", timeinfo.tm_mday);
    ESP_LOGI("RTC8563_set_timer", "timeinfo.tm_hour=%d", timeinfo.tm_hour + 8);
    ESP_LOGI("RTC8563_set_timer", "timeinfo.tm_min=%d", timeinfo.tm_min);
    ESP_LOGI("RTC8563_set_timer", "timeinfo.tm_sec=%d", timeinfo.tm_sec);
    for (a = 0; a < 20; a++)
    {
        res = pcf8563_set_time(&pcf8563_i2c_device, &timeinfo);
        if (res == ESP_OK)
        {
            break;
        }
    }

    if (ESP_OK == res)
        ESP_LOGI("RTC8563_set_timer", "write return value is ESP_OK, try counter=%d", a);
    else
        ESP_LOGI("RTC8563_set_timer", "write return value is ESP_FAIL,try counter=%d", a);
#endif // 
    return res;
}

esp_err_t pcf8563_get_timer_my_self(uint32_t *utc_second)
{
    esp_err_t res = 0x0A;
#if RTC_BM8563
    uint8_t counter = 0;
    for (counter = 0; counter < 3; counter++)
    {
        res = pcf8563_get_time(&pcf8563_i2c_device, &timeinfo);
        if (res == ESP_OK)
        {
            ESP_LOGW("RTC8563_get_time", "get RTC time : %d ", counter + 1);
            ESP_LOGI("RTC8563_get_time", "date %d / %d / %d ", timeinfo.tm_year,timeinfo.tm_mon + 1,timeinfo.tm_wday);
            ESP_LOGI("RTC8563_get_time", "watch %d:%d:%d ", timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
            timeinfo.tm_year = timeinfo.tm_year - 1900; // 规则是自1900起的年份
            timeinfo.tm_hour = timeinfo.tm_hour - 8;    // 要减去8个时区

            break;
        }
    }

    if (res != ESP_OK)
    {
        ESP_LOGW("RTC8563_get_time", "Read %d , Return destroy", counter);
    }

    if (res == ESP_OK)
        utc_second[0] = mktime(&timeinfo);
    else
        utc_second[0] = 0;
    ESP_LOGI("RTC8563_get_timer", "utc_second=%04x", utc_second[0]);
#endif // 
    return res;
}
