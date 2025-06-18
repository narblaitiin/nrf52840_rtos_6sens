/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_ADC_H
#define APP_ADC_H

//  ========== includes ====================================================================
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

//  ========== defines =====================================================================
#define ADC_REFERENCE_VOLTAGE       3300    // 3.3V reference voltage of the board
#define ADC_RESOLUTION              4096    // 12-bit resolution
#define ADC_BUFFER_SIZE             1024     
#define SAMPLING_RATE_MS            10
#define BATTERY_MAX_VOLTAGE         2980
#define BATTERY_MIN_VOLTAGE         2270

//  ========== globals =====================================================================
extern struct k_sem data_ready_sem;
extern int ring_head;

//  ========== prototypes ==================================================================
int8_t app_nrf52_adc_init();
int16_t app_nrf52_get_ain1();
void app_adc_sampling_start(void);
void app_adc_sampling_stop(void);
void app_adc_get_buffer(uint16_t *dest, size_t size, int offset);
void app_adc_set_sampling_rate(uint32_t rate_ms);

#endif /* APP_ADC_H */