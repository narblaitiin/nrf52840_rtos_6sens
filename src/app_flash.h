/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_FLASH_H
#define APP_FLASH_H

//  ========== includes ====================================================================
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

//  ========== globals =====================================================================
struct vth {
	int16_t vbat;
	int16_t temp;
	int16_t hum;
};

//  ========== defines =====================================================================
#define FLASH_PARTITION_OFFSET  0x000F8000	
#define FLASH_SECTOR_SIZE       4096		// one flash page = 4 KB		
#define FLASH_SECTOR_COUNT      4			// 4 sectors × 4 KB = 16 KB
#define FLASH_TOTAL_SIZE        (FLASH_SECTOR_SIZE * FLASH_SECTOR_COUNT)

#define FLASH_HEAD_OFFSET		0x0000                 
#define FLASH_DATA_OFFSET       0x0100

#define MAX_RECORDS             (0x8000 - FLASH_DATA_OFFSET) / RECORD_SIZE  // = 4064
#define RECORD_SIZE             8 
#define MAGIC_KEY				0xDEADBEEF

//  ========== prototypes ==================================================================
int8_t app_flash_init(void);
int8_t app_flash_store(const struct vth *data);
int8_t app_flash_handler(const struct device *dev);

#endif /* APP_FLASH_H */