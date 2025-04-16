/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_lorawan.h"
#include "app_adc.h"

//  ========== globals =====================================================================
K_THREAD_STACK_DEFINE(lorawan_stack, 1024);
struct k_thread lorawan_thread_data;

//  ========== lorawan_thread ==============================================================
static void lorawan_thread(void *arg1, void *arg2, void *arg3) {
    uint8_t data[ADC_BUFFER_SIZE * sizeof(uint16_t)];

    while (1) {
        k_thread_suspend(k_current_get()); // Wait to be triggered

        adc_get_buffer((uint16_t *)data, ADC_BUFFER_SIZE, 0);

        lorawan_send(LORAWAN_PORT, data, sizeof(data), LORAWAN_MSG_UNCONFIRMED);
        printk("data sent to LoRaWAN");
    }
}

//  ========== lorawan_trigger_tx ==========================================================
void lorawan_trigger_transmission(void) {
    k_thread_resume(&lorawan_thread_data);
}

//  ========== lorawan_start ===============================================================
int lorawan_start_tx(void) {
    k_thread_create(&lorawan_thread_data, lorawan_stack, K_THREAD_STACK_SIZEOF(lorawan_stack),
                    lorawan_thread, NULL, NULL, NULL, 3, 0, K_NO_WAIT);
}