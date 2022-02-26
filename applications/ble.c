#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "ble.h"

#define BLE_PWR_PIN     GET_PIN(B, 0)
#define BLE_UART_NAME    "uart5"
/* 用于接收消息的信号量 */
static struct rt_semaphore ble_rx_sem;
static rt_device_t ble_serial;

/* 接收数据回调函数 */
static rt_err_t ble_uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&ble_rx_sem);

    return RT_EOK;
}

static void ble_serial_thread_entry(void *parameter)
{
    char ch;

    while (1)
    {
        /* 从串口读取一个字节的数据，没有读取到则等待接收信号量 */
        while (rt_device_read(ble_serial, -1, &ch, 1) != 1)
        {
            /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
            rt_sem_take(&ble_rx_sem, RT_WAITING_FOREVER);
        }
        /* 读取到的数据通过串口错位输出 */
        rt_kprintf("ble: %c\n", ch);
        ch = ch + 1;
        rt_device_write(ble_serial, 0, &ch, 1);
    }
}

static void ble_power_on(void)
{
    rt_pin_mode(BLE_PWR_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(BLE_PWR_PIN, PIN_HIGH);
}

int config_ble(void)
{
    return rt_device_control(ble_serial, RT_DEVICE_CTRL_CONFIG, RT_NULL);
}

int init_ble(void)
{
    rt_err_t ret = RT_EOK;

    ble_power_on();

    /* 查找系统中的串口设备 */
    ble_serial = rt_device_find(BLE_UART_NAME);
    if (!ble_serial)
    {
        rt_kprintf("find %s failed!\n", BLE_UART_NAME);
        return RT_ERROR;
    }

    /* 初始化信号量 */
    rt_sem_init(&ble_rx_sem, "ble_rx_sem", 0, RT_IPC_FLAG_FIFO);
    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(ble_serial, RT_DEVICE_FLAG_INT_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(ble_serial, ble_uart_input);
    /* 发送字符串 */
//    rt_device_write(ble_serial, 0, str, (sizeof(str) - 1));

    /* 创建 ble_serial 线程 */
    rt_thread_t thread = rt_thread_create("ble_serial", ble_serial_thread_entry, RT_NULL, 1024, 25, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }
    return ret;
}
