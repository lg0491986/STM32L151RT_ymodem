/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-28     RT-Thread    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_flash.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define DEBUG_UART_NAME     "uart1"
#define BDS_UART_NAME       "uart4"
#define BLE_UART_NAME       "uart5"
#define AP_UART_NAME        "uart3"

#define SIG_LED GET_PIN(B, 14)

void print_hex(rt_uint8_t *buf, int len)
{
    for (int i = 0; i < len; i++) {
        if (i % 16 == 0) rt_kprintf("\n");
        rt_kprintf("%02x ", buf[i]);
    }
    rt_kprintf("\n");
}

struct uart_rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

int flash_test()
{
#define FLASH_TEST_START    FLASH_BANK2_BASE
    int ret;
    rt_uint8_t buf[8] = {0};
    rt_uint8_t write_buf[8] = {0x12, 0x13, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18};

    ret = stm32_flash_read(FLASH_TEST_START, buf, sizeof(buf));
    if (ret > 0) {
        print_hex(buf, sizeof(buf));
    }

    ret = stm32_flash_erase(FLASH_TEST_START, sizeof(buf));
    if (ret < 0) {
        LOG_E("erase error. ret: %d\n", ret);
    }

    ret = stm32_flash_write(FLASH_TEST_START, write_buf, sizeof(write_buf));
    if (ret < 0) {
        LOG_E("write error. ret: %d\n", ret);
    }
    print_hex(write_buf, sizeof(write_buf));

    ret = stm32_flash_read(FLASH_TEST_START, buf, sizeof(buf));
    if (ret > 0) {
        print_hex(buf, sizeof(buf));
    }

    return ret;
}

int main(void)
{
    uint32_t count = 1;

    /*
    rt_pin_mode(SIG_LED, PIN_MODE_OUTPUT);
    rt_pin_write(SIG_LED, PIN_HIGH);
     */
    LOG_D("Hello RT-Thread! %d", count);
    while (count++)
    {
        LOG_D("Hello RT-Thread! %d", count);
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}

int app_vector(void)
{
#define RT_APP_PART_ADDR 0x08040000
#define NVIC_VTOR_MASK  0x3FFFFF80
    SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;
    return 0;
}

INIT_BOARD_EXPORT(app_vector);
