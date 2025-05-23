/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_RTC_H
#define APP_RTC_H

//  ========== includes ====================================================================
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/drivers/counter.h>
#include <time.h>
 
//  ========== defines =====================================================================
#define DS3231_I2C_ADDR 0x68
 
//  ========== prototypes ==================================================================
const struct device *app_rtc_init(void);
int8_t app_rtc_set_time(const struct device *i2c_dev, const struct tm *date_time);
int32_t app_rtc_get_time(const struct device *i2c_dev, struct tm *date_time);
uint64_t get_high_res_timestamp();
uint8_t bcd_to_decimal(uint8_t val);
uint8_t decimal_to_bcd(uint8_t val);
 
#endif /* APP_RTC_H */