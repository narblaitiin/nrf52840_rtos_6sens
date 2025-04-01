/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_SHT31_H
#define APP_SHT31_H

//  ========== includes ====================================================================
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <stdint.h>

//  ========== prototypes ==================================================================
int8_t app_sht31_init(const struct device *dev);
int16_t app_sht31_get_temp(const struct device *dev);
int16_t app_sht31_get_hum(const struct device *dev);

#endif /* APP_SHT31_H */