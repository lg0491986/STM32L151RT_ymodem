/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-28     RT-Thread    first version
 */

#include <time.h>
#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_flash.h"
#include "wdt.h"
#include "ble.h"

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

extern void rtc_wkup_enable(uint32_t seconds);
extern int rtc_wkup_flag_get(void);
extern void system_clock_config(int target_freq_Mhz);

#define BLE_RX_PIN  GET_PIN(D, 2)
static uint8_t ble_irt_flag = 0;

static void debug_uart_irq_cb(void *args)
{
    rt_kprintf("ble irq\n");
    ble_irt_flag = 1;
    rt_pin_irq_enable(BLE_RX_PIN, PIN_IRQ_ENABLE);
}

static int enable_ble_interupt()
{
    rt_pin_mode(BLE_RX_PIN, PIN_MODE_INPUT);
    rt_pin_mode(BLE_RX_PIN, PIN_MODE_INPUT);
    rt_pin_attach_irq(BLE_RX_PIN, PIN_IRQ_MODE_RISING_FALLING, debug_uart_irq_cb, RT_NULL);
    rt_pin_irq_enable(BLE_RX_PIN, PIN_IRQ_ENABLE);
    return 0;
}

static int disable_ble_interupt()
{
    rt_pin_irq_enable(BLE_RX_PIN, PIN_IRQ_DISABLE);
    rt_pin_detach_irq(BLE_RX_PIN);
    rt_kprintf("ble irq flag: %d\n", ble_irt_flag);
    ble_irt_flag = 0;
    return 0;
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC0 PC1 PC2
                           PC3 PC4 PC5 PC6
                           PC7 PC8 PC9 PC10
                           PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 PA5
                           PA6 PA7 PA8 PA9
                           PA10 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB2 PB10 PB11
                           PB12 PB13 PB14 PB15
                           PB3 PB4 PB5 PB6
                           PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

static int close_peripheral()
{
    /* todo */
    return 0;
}

static int get_startup_source(void)
{
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET)
    {
        LOG_E("software reset!\n");
    }
    if( __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET)
    {
        LOG_E("watch dog reset!\n");
    }
    __HAL_RCC_CLEAR_RESET_FLAGS(); /* 清除RCC各个复位标志 */

    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU) != RESET)
        {
            rt_kprintf(" wake up!\n");
        }
    }

    /*RTC唤醒*/
    if(rtc_wkup_flag_get() == 1)
    {
        rt_kprintf("RTC wake up!\n");
    }

    rt_kprintf("[%s:%d]\n", __func__, __LINE__);

    /* clean pending interupt,set bit 0~16 and 18~22 0x7dffff */
    __HAL_GPIO_EXTI_CLEAR_IT(0xffffff);
    return 0;
}
INIT_ENV_EXPORT(get_startup_source);

void stop_mode(uint8_t argc, char **argv)
{
    __HAL_RCC_CLEAR_RESET_FLAGS();  /*清除RCC各个复位标志*/
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

     /*清除所有相关唤醒标志位*/
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* 关闭外设及其电源 */
    close_peripheral();

    rt_kprintf("enter low power mode!\n");
//    set_pins_analog();

    /* 开启BLE唤醒中断 */
    enable_ble_interupt();

    rtc_wkup_enable(28); /* 调整RTC唤醒周期20s */

    __HAL_GPIO_EXTI_CLEAR_IT(0xffffff);

    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    /* restore from stop mode */
    system_clock_config(BSP_CLOCK_SYSTEM_FREQ_MHZ);

    disable_ble_interupt();

    config_ble();

    rt_kprintf("wakeup from low power mode!\n");

    get_startup_source();
    /* 复位 */
//    HAL_NVIC_SystemReset();
}
MSH_CMD_EXPORT_ALIAS(stop_mode, lowpower, enter lowpower);


void standby_mode(uint8_t argc, char **argv)
{
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    //HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
//    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
//    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);
    /* 清除所有相关唤醒标志位 */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    LOG_I("LOW_MODE\n");
    rtc_wkup_enable(8); /*调整RTC唤醒周期*/

    LOG_I("Will enter standby\r\n");
    HAL_PWR_EnterSTANDBYMode();
}
MSH_CMD_EXPORT_ALIAS(standby_mode, standby, enter standby);

/* 同步时间 */
int sync_time()
{
    rt_err_t ret = RT_EOK;
    ret = set_date(2018, 12, 3);
    if (ret != RT_EOK)
    {
        rt_kprintf("set RTC date failed\n");
        return ret;
    }
    /* 设置时间 */
    ret = set_time(11, 15, 50);
    if (ret != RT_EOK)
    {
        rt_kprintf("set RTC time failed\n");
        return ret;
    }

    return ret;
}

int main(void)
{
    uint32_t count = 1;
    time_t now;

    LOG_D("Hello RT-Thread! %d", count);

    init_watchdog();
    MX_GPIO_Init();
    init_ble();
    /* 同步时间 */
//    sync_time();
    //    load_app(0, RT_NULL);
    while (count++)
    {
        LOG_D("Hello RT-Thread! %d", count);
        rt_thread_mdelay(1000);
        /* 获取时间 */
        now = time(RT_NULL);
        rt_kprintf("%s\n", ctime(&now));
    }

    return RT_EOK;
}

