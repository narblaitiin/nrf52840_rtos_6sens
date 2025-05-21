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
static int16_t adc_buffer[ADC_BUFFER_SIZE];
static uint16_t ring_buffer[ADC_BUFFER_SIZE];
static uint32_t sampling_rate_ms = DEFAULT_SAMPLING_RATE_MS;
static bool stop_sampling = false;
static bool adc_initialized = false;

// ADC channel configuration obtained from the device tree
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
static struct adc_sequence sequence0, sequence1;

// define a stack for the ADC thread, with a size of 1024 bytes
K_THREAD_STACK_DEFINE(adc_stack, 1024);

// structure to hold ADC thread data
struct k_thread adc_thread_data;

// mutex to manage concurrent access to the ADC ring buffer
K_MUTEX_DEFINE(buffer_lock);

// mutex to manage access to buffers when retrieving ADC data
K_MUTEX_DEFINE(get_buffer_lock);

// semaphore to signal when new ADC data is available
K_SEM_DEFINE(data_ready_sem, 0, 1);

// define a ring buffer to store ADC samples
static uint16_t ring_buffer[ADC_BUFFER_SIZE];

// index to track the head of the ring buffer
static int ring_head = 0;

// configure ADC sequence
static int8_t configure_adc_sequence(struct adc_sequence *sequence, uint8_t channel_bitmask, void *buffer, size_t buffer_size) {
    sequence->channels = BIT(channel_bitmask);
    sequence->buffer = buffer;
    sequence->buffer_size = buffer_size;
    return adc_sequence_init_dt(&adc_channel, sequence);
}

//  ========== app_nrf52_adc_init ==========================================================
int8_t app_nrf52_adc_init()
{
    if (adc_initialized) {
        printk("ADC is already initialized\n");
        return 0;
    }

   if (!adc_is_ready_dt(&adc_channel)) {
        printk("ADC is not ready. Check hardware configuration.\n");
        return -1;
    }

    int8_t err = adc_channel_setup_dt(&adc_channel);
    if (err < 0) {
        printk("failed to setup ADC channel. Error: %d\n", err);
        return err;
    }

    if (configure_adc_sequence(&sequence0, 0, adc_buffer, sizeof(adc_buffer)) < 0 ||
        configure_adc_sequence(&sequence1, 1, &buffer, sizeof(buffer)) < 0) {
        printk("Failed to configure ADC sequences\n");
        return -1;
    }

    adc_initialized = true;
    printk("ADC initialized successfully\n");

    return 1;
}

//  ========== app_nrf52_get_ain1 ==========================================================
int16_t app_nrf52_get_ain1()
{
    int16_t percent;

    // read sample from the ADC
    if (adc_read(adc_channel.dev, &sequence1) < 0) {        
	    printk("failed to read ADC sequence 1.\n");
	    return 0;
    }

    // convert ADC reading to voltage
    int32_t voltage = (adc_buffer[0] * ADC_REFERENCE_VOLTAGE) / ADC_RESOLUTION;
    printk("convert voltage: %d mV\n", voltage);

    // ensure voltage is within range
    if (voltage > BATTERY_MAX_VOLTAGE) voltage = BATTERY_MAX_VOLTAGE;
    if (voltage < BATTERY_MIN_VOLTAGE) voltage = BATTERY_MIN_VOLTAGE + 1;
    printk("clamped voltage: %d mV\n", voltage);

    // non-linear scaling
    int32_t range = BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE;
    int32_t difference = voltage - BATTERY_MIN_VOLTAGE;

    if (range > 0 && difference > 0) {
        // use power scaling: percentage = ((difference / range) ^ 1.5) * 100
        double normalized = (double)difference / range;  // normalize to range [0, 1]
        double scaled = pow(normalized, 1.2);            // apply non-linear scaling
        percent = (int16_t)(scaled * 100);               // convert to percentage
    } else {
        printk("error: invalid range or difference.\n");
        percent = 0;
    }

    printk("battery level (non-linear, int16): %d%%\n", percent);
    return percent;
}

//  ========== app_thread ==================================================================
static void adc_thread(void *arg1, void *arg2, void *arg3) {
    while (!stop_sampling) {
        if (adc_read(adc_channel.dev, &sequence0) == 0) {
            k_mutex_lock(&buffer_lock, K_FOREVER);
            for (size_t i = 0; i < ADC_BUFFER_SIZE; i++) {
                ring_buffer[(ring_head + i) % ADC_BUFFER_SIZE] = adc_buffer[i];
                printk("ADC buffer values: %d\n", adc_buffer[i]);
            }
            ring_head = (ring_head + ADC_BUFFER_SIZE) % ADC_BUFFER_SIZE;
            k_mutex_unlock(&buffer_lock);
            k_sem_give(&data_ready_sem);
        } else {
            printk("failed to read ADC sequence 0\n");
        }
        k_sleep(K_MSEC(sampling_rate_ms));
    }
}

//  ========== adc_sampling_start and stop  ================================================
// start the ADC sampling thread
// the thread reads data from the ADC and stores it in a ring buffer
void adc_sampling_start(void) {
    stop_sampling = false;
    k_thread_create(&adc_thread_data, adc_stack, K_THREAD_STACK_SIZEOF(adc_stack),
                    adc_thread, NULL, NULL, NULL, 1, 0, K_NO_WAIT);
    printk("ADC sampling thrad started\n");
}

// stop ADC sampling thread
void adc_sampling_stop(void) {
    stop_sampling = true;
    k_thread_join(&adc_thread_data, K_FOREVER);
    printk("ADC sampling thread stopped.\n");
}

//  ========== adc_get_buffer ==============================================================
// copie a portion of the ADC ring buffer to a user-supplied buffer.
// use a mutex to ensure thread-safe access.
void adc_get_buffer(uint16_t *dest, size_t size, int offset) {
    if (!dest || size > ADC_BUFFER_SIZE || offset < 0 || offset >= ADC_BUFFER_SIZE) {
        printk("Invalid parameters in adc_get_buffer.\n");
        return;
    }

    k_mutex_lock(&buffer_lock, K_FOREVER);
    for (size_t i = 0; i < size; i++) {
        dest[i] = ring_buffer[(ring_head + offset + i) % ADC_BUFFER_SIZE];
    }
    k_mutex_unlock(&buffer_lock);
}

// set ADC sampling rate
void set_sampling_rate(uint32_t rate_ms) {
    sampling_rate_ms = rate_ms;
    printk("Sampling rate set to %d ms.\n", rate_ms);
}