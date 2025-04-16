/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_GEO_HANDLER_H
#define APP_GEO_HANDLER_H

//  ========== includes ====================================================================
#include "app_adc.h"
#include "app_eeprom.h"
#include "app_rtc.h"
#include "app_lorawan.h"

//  ========== defines =====================================================================
#define BUFFER_SIZE        1000
#define STA_WINDOW          10
#define LTA_WINDOW          100
#define THRESHOLD           3.0

//  ========== prototypes ==================================================================
int8_t app_send_to_lorawan(uint16_t *data, size_t length);
int8_t app_geo_handler();
int8_t app_adc_callback(uint16_t sample);

#endif /* APP_SHT31_ADC_HANDLER_H */
