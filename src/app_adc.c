/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_adc.h"

//  ========== globals =====================================================================
// ADC buffer to store raw ADC readings
static int16_t buffer;
static uint16_t adc_buffer[ADC_BUFFER_SIZE];

// ADC channel configuration obtained from the device tree
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

// ADC sequence configuration to specify the ADC operation
static struct adc_sequence sequence1 = {
    .channels = BIT(1),
    .buffer = &buffer,
    .buffer_size = sizeof(buffer),
};

static struct adc_sequence sequence0 = {
    .channels    = BIT(0),
    .buffer      = adc_buffer,
    .buffer_size = sizeof(adc_buffer),
};

K_THREAD_STACK_DEFINE(adc_stack, 1024);
struct k_thread adc_thread_data;
K_MUTEX_DEFINE(buffer_lock);
K_MUTEX_DEFINE(get_buffer_lock);
K_SEM_DEFINE(data_ready_sem, 0, 1);

static uint16_t ring_buffer[ADC_BUFFER_SIZE];
static int ring_head = 0;

//  ========== app_nrf52_adc_init ==========================================================
int8_t app_nrf52_adc_init()
{
    int8_t err;

    // verify if the ADC is ready for operation
    if (!adc_is_ready_dt(&adc_channel)) {
		printk("ADC is not ready. error: %d\n", err);
		return 0;
	} else {
        printk("- found device \"%s\"\n", adc_channel.dev->name);
    }

    // configure the ADC channel settings
    err = adc_channel_setup_dt(&adc_channel);
	if (err < 0) {
		printk("error: %d. could not setup channel\n", err);
		return 0;
	}

    // initialize the ADC sequence for continuous or single readings (ain1)
    err = adc_sequence_init_dt(&adc_channel, &sequence1);
	if (err < 0) {
		printk("error: %d. could not initalize sequnce\n", err);
		return 0;
	}

    // initialize the ADC sequence for continuous or single readings (ain0)
    err = adc_sequence_init_dt(&adc_channel, &sequence0);
	if (err < 0) {
		printk("error: %d. could not initalize sequnce\n", err);
		return 0;
	}
    return 0;
}

//  ========== app_nrf52_get_ain1 ==========================================================
int16_t app_nrf52_get_ain1()
{
    int16_t percent = 0;
    int8_t ret = 0;
    int32_t voltage = 0;        // variable to store converted ADC value

    // read sample from the ADC
    ret = adc_read(adc_channel.dev, &sequence1);
    if (ret < 0 ) {        
	    printk("raw adc values is not up to date. error: %d\n", ret);
	    return 0;
    }
//    printk("raw adc value: %d\n", buf1);

    // convert ADC reading to voltage
    voltage = (buffer * ADC_REFERENCE_VOLTAGE) / ADC_RESOLUTION;
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

//  ========== app_thread ==================================================================
static void adc_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        int ret = adc_read(adc_channel.dev, &sequence0);
        if (ret == 0) {
            k_mutex_lock(&buffer_lock, K_FOREVER);
            for (size_t i = 0; i < ADC_BUFFER_SIZE; i++) {
                ring_buffer[ring_head] = adc_buffer[i];
                ring_head = (ring_head + 1) % ADC_BUFFER_SIZE;
            }
            k_mutex_unlock(&buffer_lock);
            k_sem_give(&data_ready_sem);
        }
        k_sleep(K_MSEC(1)); // Adjust for sampling rate
    }
}

//  ========== adc_sampling_start ==========================================================
void adc_sampling_start(void) {
    k_thread_create(&adc_thread_data, adc_stack, K_THREAD_STACK_SIZEOF(adc_stack),
                    adc_thread, NULL, NULL, NULL, 1, 0, K_NO_WAIT);
}

//  ========== adc_get_buffer ==============================================================
void adc_get_buffer(uint16_t *dest, size_t size, int offset) {
    k_mutex_lock(&buffer_lock, K_FOREVER);
    int start_idx = (ring_head + offset + ADC_BUFFER_SIZE) % ADC_BUFFER_SIZE;

    for (size_t i = 0; i < size; i++) {
        dest[i] = ring_buffer[(start_idx + i) % ADC_BUFFER_SIZE];
    }

    k_mutex_unlock(&buffer_lock);
}
