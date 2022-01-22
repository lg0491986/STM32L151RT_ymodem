/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-15     dahong       the first version
 */
#include "board.h"
#include "rtdef.h"
#include "cfg.h"

#define DBG_TAG "cfg"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>


int stm32_eeprom_read(rt_uint32_t addr, rt_uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > FLASH_EEPROM_BASE + FLASH_EEPROM_SIZE)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return -RT_EINVAL;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(rt_uint8_t *) addr;
    }

    return size;
}

int stm32_eeprom_write(rt_uint32_t addr, const rt_uint8_t *buf, size_t size)
{
    rt_err_t result        = RT_EOK;
    rt_uint32_t end_addr   = addr + size;

    if (addr % 4 != 0)
    {
        LOG_E("write addr must be 4-byte alignment");
        return -RT_EINVAL;
    }

    if ((end_addr) > FLASH_EEPROM_BASE + FLASH_EEPROM_SIZE)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return -RT_EINVAL;
    }

    HAL_FLASHEx_DATAEEPROM_Unlock();

    while (addr < end_addr)
    {
        if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, addr, *((rt_uint32_t *)buf)) == HAL_OK)
        {
            if (*(rt_uint32_t *)addr != *(rt_uint32_t *)buf)
            {
                result = -RT_ERROR;
                break;
            }
            addr += 4;
            buf  += 4;
        }
        else
        {
            result = -RT_ERROR;
            break;
        }
    }

    HAL_FLASHEx_DATAEEPROM_Lock();

    if (result != RT_EOK)
    {
        return result;
    }

    return size;
}
