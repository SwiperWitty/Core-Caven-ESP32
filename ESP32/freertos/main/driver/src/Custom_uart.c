#include "Custom_uart.h"

/*
    ESP_LOGE - 错误（最低）0
    ESP_LOGW - 警告
    ESP_LOGI - 信息
    ESP_LOGD - 调试
    ESP_LOGV - 详细（最高）
*/

static const char *TAG = "Exist_UART";
QueueHandle_t uart1_event_queue;
QueueHandle_t uart2_event_queue;

int custom_uart1_init(int baud_rate, int set)
{
    int retval = 0;
#ifdef CUSTOM_UART1
    if (set)
    {
        uart_config_t uart_config = {
            .baud_rate = baud_rate,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
        uart_param_config(UART_NUM_1, &uart_config);
        uart_set_pin(UART_NUM_1, ECHO_TXD1, ECHO_RXD1, ECHO_TEST_RTS, ECHO_TEST_CTS);
        // 串口1的接收缓冲区2048，发送缓冲区2048，定义uart1_event_queue 事件消息队列大小为10
        uart_driver_install(UART_NUM_1, 2048, 2048, 10, &uart1_event_queue, 0);
    }
    else
    {
    }
    ESP_LOGI(TAG, "init %d ", retval);
#endif
    return retval;
}

int custom_uart2_init(int baud_rate, int set)
{
    int retval = 0;
#ifdef CUSTOM_UART2
    if (set)
    {
        uart_config_t uart_config = {
            .baud_rate = baud_rate,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
        uart_param_config(UART_NUM_2, &uart_config);
        uart_set_pin(UART_NUM_2, ECHO_TXD2, ECHO_RXD2, ECHO_TEST_RTS, ECHO_TEST_CTS);
        // 串口2的接收缓冲区2048，发送缓冲区2048，定义uart2_event_queue 事件消息队列大小为10
        uart_driver_install(UART_NUM_2, 2048, 2048, 10, &uart2_event_queue, 0);
    }
    else
    {
    }
    ESP_LOGI(TAG, "init %d ", retval);
#endif
    return retval;
}

/*  rfid    */
void uart1_task(void *pvParam)
{
#ifdef CUSTOM_UART1
    EventBits_t r_event;
    uart_event_t event;
    TickType_t absolute_Time = 1;
    int counter = 0;
    uint8_t data[512];
    custom_uart1_init(115200, ENABLE);
    memset(data, 0, sizeof(data));
    while (1)
    {
        // Waiting for UART event.
        if (xQueueReceive(uart1_event_queue, (void *)&event, 1))
        {
            switch (event.type)
            {
            case UART_DATA:
            {
            }
            case UART_BUFFER_FULL:
            {
                counter = uart_read_bytes(UART_NUM_1, data, 512, 0);
                // 这里需要区分是哪一个链接在通讯，串口连接则转发到串口，wifi则转发到wifi，蓝牙则转发到蓝牙，有线则转发到有线
                if (counter > 0)
                {
                    uart_write_bytes(UART_NUM_1, data, counter);
                    receive_info_frame_handle (data, counter, 1);
                }
            }
            break;
            default:
                break;
            }
        }
        else
        {
        }
        vTaskDelay(pdMS_TO_TICKS(absolute_Time));
    }
    vTaskDelete(NULL); /*  基本不用退出 */
#endif
}

/*  mcu */
void uart2_task(void *pvParam)
{
#ifdef CUSTOM_UART2

#endif
}

void custom_uart_task_Fun(void)
{

#if (BOARD_NAME == ESP32_CAVEND)
    xTaskCreate(uart1_task, "task-[uart1]", 4096 * 1, NULL, UART1_TASK_PRIORITY, NULL);
#elif (BOARD_NAME == EY1001)
    xTaskCreate(uart1_task, "task-[uart1]", 4096 * 1, NULL, UART1_TASK_PRIORITY, NULL);
    xTaskCreate(uart2_task, "task-[uart2]", 4096 * 1, NULL, UART2_TASK_PRIORITY, NULL);
#endif //
}
