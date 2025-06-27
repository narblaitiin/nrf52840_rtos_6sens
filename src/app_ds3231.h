/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_DS3231_H
#define APP_DS3231_H

//  ========== includes ====================================================================
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/counter.h>
#include <time.h>

//  ========== defines =====================================================================
#define ONE_YEAR_MS         31536000000LL
#define DS3231_I2C_ADDR     0x68

//  ========== prototypes ==================================================================
const struct device *app_ds3231_init(void);
int8_t  app_ds3231_sync_uptime(const struct device *i2c_dev);
uint64_t app_ds3231_get_time();
int8_t app_ds3231_periodic_sync(const struct device *rtc_dev);

#endif /* APP_DS3231_H */