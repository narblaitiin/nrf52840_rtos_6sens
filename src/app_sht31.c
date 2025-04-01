/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_sht31.h"

//  ========== app_sht31_init ==============================================================
int8_t app_sht31_init(const struct device *dev)
{
    // getting sht31 sensor i2c device (SDA: P0.09, SCL:P0.0)
    dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);
    if (dev == NULL) {
        printk("error: no sht31 device found\n");
		return 0;
	}

    if (!device_is_ready(dev)) {
		printk("error: sht31 is not ready\n");
		return 0;
	} else {
        printk("- found device \"%s\", getting sht31 data\n", dev->name);
    }
    return 0;
}

//  ========== app_sht31_get_temp ==========================================================
int16_t app_sht31_get_temp(const struct device *dev)
{
    struct sensor_value val;
    int32_t temp = 0;
    int8_t ret = 0;
    int8_t scale = 100;

    // fetching data
	ret = sensor_sample_fetch(dev);
    if (ret < 0 && ret != -EBADMSG) { 
	    printk("SHT31 device sample is not up to date. error: %d\n", ret);
	    return 0;
    }

    // getting channel function
	ret = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &val);
    if (ret < 0) {
        printk("can't read sensor channels. error: %d\n", ret);
	    return 0;
    }

    // convert struct to int16
    temp = (val.val1 * scale) + (val.val2 / (1000000)/scale);
    printk("sht31 temp (int16): %d degree\n", temp);
    return (int16_t)temp;
}

//  ========== app_sht31_get_hum ===========================================================
int16_t app_sht31_get_hum(const struct device *dev)
{
    struct sensor_value val;
    float hum = 0;
    int8_t ret = 0;
    int8_t scale = 100;

    // fetching data
	ret = sensor_sample_fetch(dev);
    if (ret < 0 && ret != -EBADMSG) { 
	    printk("sht31 device sample is not up to date. error: %d\n", ret);
	    return 0;
    }

    // getting channel function
	ret = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &val);
    if (ret < 0) {
        printk("can't read sensor channels. error: %d\n", ret);
	    return 0;
    }

    // convert struct to int16
    hum = (val.val1 * scale) + (val.val2 / (1000000)/scale);
    printk("sht31 humidity (int16): %d%%RH\n", hum);
    return (int16_t)hum;
}