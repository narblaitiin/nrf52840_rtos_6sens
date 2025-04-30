/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_sht31.h"

//  ========== app_sht31_get_temp ==========================================================
int16_t app_sht31_get_temp(const struct device *dev)
{
    struct sensor_value val;
    int32_t temp = 0;
    int8_t ret = 0;

    // get sensor device
	dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);

    // fetch the latest sample from the sensor
	ret = sensor_sample_fetch(dev);
    if (ret < 0 && ret != -EBADMSG) { 
	    printk("SHT31 device sample is not up to date. error: %d\n", ret);
	    return 0;
    }

    // retrieve the ambient temperature from the sensor
	ret = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &val);
    if (ret < 0) {
        printk("can't read sensor channels. error: %d\n", ret);
	    return 0;
    }

    // convert the raw temperature value to integer format with scaling
    temp = (val.val1 * TEMP_SCALE) + ((int64_t)val.val2 * TEMP_SCALE / 1000000);

    // print the temperature value with two decimal places
    // printk("SHT31 temperature: %d.%02d °C\n", temp / TEMP_SCALE, temp % TEMP_SCALE);
    
    return (int16_t)temp;
}

//  ========== app_sht31_get_hum ===========================================================
int16_t app_sht31_get_hum(const struct device *dev)
{
    struct sensor_value val;
    int32_t hum = 0;
    int8_t ret = 0;


    // get sensor device
	dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);

    // fetch the latest sample from the sensor
	ret = sensor_sample_fetch(dev);
    if (ret < 0 && ret != -EBADMSG) { 
	    printk("sht31 device sample is not up to date. error: %d\n", ret);
	    return 0;
    }

    // retrieve the humidity from the sensor
	ret = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &val);
    if (ret < 0) {
        printk("can't read sensor channels. error: %d\n", ret);
	    return 0;
    }

    // convert the raw humicity value to integer format with scaling
    hum = (val.val1 * HUM_SCALE) + ((int64_t)val.val2 * HUM_SCALE / 1000000);

    // print the humifity value with two decimal places
    // printk("sht31 humidity: %d.%02d %%RH\n", hum / HUM_SCALE, hum % HUM_SCALE);
    
    return (int16_t)hum;
}