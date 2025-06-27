/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_lorawan.h"
#include "app_adc.h"
#include "app_rtc.h"

//  ========== defines =====================================================================
#define TIMESTAMP_SIZE 8
#define ADC_BUFFER_OFFSET TIMESTAMP_SIZE

//  ========== globals =====================================================================
// define a stack for the LoRaWAN thread with a size of 1024 bytes
K_THREAD_STACK_DEFINE(lorawan_stack, 1024);

// declare a thread structure to manage the LoRaWAN thread's data
struct k_thread lorawan_thread_data;

//  ========== serialize_uint64_to_bytes ===================================================
static void serialize_uint64_to_bytes(uint64_t value, uint8_t *buffer)
{
    for (int i = 0; i < 8; i++) {
        buffer[i] = (value >> (56 - 8 * i)) & 0xFF;
    }
}

//  ========== app_lorawan_thread ==========================================================
// LoRaWAN thread function: handles ADC data acquisition and sends it over LoRaWAN
static void app_lorawan_thread(void *arg1, void *arg2, void *arg3) 
{
    // allocate a buffer for ADC data with additional space for the timestamp
    uint8_t data[ADC_BUFFER_SIZE * sizeof(uint16_t) + 8];

    // initialize DS3231 RTC device via I2C (Pins: SDA -> P0.09, SCL -> P0.0)
    const struct device *rtc = DEVICE_DT_GET_ONE(maxim_ds3231);

    while (1) {
        // retrieve the current timestamp
        uint64_t timestamp = app_rtc_get_time();
        //uint64_t timestamp = app_ds3231_get_time();
        
        // serialize timestamp into the data buffer
        serialize_uint64_to_bytes(timestamp, data);

        // suspend the thread, effectively pausing it until explicitly resumed
        k_thread_suspend(k_current_get());
        printk("sending timestamp and velocity...\n");

        // acquire ADC data into the buffer
        app_adc_get_buffer((uint16_t *)&data[ADC_BUFFER_OFFSET], ADC_BUFFER_SIZE, 0);

        // transmit the data over LoRaWAN
        lorawan_send(LORAWAN_PORT, data, sizeof(data), LORAWAN_MSG_UNCONFIRMED);
        printk("data sent to LoRaWAN\n");
    }
}

//  ========== app_lorawan_trigger_tx ======================================================
// function to trigger the LoRaWAN thread to resume execution.
void app_lorawan_trigger_tx(void)
{
    // resume the suspended LoRaWAN thread, allowing it to execute.
    k_thread_resume(&lorawan_thread_data);
}

//  ========== app_lorawan_start ===========================================================
// function to initialize and start the LoRaWAN transmission thread
int app_lorawan_start_tx(void)
{
    // create the LoRaWAN thread with the defined stack and function
    k_thread_create(&lorawan_thread_data, lorawan_stack, K_THREAD_STACK_SIZEOF(lorawan_stack),
                    app_lorawan_thread, NULL, NULL, NULL, 3, 0, K_NO_WAIT);
}