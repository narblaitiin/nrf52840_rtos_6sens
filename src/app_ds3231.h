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
#include <zephyr/drivers/rtc.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys_clock.h>
#include <time.h>

//  ========== defines =====================================================================
#define ONE_YEAR_MS         31536000000LL
#define DS3231_I2C_ADDR     0x68
#define DS3231_REG_TIME     0x00

//  ========== prototypes ===================================================================
const struct device *app_ds3231_init(void);
int8_t  app_ds3231_sync_uptime(const struct device *i2c_dev);
uint64_t app_ds3231_get_time();
int8_t app_ds3231_periodic_sync(const struct device *i2c_dev);
int8_t ds3231_set_time(const struct device *i2c_dev, const struct tm *tm);
int8_t ds3231_get_time(const struct device *i2c_dev, struct tm *tm);
void set_ds3231_from_unix_time(const struct device *rtc_dev, int64_t unix_time);

#endif /* APP_DS3231_H */