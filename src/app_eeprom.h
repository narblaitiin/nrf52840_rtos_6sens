/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_EEPROM_H
#define APP_EEPROM_H

//  ========== includes ====================================================================
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>

//  ========== defines =====================================================================
#define SPI_FLASH_DEVICE        DT_COMPAT_GET_ANY_STATUS_OKAY(nordic_qspi_nor)
#define SPI_FLASH_OFFSET		0x00000
#define SPI_FLASH_SECTOR_SIZE	4096
#define SPI_FLASH_SECTOR_NB     2048
#define MAX_SAMPLES             120     // 1 sample: 34 bytes -> 120 samples/sectors
#define MAX_RECORDS             2  // 120 samples - 2048 sectors                            

//  ========== prototypes ==================================================================
int8_t app_eeprom_init(const struct device *dev);
int8_t app_eeprom_write(const struct device *dev, int16_t data);
int16_t* app_eeprom_read(const struct device *dev);

#endif /* APP_EEPROM_H */