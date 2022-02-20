/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-02-20     dahong       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#define WDT_DEVICE_NAME    "wdt"    /* 看门狗设备名称 */
static rt_device_t wdg_dev;         /* 看门狗设备句柄 */
static void idle_hook(void);

int init_watchdog(void)
{
    rt_uint32_t timeout = 1;       /* 溢出时间，单位：秒*/
    /* 根据设备名称查找看门狗设备，获取设备句柄 */
    wdg_dev = rt_device_find(WDT_DEVICE_NAME);

    /* 初始化设备 */
    rt_device_init(wdg_dev);

    /* 设置看门狗溢出时间 */
    rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);

    /* 设置空闲线程回调函数 */
    rt_thread_idle_sethook(idle_hook);

    return 0;
}

static void idle_hook(void)
{
    /* 在空闲线程的回调函数里喂狗 */
    rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
}
