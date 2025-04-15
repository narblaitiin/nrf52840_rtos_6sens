/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_SHT31_ADC_HANDLER_H
#define APP_SHT31_ADC_HANDLER_H

//  ========== includes ====================================================================
#include "app_adc.h"
#include "app_flash.h"
#include "app_lorawan.h"
#include "app_sht31.h"

//  ========== defines =====================================================================
/* led control */
#define LED_TX                  DT_ALIAS(ledtx)     // declared in device tree
#define LED_RX                  DT_ALIAS(ledrx)
#define RAW_PAYLOAD             3                   // temperature, humdity, and battery level
#define BYTE_PAYLOAD            6                   // 3 int16 values => 6 bytes                 

//  ========== prototypes ==================================================================
int8_t app_sht31_bat_handler();

#endif /* APP_SHT31_ADC_HANDLER_H */