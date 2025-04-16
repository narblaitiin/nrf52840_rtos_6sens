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
#define STA_WINDOW_SIZE 20
#define LTA_WINDOW_SIZE 100
#define THRESHOLD       3.0

//  ========== globals =====================================================================
K_THREAD_STACK_DEFINE(sta_lta_stack, 1024);
struct k_thread sta_lta_thread_data;

static uint16_t sta_buffer[STA_WINDOW_SIZE];
static uint16_t lta_buffer[LTA_WINDOW_SIZE];

//  ========== calculate_sta ===============================================================
static float calculate_sta(const uint16_t *buffer, size_t size) {
    float sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += buffer[i];
    }
    return sum / size;
}

//  ========== calculate ===================================================================
static float calculate_lta(const uint16_t *buffer, size_t size) {
    float sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += buffer[i];
    }
    return sum / size;
}

//  ========== sta_lta_thread ==============================================================
static void sta_lta_thread(void *arg1, void *arg2, void *arg3) {
    while (1) {
        k_sem_take(&data_ready_sem, K_FOREVER);

        adc_get_buffer(sta_buffer, STA_WINDOW_SIZE, -STA_WINDOW_SIZE);
        adc_get_buffer(lta_buffer, LTA_WINDOW_SIZE, -LTA_WINDOW_SIZE);

        float sta = calculate_sta(sta_buffer, STA_WINDOW_SIZE);
        float lta = calculate_lta(lta_buffer, LTA_WINDOW_SIZE);

        if (sta / lta > THRESHOLD) {
            printk("event detected: STA/LTA = %.2f", sta / lta);
            lorawan_trigger_transmission();
        }
    }
}
//  ========== sta_lta_start ===============================================================
void sta_lta_start(void) {
    k_thread_create(&sta_lta_thread_data, sta_lta_stack, K_THREAD_STACK_SIZEOF(sta_lta_stack),
                    sta_lta_thread, NULL, NULL, NULL, 2, 0, K_NO_WAIT);
}