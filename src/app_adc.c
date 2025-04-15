/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_adc.h"

//  ========== globals =====================================================================
// ADC buffer to store raw ADC readings
int16_t buf0, buf1;

// ADC channel configuration obtained from the device tree
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

// ADC sequence configuration to specify the ADC operation
struct adc_sequence sequence0 = {
        .channels = 0,
		.buffer = &buf0,
		.buffer_size = sizeof(buf0),
};

// ADC sequence configuration to specify the ADC operation
struct adc_sequence sequence1 = {
    .channels = 1,
    .buffer = &buf1,
    .buffer_size = sizeof(buf1),
};

//  ========== app_nrf52_adc_init ==========================================================
int8_t app_nrf52_adc_init()
{
    int8_t err;

    // verify if the ADC is ready for operation
    if (!adc_is_ready_dt(&adc_channel)) {
		printk("ADC is not ready. error: %d\n", err);
		return 0;
        printk("- found device \"%s\", getting sensor data\n", adc_channel.dev->name);
    }

    // configure the ADC channel settings
    err = adc_channel_setup_dt(&adc_channel);
	if (err < 0) {
		printk("error: %d. could not setup channel\n", err);
		return 0;
	}

     // initialize the ADC sequence for continuous or single readings (ain0)
    err = adc_sequence_init_dt(&adc_channel, &sequence0);
	if (err < 0) {
		printk("error: %d. could not initalize sequnce\n", err);
		return 0;
	}

    // initialize the ADC sequence for continuous or single readings (ain1)
    err = adc_sequence_init_dt(&adc_channel, &sequence1);
	if (err < 0) {
		printk("error: %d. could not initalize sequnce\n", err);
		return 0;
	}
    return 0;
}

//  ========== app_nrf52_get_ain0 ==========================================================
int16_t app_nrf52_get_ain0()
{
    int16_t velocity;
    int8_t ret;
    
    // read sample from the ADC
    ret = adc_read(adc_channel.dev, &sequence0);
    if (ret < 0) {        
	    printk("raw adc value is not up to date. error: %d\n", ret);
	    return 0;
    }

    printk("raw adc value: %d\n", buf0);

    // convert ADC reading to voltage
    velocity = (buf0 * ADC_REFERENCE_VOLTAGE) / ADC_RESOLUTION;
    printk("velocity: %d mV\n", velocity);

    return velocity;
    }

//  ========== app_nrf52_get_ain1 ======================================================
int16_t app_nrf52_get_ain1()
{
    int16_t percent = 0;
    int8_t ret = 0;
    int32_t voltage = 0;        // variable to store converted ADC value

    // read sample from the ADC
    ret = adc_read(adc_channel.dev, &sequence1);
    if (ret < 0 ) {        
	    printk("raw adc valueis not up to date. error: %d\n", ret);
	    return 0;
    }
//    printk("raw adc value: %d\n", buf1);

    // convert ADC reading to voltage
    voltage = (buf1 * ADC_REFERENCE_VOLTAGE) / ADC_RESOLUTION;
//    printk("convert voltage: %d mV\n", voltage);

    // ensure voltage is within range
    if (voltage > BATTERY_MAX_VOLTAGE) voltage = BATTERY_MAX_VOLTAGE;
    if (voltage < BATTERY_MIN_VOLTAGE) voltage = BATTERY_MIN_VOLTAGE + 1;
//    printk("clamped voltage: %d mV\n", voltage);

    // non-linear scaling
    int32_t range = BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE;
    int32_t difference = voltage - BATTERY_MIN_VOLTAGE;

    if (range > 0 && difference > 0) {
        // use power scaling: percentage = ((difference / range) ^ 1.5) * 100
        double normalized = (double)difference / range;  // normalize to range [0, 1]
        double scaled = pow(normalized, 1.5);            // apply non-linear scaling
        percent = (int16_t)(scaled * 100);               // convert to percentage
    } else {
        printk("error: invalid range or difference.\n");
        percent = 0;
    }

//    printk("battery level (non-linear, int16): %d%%\n", percent);
    return percent;
}
