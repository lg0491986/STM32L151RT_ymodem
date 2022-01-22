/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-15     dahong       the first version
 */
#ifndef APPLICATIONS_CFG_H_
#define APPLICATIONS_CFG_H_
#include "main.h"

struct cfg_ota{
    uint32_t    addr_app0;
    uint32_t    addr_app1;
    uint8_t     type; /* 0:app0 1:app1 */
};

int stm32_eeprom_read(rt_uint32_t addr, rt_uint8_t *buf, size_t size);
int stm32_eeprom_write(rt_uint32_t addr, const rt_uint8_t *buf, size_t size);

static inline int write_cfg(rt_uint32_t addr, const rt_uint8_t *buf, size_t size)
{
    return stm32_eeprom_write(addr, buf, size);
}

static inline int read_cfg(rt_uint32_t addr, rt_uint8_t *buf, size_t size)
{
    return stm32_eeprom_read(addr, buf, size);
}

#endif /* APPLICATIONS_CFG_H_ */
