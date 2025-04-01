/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_rtc.h"

//  ========== app_rtc_init ================================================================
int8_t app_rtc_init(const struct device *dev)
{
    int8_t ret = 0;
    
    // getting ds3231 rtc i2c device (SDA: P0.09, SCL:P0.0)
    dev = DEVICE_DT_GET_ONE(maxim_ds3231);
    if (dev == 0) {
        printk("error: no =rtc device found\n");
		return 0;
	}

    if (!device_is_ready(dev)) {
		printk("error: rtc is not ready\n");
		return 0;
	} else {
        printk("- found device \"%s\", getting rtc data\n", dev->name);
    }
    return 0;
}

//  ========== app_rtc_get_time ============================================================
int32_t app_rtc_get_time (const struct device *dev)
{
    int8_t ret = 0;
	int32_t timestamp = 0;
	struct rtc_time time = {0};

	// convert rtc_time to Unix timestamp (seconds since 1970)
    struct tm t = {
        .tm_year = time.tm_year,
        .tm_mon  = time.tm_mon,
        .tm_mday = time.tm_mday,
        .tm_hour = time.tm_hour,
        .tm_min  = time.tm_min,
        .tm_sec  = time.tm_sec
    };

    ret = rtc_get_time(dev, &time);
	if (ret < 0) {
		printk("cannot read date time: %d\n", ret);
		return ret;
	}

	// Convert to Unix timestamp
	timestamp = (int32_t)mktime(&t);
	printk("timestamp in unix: %d\n", timestamp);

    return timestamp;
}