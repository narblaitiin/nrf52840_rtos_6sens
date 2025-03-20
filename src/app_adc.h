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
#include <math.h>

//  ========== defines =====================================================================
#define THRESHOLD       2048    // for now, VDD/2

//  ========== prototypes ==================================================================
int8_t app_nrf52_adc_init();
int16_t app_nrf52_get_ain0();
int16_t app_nrf52_get_ain1();

#endif /* APP_ADC_H */