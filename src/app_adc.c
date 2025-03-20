/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_adc.h"

//  ========== globals =====================================================================
int16_t buf;

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

struct adc_sequence sequence_ain0 = {
        .channels = 0,
		.buffer = &buf,
		.buffer_size = sizeof(buf), // in bytes
};

struct adc_sequence sequence_ain1 = {
    .channels = 1,
    .buffer = &buf,
    .buffer_size = sizeof(buf), // in bytes
};

//  ========== app_nrf52_adc_init ==========================================================
int8_t app_nrf52_adc_init()
{
    int8_t err;

    if (!adc_is_ready_dt(&adc_channel)) {
		printk("ADC is not ready. error: %d\n", err);
		return 0;
        printk("- found device \"%s\", getting sensor data\n", adc_channel.dev->name);
    }

    // setup ADC channel
    err = adc_channel_setup_dt(&adc_channel);
	if (err < 0) {
		printk("error: %d. could not setup channel\n", err);
		return 0;
	}

    // initializing ADC sequence
    err = adc_sequence_init_dt(&adc_channel, &sequence_ain0);
	if (err < 0) {
		printk("error: %d. could not initalize sequnce\n", err);
		return 0;
	}

    err = adc_sequence_init_dt(&adc_channel, &sequence_ain1);
	if (err < 0) {
		printk("error: %d. could not initalize sequnce\n", err);
		return 0;
	}
    return 0;
}

//  ========== app_nrf52_get_ain0 ==========================================================
int16_t app_nrf52_get_ain0()
{
    int16_t val_mv;
    int8_t ret;

    val_mv = (int)buf;
    // reading sample from the ADC
    ret = adc_read(adc_channel.dev, &sequence_ain0);
    if (ret < 0 ) {        
	    printk("sensor sample is not up to date. error: %d\n", ret);
	    return 0;
    }

    // geophone sensor level received and converted from channel get
    // resolution 12bits: 0 to 4095 (uint16)
    ret = adc_raw_to_millivolts_dt(&adc_channel, &val_mv);
    if (ret < 0) {
			printk("value in mV not available. error: %d\n", ret);
		} else {
			printk("adc: %d mV\n", val_mv);
		}
    return val_mv;
    }

//  ========== app_nrf52_get_ain1 ======================================================
int16_t app_nrf52_get_ain1()
{
    float percent = 0;
    int8_t ret = 0;

    // reading sample from the ADC
    ret = adc_read(adc_channel.dev, &sequence_ain1);
    if (ret < 0 ) {        
	    printk("nrf52 vbat sample is not up to date. error: %d\n", ret);
	    return 0;
    }

    // battery level received and converted from channel get
    // resolution 12bits: 0 to 4095 (uint16)
    ret = (adc_raw_to_millivolts_dt(&adc_channel, &buf))/1000;
    if (ret < 0) {
			printk("value in mV not available. error: %d\n", ret);
		} else {
			printk("vbat: %d mV\n", buf);
		}
    // quadratic curve fit of lipo voltage measurement
    percent= (-2.281*pow(10,2)*buf) + (2.066*pow(10,3)*buf) - (4.57*pow(10,3)*buf);
    return percent;
}
