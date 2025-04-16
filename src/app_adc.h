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
#include <stddef.h>

//  ========== defines =====================================================================
#define ADC_REFERENCE_VOLTAGE       3300    // 3.3V reference voltage of the board
#define ADC_RESOLUTION              4096    // 12-bit resolution
#define ADC_BUFFER_SIZE             256
#define BATTERY_MAX_VOLTAGE         4200
#define BATTERY_MIN_VOLTAGE         2900

//  ========== globals =====================================================================
extern struct k_sem data_ready_sem;

//  ========== prototypes ==================================================================
int8_t app_nrf52_adc_init();
int16_t app_nrf52_get_ain1();
int adc_sampling_init(void);
void adc_sampling_start(void);
void adc_get_buffer(uint16_t *dest, size_t size, int offset);
void sta_lta_start(void);

#endif /* APP_ADC_H */