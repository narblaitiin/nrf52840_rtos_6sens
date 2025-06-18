/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_rtc.h"

//  ========== globals ====================================================================
// global variable to track the offset between the system clock and RTC
static int64_t system_rtc_offset_ms = 0;
static struct k_mutex offset_mutex;

//  ========== app_rtc_init ================================================================
const struct device *app_rtc_init(void)
{
    const struct device *rtc_dev = DEVICE_DT_GET_ONE(maxim_ds3231);
    if (!rtc_dev) {
        printk("no RTC device found\n");
        return NULL;
    }

    if (!device_is_ready(rtc_dev)) {
        printk("RTC device is not ready\n");
        return NULL;
    }

    // initialize mute
    k_mutex_init(&offset_mutex);

    printk("RTC device \"%s\" initialized successfully\n", rtc_dev->name);
    return rtc_dev;
}

//  ========== app_rtc_sync_uptime ========================================================
// synchronize the system uptime with the DS3231 RTC.
int8_t  app_rtc_sync_uptime(const struct device *i2c_dev)
{
    struct tm rtc_time;

    // read RTC time
    int8_t ret = rtc_get_time(i2c_dev, &rtc_time);
    if (ret < 0) {
        printk("failed to get time from RTC, %d\n", ret);
    }

    printk("RTC time: year=%d, month=%d, day=%d, hour=%d, minute=%d, second=%d\n",
       rtc_time.tm_year, rtc_time.tm_mon, rtc_time.tm_mday,
       rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);

    // convert RTC time to Unix seconds
    int64_t rtc_unix_seconds = mktime(&rtc_time);
    if (rtc_unix_seconds > 0) {
        printk("RTC time: %lld seconds\n", rtc_unix_seconds);
    } else {
        printk("invalid RTC time conversion\n");
    }

    // calculate current system uptime in milliseconds
    int64_t current_uptime_ms = k_uptime_get();

    // debugging logs for validation
    printk("RTC unix seconds: %lld\n", rtc_unix_seconds);
    printk("system uptime (ms): %lld\n", current_uptime_ms);

    // calculate the offset between the RTC and the system clock
    int64_t new_offset_ms = (rtc_unix_seconds * 1000LL) - current_uptime_ms;

    if (new_offset_ms< -31536000000LL || new_offset_ms > 31536000000LL) {
        printk("offset out of range! calculation error\n");
    }

    // update global offset safely
    k_mutex_lock(&offset_mutex, K_FOREVER);
    system_rtc_offset_ms = new_offset_ms;
    k_mutex_unlock(&offset_mutex);

    // debugging offset
    printk("calculated offset (ms): %lld\n", system_rtc_offset_ms);
    return 0;
}

//  ========== app_rtc_get_time ==========================================================
uint64_t app_rtc_get_time(void)
{
    int64_t current_uptime_ms = k_uptime_get();
    int64_t offset_ms;

    // safely read the offset
    k_mutex_lock(&offset_mutex, K_FOREVER);
    offset_ms = system_rtc_offset_ms;
    k_mutex_unlock(&offset_mutex);

    // check for overflow or underflow
    if (offset_ms > 0 &&
        current_uptime_ms > UINT64_MAX - offset_ms) {
        printk("overflow detected in timestamp calculation\n");
        return UINT64_MAX;
    }
    if (offset_ms < 0 &&
        current_uptime_ms < (uint64_t)(-offset_ms)) {
        printk("underflow detected in timestamp calculation\n");
        return 0;
    }
    uint64_t timestamp_ms = current_uptime_ms + offset_ms;
    return timestamp_ms;
}

//  ========== app_rtc_periodic_sync====================================================
int8_t app_rtc_periodic_sync(const struct device *rtc_dev)
{
    // example: Call this periodically from a thread or workqueue
    int ret = app_rtc_sync_uptime(rtc_dev);
    if (ret < 0) {
        printk("periodic sync failed, error: %d", ret);
    }
    return 0;
}