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

//  ========== globals =====================================================================
// define a stack for the LoRaWAN thread with a size of 1024 bytes
K_THREAD_STACK_DEFINE(lorawan_stack, 1024);

// declare a thread structure to manage the LoRaWAN thread's data
struct k_thread lorawan_thread_data;

//  ========== lorawan_thread ==============================================================
// LoRaWAN thread function: handles ADC data acquisition and sends it over LoRaWAN
static void lorawan_thread(void *arg1, void *arg2, void *arg3) {
    // allocate a buffer for ADC data, sized to h56old ADC_BUFFER_SIZE samples of 16-bit values
    uint8_t data[ADC_BUFFER_SIZE * sizeof(uint16_t) + 8];

    // initialize DS3231 RTC device via I2C (Pins: SDA -> P0.09, SCL -> P0.0)
    const struct device *rtc = DEVICE_DT_GET_ONE(maxim_ds3231);

    // retrieve the current timestamp from the RTC device 
    struct tm current_time;  
    int32_t timestamp = app_rtc_get_time(rtc, &current_time);
    //uint64_t timestamp = get_high_res_timestamp();

    // add the timestamp to the start of the data buffer
    // data[0] = (timestamp >> 56) & 0xFF; // most significant byte
    // data[1] = (timestamp >> 48) & 0xFF;
    // data[2] = (timestamp >> 40) & 0xFF;
    // data[3] = (timestamp >> 32) & 0xFF;
    // data[4] = (timestamp >> 24) & 0xFF;
    // data[5] = (timestamp >> 16) & 0xFF;
    // data[6] = (timestamp >> 8) & 0xFF;
    // data[7] = timestamp & 0xFF;        // least significant byte

    // add the timestamp to the start of the data buffer
    data[0] = (timestamp >> 24) & 0xFF; // most significant byte
    data[1] = (timestamp >> 16) & 0xFF;
    data[2] = (timestamp >> 8) & 0xFF;
    data[3] = timestamp & 0xFF;        // least significant byte

    while (1) {
        // suspend the thread, effectively pausing it until explicitly resumed
        k_thread_suspend(k_current_get());
        printk("sending timestamp and velocity...\n");
        // acquire ADC data into the buffer
        adc_get_buffer((uint16_t *)&data[4], ADC_BUFFER_SIZE, 0);

        // transmit the data over LoRaWAN
        lorawan_send(LORAWAN_PORT, data, sizeof(data), LORAWAN_MSG_UNCONFIRMED);
        printk("data sent to LoRaWAN\n");
    }
}

//  ========== lorawan_trigger_transmission ================================================
// function to trigger the LoRaWAN thread to resume execution.
void lorawan_trigger_transmission(void) {
    // resume the suspended LoRaWAN thread, allowing it to execute.
    k_thread_resume(&lorawan_thread_data);
}

//  ========== lorawan_start ===============================================================
// function to initialize and start the LoRaWAN transmission thread
int lorawan_start_tx(void) {
    // create the LoRaWAN thread with the defined stack and function
    k_thread_create(&lorawan_thread_data, lorawan_stack, K_THREAD_STACK_SIZEOF(lorawan_stack),
                    lorawan_thread, NULL, NULL, NULL, 3, 0, K_NO_WAIT);
}