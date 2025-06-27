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
    const struct device *rtc_dev = DEVICE_DT_GET(DT_NODELABEL(rtc0));
    if (!device_is_ready(rtc_dev)) {
        printk("RTC device is not ready\n");
        return NULL;
    }

    // start the counter
    int ret = counter_start(rtc_dev);
    if (ret < 0) {
        printk("failed to start RTC: %d\n", ret);
        return NULL;
    }

    // initialize mute
    k_mutex_init(&offset_mutex);

    printk("RTC initialized and started successfully\n");
    return rtc_dev;
}

//  ========== app_rtc_sync_uptime ========================================================
// synchronize the system uptime with the on board RTC.
int8_t app_rtc_sync_uptime(const struct device *rtc_dev)
{
    if (!rtc_dev) {
        printk("RTC device is NULL\n");
        return -EINVAL;
    }

    uint32_t rtc_ticks;
    uint32_t top_value;
    uint32_t frequency;
    int64_t rtc_time_ms;
    int64_t current_uptime_ms;
    int64_t new_offset_ms;

    // retrieve the current RTC counter value
    int ret = counter_get_value(rtc_dev, &rtc_ticks);
    if (ret < 0) {
        printk("failed to get RTC counter value, error: %d\n", ret);
        return ret;
    }

    // get the counter's top value and frequency
    top_value = counter_get_top_value(rtc_dev);
    frequency = counter_get_frequency(rtc_dev);

    // convert the RTC ticks to milliseconds
    rtc_time_ms = ((int64_t)rtc_ticks * 1000) / frequency;

    // debug output
    printk("RTC ticks: %u, top: %u, freq: %u Hz, time_ms: %lld\n",
           rtc_ticks, top_value, frequency, rtc_time_ms);

    // get the current system uptime in milliseconds
    current_uptime_ms = k_uptime_get();

    // calculate the offset between RTC time and system uptime
    new_offset_ms = rtc_time_ms - current_uptime_ms;

    // validate the offset (example: restrict offset to ±1 year for sanity)
    if (new_offset_ms < -ONE_YEAR_MS|| new_offset_ms > ONE_YEAR_MS) {
        printk("offset out of range! calculation error\n");
        return -EINVAL;
    }

    // safely update the global offset
    k_mutex_lock(&offset_mutex, K_FOREVER);
    system_rtc_offset_ms = new_offset_ms;
    k_mutex_unlock(&offset_mutex);

    // debugging output
    printk("calculated offset (ms): %lld\n", system_rtc_offset_ms);

    return 0;
}

//  ========== app_rtc_get_time ==========================================================
uint64_t app_rtc_get_time(const struct device *rtc_dev)
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
    if (!rtc_dev) {
        printk("RTC device is NULL\n");
        return -EINVAL;
    }
    
    // call this periodically from a thread or workqueue
    int ret = app_rtc_sync_uptime(rtc_dev);
    if (ret < 0) {
        printk("periodic sync failed, error: %d", ret);
    }
    return 0;
}