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

typedef void(*app_load_func)(void);
void load_app(uint8_t argc, char **argv)
{
#define APP_ADDRESS 0x08030000UL
  app_load_func app_load;
  uint32_t app_addr = APP_ADDRESS;

  rt_kprintf("[%s:%d]\n", __func__, __LINE__);
  if(((*(__IO uint32_t*)(app_addr+4))&0xFF000000)==0x08000000){
      if(((*(__IO uint32_t*)app_addr)&0x2FFE0000)==0x20000000)
      {
          app_load = (app_load_func)*(__IO uint32_t*)(app_addr + 4);
          __set_MSP(*(__IO uint32_t*)(app_addr));
          app_load();
          rt_kprintf("[%s:%d]\n", __func__, __LINE__);
      }
  }
}
MSH_CMD_EXPORT_ALIAS(load_app, loadapp, load app);

int main(void)
{
    uint32_t count = 1;

    LOG_D("Hello RT-Thread! %d", count);
//    load_app(0, RT_NULL);
    while (count++)
    {
        LOG_D("Hello RT-Thread! %d", count);
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}

