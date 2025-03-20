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
int* app_rtc_get_time (const struct device *dev)
{
    int8_t ret = 0;
	struct rtc_time time = {
		.tm_year = 2024 - 1900,
		.tm_mon = 11 - 1,
		.tm_mday = 17,
		.tm_hour = 4,
		.tm_min = 19,
		.tm_sec = 0,
	};
	int buffer[sizeof(time)];

	ret = rtc_set_time(dev, &time);
	if (ret < 0) {
		printk("Cannot write date time: %d\n", ret);
		return ret;
	}
	
    ret = rtc_get_time(dev, &time);
	if (ret < 0) {
		printk("cannot read date time: %d\n", ret);
		return ret;
	}

	// 	// putting timestamp in fisrt page for this test
 	memcpy(buffer, &time, sizeof(time));
	for (int8_t itr = 0; itr < sizeof(buffer); itr) {
		printk("RTC date and time: %04d-%02d-%02d %02d:%02d:%02d\n", buffer[itr]);
	}

	// printk("RTC date and time: %04d-%02d-%02d %02d:%02d:%02d\n", time.tm_year + 1900,
	//        time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
    return buffer;
}