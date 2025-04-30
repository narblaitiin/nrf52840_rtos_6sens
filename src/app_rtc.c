/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_rtc.h"

//  ========== bcd_to_decimal ==============================================================
uint8_t bcd_to_decimal(uint8_t val) {
    return ((val / 16) * 10) + (val % 16);
}

//  ========== decimal_to_bcd ==============================================================
uint8_t decimal_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

//  ========== app_rtc_set_time ============================================================
int8_t app_rtc_set_time(const struct device *i2c_dev, const struct tm *date_time)
{
    uint8_t time_data[7] = {
        decimal_to_bcd(date_time->tm_sec),       // seconds
        decimal_to_bcd(date_time->tm_min),       // minutes
        decimal_to_bcd(date_time->tm_hour),      // hours
        decimal_to_bcd(date_time->tm_wday),      // day of the week (1 = Sunday)
        decimal_to_bcd(date_time->tm_mday),      // day of the month
        decimal_to_bcd(date_time->tm_mon + 1),   // month (1-based)
        decimal_to_bcd(date_time->tm_year - 100) // year (since 2000)
    };

    int ret = i2c_burst_write(i2c_dev, DS3231_I2C_ADDR, 0x00, time_data, 7);
    if (ret < 0) {
        printk("failed to set RTC time. error %d\n", ret);
    }

    printk("RTC time set successfully\n");
    return 0;
}

//  ========== app_rtc_get_time ============================================================
int32_t app_rtc_get_time(const struct device *i2c_dev, struct tm *date_time)
{
    uint8_t rtc_data[7];
    int ret = i2c_burst_read(i2c_dev, DS3231_I2C_ADDR, 0x00, rtc_data, sizeof(rtc_data));
    if (ret < 0) {
        printk("failed to read RTC registers. error: %d", ret);
        return ret;
    }

    date_time->tm_sec = bcd_to_decimal(rtc_data[0]);
    date_time->tm_min = bcd_to_decimal(rtc_data[1]);
    date_time->tm_hour = bcd_to_decimal(rtc_data[2]);
    date_time->tm_wday = bcd_to_decimal(rtc_data[3]);
    date_time->tm_mday = bcd_to_decimal(rtc_data[4]);
    date_time->tm_mon = bcd_to_decimal(rtc_data[5]) - 1; // Months are 0-based
    date_time->tm_year = bcd_to_decimal(rtc_data[6]) + 100; // Years since 1900

    int32_t timestamp = (int32_t)mktime(date_time);
    printk("current date and time: %04d-%02d-%02d %02d:%02d:%02d",
            date_time->tm_year + 1900, date_time->tm_mon + 1, date_time->tm_mday,
            date_time->tm_hour, date_time->tm_min, date_time->tm_sec);
    printk("unix timestamp: %d", timestamp);

    return timestamp;
}

//  ========== app_rtc_init ================================================================
const struct device *app_rtc_init(void)
{
    const struct device *rtc_dev = DEVICE_DT_GET_ONE(maxim_ds3231);
    if (!rtc_dev) {
        printk("no RTC device found");
        return NULL;
    }

    if (!device_is_ready(rtc_dev)) {
        printk("RTC device is not ready");
        return NULL;
    }

    printk("RTC device \"%s\" initialized successfully", rtc_dev->name);
    return rtc_dev;
}