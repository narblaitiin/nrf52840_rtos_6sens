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
    // getting sht31 sensor i2c device (SDA: P0.24, SCL:P0.25)
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

    // fetch data
	ret = sensor_sample_fetch(dev);
    if (ret < 0 && ret != -EBADMSG) { 
	    printk("sht31 device sample is not up to date. error: %d\n", ret);
	    return 0;
    }

    // get channel function
	ret = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &val);
    if (ret < 0) {
        printk("can't read sensor channels. error: %d\n", ret);
	    return 0;
    }

     // convert temperature to int16_t with scaling
    temp = (val.val1 * TEMP_SCALE) + ((int64_t)val.val2 * TEMP_SCALE / 1000000);
//    printk("sht31 temperature: %d.%02d °C\n", temp / TEMP_SCALE, temp % TEMP_SCALE);
    
    return (int16_t)temp;
}

//  ========== app_sht31_get_hum ===========================================================
int16_t app_sht31_get_hum(const struct device *dev)
{
    struct sensor_value val;
    int32_t hum = 0;
    int8_t ret = 0;

       // fetch data
	ret = sensor_sample_fetch(dev);
    if (ret < 0 && ret != -EBADMSG) { 
	    printk("sht31 device sample is not up to date. error: %d\n", ret);
	    return 0;
    }

    // get channel function
	ret = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &val);
    if (ret < 0) {
        printk("can't read sensor channels. error: %d\n", ret);
	    return 0;
    }

    // convert struct to int16
    hum = (val.val1 * HUM_SCALE) + ((int64_t)val.val2 * HUM_SCALE / 1000000);
//    printk("sht31 humidity: %d.%02d %%RH\n", hum / HUM_SCALE, hum % HUM_SCALE);
    
    return (int16_t)hum;
}