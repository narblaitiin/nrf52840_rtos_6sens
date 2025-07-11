/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_adc.h"
#include "app_sht31.h"
#include "app_flash.h"

//  ========== globals =====================================================================
struct flash_header {
    uint32_t magic;
    uint32_t head;
};

//  ========== app_flash_init ==============================================================
int8_t app_flash_init()
{
	const struct flash_area *fa;
	int8_t  ret = flash_area_open(FIXED_PARTITION_ID(storage_partition), &fa);
	if (ret != 0) {
    	printk("Failed to open flash area\n");
    	return -1;
	}
	
	struct flash_header hdr;
	flash_area_read(fa, FLASH_HEAD_OFFSET, &hdr, sizeof(hdr));

	if (hdr.magic != MAGIC_KEY) {
		printk("flash not initialized. initializing now.\n");

		// Erase first sector
		flash_area_erase(fa, FLASH_HEAD_OFFSET, FLASH_SECTOR_SIZE);

		// Write new header
		hdr.magic = MAGIC_KEY;
		hdr.head = 0;
		flash_area_write(fa, FLASH_HEAD_OFFSET, &hdr, sizeof(hdr));
	} else {
		printk("flash already initialized. Head: %u\n", hdr.head);
	}
	flash_area_close(fa);
	return 1;
}

//  ========== app_flash_store =============================================================
int8_t app_flash_store(const struct vth *data)
{
    const struct flash_area *fa;
    int8_t ret = flash_area_open(FIXED_PARTITION_ID(storage_partition), &fa);
    if (ret != 0) {
        printk("failed to open flash area\n");
        return -1;
    }

    // read head from the head sector
    uint32_t head = 0;
    flash_area_read(fa, 0, &head, sizeof(head));  // head at offset 0

    // compute current data offset
    uint32_t index = head % MAX_RECORDS;
    off_t data_offset = FLASH_DATA_OFFSET + index * RECORD_SIZE;

    // erase sector only when writing first record of a sector
	uint32_t records_per_sector = FLASH_SECTOR_SIZE / RECORD_SIZE;
	if ((index % records_per_sector) == 0) {
		off_t sector_offset = data_offset & ~(FLASH_SECTOR_SIZE - 1);
		ret = flash_area_erase(fa, sector_offset, FLASH_SECTOR_SIZE);
		if (ret != 0) {
			printk("erase failed at offset 0x%x\n", (uint32_t)sector_offset);
			flash_area_close(fa);
			return -1;
		}
	}

    // write record
    ret = flash_area_write(fa, data_offset, data, RECORD_SIZE);
    if (ret != 0) {
        printk("write failed at offset 0x%x\n", (uint32_t)data_offset);
        flash_area_close(fa);
        return -1;
    }

    // Update head
    head = (head + 1) % MAX_RECORDS;
    flash_area_write(fa, 0, &head, sizeof(head));
    if (ret != 0) {
		printk("failed to update head\n");
		flash_area_close(fa);
		return -1;
	}
    flash_area_close(fa);
    return 0;
}

//  ========== app_flash_handler ===========================================================
int8_t app_flash_handler(const struct device *dev)
{
	struct vth data;

	// retrieve the sensor device using the device tree API
	if (!device_is_ready(dev)) {
        printk("%s: sensor device not ready\n", dev->name);
        return -1;
    }

	// collect sensor data until the maximum number of records is reached
	// measure and store the battery voltage
	data.vbat = app_nrf52_get_vbat();

	// measure and store the temperature using the SHT31 sensor
	data.temp = app_sht31_get_temp(dev);

	k_msleep(5000);		// small delay  between reading the temperature and humidity values

	// measure and store the humidity using the SHT31 sensor
	data.hum = app_sht31_get_hum(dev);

	// save the collected sensor data into the flash memory
	int8_t ret = app_flash_store(&data);
    return 1;
}

