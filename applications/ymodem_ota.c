/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-30     armink       the first version
 * 2018-08-27     Murphy       update log
 */

#include <rtthread.h>
#include <stdio.h>
#include <stdbool.h>
#if defined(RT_USING_FINSH)
#include <finsh.h>
#endif
#include <ymodem.h>
#include <drv_flash.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME               "ymodem"
#ifdef OTA_DOWNLOADER_DEBUG
#define DBG_LEVEL                      DBG_LOG
#else
#define DBG_LEVEL                      DBG_INFO
#endif
#define DBG_COLOR
#include <rtdbg.h>

#define PKG_USING_YMODEM_OTA

#ifdef PKG_USING_YMODEM_OTA

#define APP_DEFAULT_ADDR    FLASH_BANK2_BASE
#define APP_DEFAULT_SIZE    (FLASH_BANK2_BASE - FLASH_BASE)

struct firmware_patition{
    uint32_t addr;
    int len;
    int (*write)(rt_uint32_t addr, const rt_uint8_t *buf, size_t size);
    int (*read)(rt_uint32_t addr, rt_uint8_t *buf, size_t size);
    int (*erase)(rt_uint32_t addr, size_t size);
};

static size_t update_file_total_size, update_file_cur_size;

static struct firmware_patition app_part = {
        .addr = APP_DEFAULT_ADDR,
        .len = APP_DEFAULT_SIZE,
        .write = stm32_flash_write,
        .read = stm32_flash_read,
        .erase = stm32_flash_erase
};

#define YMODEM_DEV_NAME "uart5"
static rt_device_t ymodem_dev;

rt_device_t get_ymodem_dev()
{
    return ymodem_dev;
}

static int init_ymodem_dev()
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT; /* 初始化配置参数 */

    ymodem_dev = rt_device_find(YMODEM_DEV_NAME);
    if (!ymodem_dev) {
        rt_kprintf("find %s failed!\n", YMODEM_DEV_NAME);
        return -RT_ERROR;
    }

    config.bufsz = 64;
    rt_device_control(ymodem_dev, RT_DEVICE_CTRL_CONFIG, &config);
    return RT_EOK;
}

static enum rym_code ymodem_on_begin(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    char *file_name, *file_size;

    /* calculate and store file size */
    file_name = (char *)&buf[0];
    file_size = (char *)&buf[rt_strlen(file_name) + 1];
    update_file_total_size = atol(file_size);
    rt_kprintf("Ymodem file_size:%d\n", update_file_total_size);

    update_file_cur_size = 0;

    /* Get download partition information and erase download partition data */

    if (update_file_total_size > app_part.len)
    {
        LOG_E("Firmware is too large! File size (%d), partition size (%d)", update_file_total_size, app_part.len);
        return RYM_CODE_CAN;
    }

    LOG_I("Start erase. Size (%d)", update_file_total_size);

    /* erase DL section */
    if (app_part.erase(app_part.addr, update_file_total_size) < 0)
    {
        LOG_E("Firmware download failed! Partition erase error!");
        return RYM_CODE_CAN;
    }

    return RYM_CODE_ACK;
}

static enum rym_code ymodem_on_data(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len)
{
    /* write data of application to DL partition  */
    if (app_part.write(app_part.addr + update_file_cur_size, buf, len) < 0)
    {
        LOG_E("Firmware download failed! Partition write data error!");
        return RYM_CODE_CAN;
    }

    update_file_cur_size += len;

    return RYM_CODE_ACK;
}

void ymodem_ota(uint8_t argc, char **argv)
{
    struct rym_ctx rctx;

    rt_kprintf("Warning: Ymodem has started! This operator will not recovery.\n");
    rt_kprintf("Please select the ota firmware file and use Ymodem to send.\n");
    init_ymodem_dev();

    if (!rym_recv_on_device(&rctx, get_ymodem_dev(), RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                            ymodem_on_begin, ymodem_on_data, NULL, RT_TICK_PER_SECOND))
    {
        rt_kprintf("Download firmware to flash success.\n");              
        
        /* check firmware */
        {
            rt_kprintf("Download firmware verify........[OK]\n");  
            rt_kprintf("Reset system and apply new firmware.\n");  
            /* wait some time for terminal response finish */
            rt_thread_delay(RT_TICK_PER_SECOND); 
        }
    }
    else
    {
        rt_kprintf("Download firmware to flash fail.\n");
    }    
}
/**
 * msh />ymodem_ota
*/
MSH_CMD_EXPORT_ALIAS(ymodem_ota, ymdown, Use Y-MODEM to download the firmware);

#endif /* PKG_USING_YMODEM_OTA */
