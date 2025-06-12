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

    printk("RTC device \"%s\" initialized successfully\n", rtc_dev->name);
    return rtc_dev;
}

//  ========== sync_uptime_with_rtc ================================================================
// synchronize the system uptime with the DS3231 RTC.
int8_t  sync_uptime_with_rtc(const struct device *i2c_dev)
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
        printk("invalid RTC time\n");
    }

    // calculate current system uptime in milliseconds
    int64_t current_uptime_ms = k_uptime_get();

    // debugging logs for validation
    printk("RTC unix seconds: %lld\n", rtc_unix_seconds);
    printk("system uptime (ms): %lld\n", current_uptime_ms);

    // calculate the offset between the RTC and the system clock
    system_rtc_offset_ms = (rtc_unix_seconds * 1000LL) - current_uptime_ms;

    // debugging offset
    printk("calculated offset (ms): %lld\n", system_rtc_offset_ms);

    if (system_rtc_offset_ms < -31536000000LL || system_rtc_offset_ms > 31536000000LL) {
        printk("offset out of range! calculation error\n");
        system_rtc_offset_ms = 0; // reset offset to prevent incorrect timestamps
    }
}

//  ========== app_rtc_get_time ============================================================
uint64_t app_rtc_get_time()
{
    // get the current system uptime in milliseconds
    int64_t current_uptime_ms = k_uptime_get();

    // check for overflow or underflow
    if (system_rtc_offset_ms > 0 && 
        current_uptime_ms > UINT64_MAX - system_rtc_offset_ms) {
        printk("overflow detected in timestamp calculation\n");
        return UINT64_MAX;
    }
    if (system_rtc_offset_ms < 0 && 
        current_uptime_ms < (uint64_t)(-system_rtc_offset_ms)) {
        printk("underflow detected in timestamp calculation\n");
        return 0;
    }

    // adjust the system uptime using the RTC offset
    uint64_t millisecond_timestamp = current_uptime_ms + system_rtc_offset_ms;

    return millisecond_timestamp;
}