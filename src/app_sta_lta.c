/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_adc.h"
#include "app_lorawan.h"

//  ========== defines =====================================================================
// STA and LTA window durations in milliseconds
#define STA_WINDOW_DURATION_MS      1000     // 1 seconds
#define LTA_WINDOW_DURATION_MS      10000    // 10 seconds

// derived buffer sizes
#define STA_WINDOW_SIZE (STA_WINDOW_DURATION_MS / SAMPLING_RATE_MS)
#define LTA_WINDOW_SIZE (LTA_WINDOW_DURATION_MS / SAMPLING_RATE_MS)

// trigger thresholds with hysteresis
#define TRIGGER_THRESHOLD           0.8f    // 5.0f // STA/LTA ratio to trigger event
#define RESET_THRESHOLD             0.6f    // 2.5f // STA/LTA ratio to reset trigger

//  ========== globals =====================================================================
// define a thread stack with a size of 1024 bytes for the STA/LTA thread.
K_THREAD_STACK_DEFINE(sta_lta_stack, 1024);

// declare a thread data structure to manage the STA/LTA thread.
struct k_thread sta_lta_thread_data;

// buffer to hold the Short-Term Average (STA) and Long-Term Average (LTA) samples.
static uint16_t sta_buffer[STA_WINDOW_SIZE];
static uint16_t lta_buffer[LTA_WINDOW_SIZE];

//  ========== calculate_sta ===============================================================
// function to calculate the Short-Term Average (STA) of a given buffer
static float calculate_sta(const uint16_t *buffer, size_t size)
{
    float sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += (float)buffer[i];
    }
    //printk("STA sum: %.2f\n", sum/size);
    return sum / size;
}

//  ========== calculate_lad ===============================================================
// function to calculate the Long-Term Average (LTA) of a given buffer
static float calculate_lta(const uint16_t *buffer, size_t size)
{
    float sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += (float)buffer[i];
    }
    //printk("LTA sum: %.2f\n", sum/size);
    return sum / size;
}

//  ========== sta_lta_thread ==============================================================
// thread function to monitor and analyze data using the STA/LTA algorithm
static void app_sta_lta_thread(void *arg1, void *arg2, void *arg3)
{
    static bool event_triggered = false;
    while (1) {
        // wait for a semaphore indicating that new ADC data is available
        k_sem_take(&data_ready_sem, K_FOREVER);

        // printk("sta/lta: get adc buffer\n");

        int sta_offset = (ring_head - STA_WINDOW_SIZE + ADC_BUFFER_SIZE) % ADC_BUFFER_SIZE;
        int lta_offset = (ring_head - LTA_WINDOW_SIZE + ADC_BUFFER_SIZE) % ADC_BUFFER_SIZE;
        
        // retrieve the most recent data for the STA and LTA buffers
        app_adc_get_buffer(sta_buffer, STA_WINDOW_SIZE, sta_offset);
        app_adc_get_buffer(lta_buffer, LTA_WINDOW_SIZE, lta_offset);

        // calculate the STA and LTA values
        float sta = calculate_sta(sta_buffer, STA_WINDOW_SIZE);
        float lta = calculate_lta(lta_buffer, LTA_WINDOW_SIZE);

        // validate values before calculating ratio
        // check if LTA is zero
        if (lta == 0) {
            printk("Error: LTA is zero. Buffer might not be initialized properly.\n");
            // debug the buffer
            for (size_t i = 0; i < LTA_WINDOW_SIZE; i++) {
                printk("lta_buffer[%zu] = %d\n", i, lta_buffer[i]);
            }
            return 0;
        }
        
        float ratio = sta/lta;
        printk("STA: %.2f, LTA: %.2f, ratio: %.2f\n", sta, lta, ratio);

        // check if the STA/LTA ratio exceeds the defined threshold
        // if (ratio > TRIGGER_THRESHOLD) {
        //     printk(">>> EVENT START (ratio = %.2f)\n", ratio);
        //     app_lorawan_trigger_tx();
        // }

        // check if the STA/LTA ratio exceeds the defined threshold
        // trigger event with hysteresis
        if (!event_triggered && ratio > TRIGGER_THRESHOLD) {
            event_triggered = true;
            printk(">>> EVENT START (ratio = %.2f)\n", ratio);
            app_lorawan_trigger_tx();
        }
        else if (event_triggered && ratio < RESET_THRESHOLD) {
            event_triggered = false;
            printk("<<< EVENT END (ratio = %.2f)\n", ratio);
        }
    }
}

//  ========== sta_lta_start ===============================================================
// create and initialize the thread with the specified stack and priority
void app_sta_lta_start(void)
{
    k_thread_create(&sta_lta_thread_data, sta_lta_stack, K_THREAD_STACK_SIZEOF(sta_lta_stack),
                    app_sta_lta_thread, NULL, NULL, NULL, 2, 0, K_NO_WAIT);
}