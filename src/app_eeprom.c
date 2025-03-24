/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_eeprom.h"
#include "app_rtc.h"

//  ========== app_eeprom_init =============================================================
int8_t app_eeprom_init(const struct device *dev)
{
	int8_t ret = 0;
	ssize_t size, page;

	// getting EEPROM size
	dev = DEVICE_DT_GET(SPI_FLASH_DEVICE);
	if (dev == NULL) {
		printk("no eeprom device found. error: %d\n", dev);
		return 0;
	}
	
	if (!device_is_ready(dev)) {
		printk("eeprom is not ready\n");
		return 0;
	} else {
        printk("- found device \"%s\", writing/reading data\n", dev->name);
    }

	// getting the total number of flash pages in bytes
	page = flash_get_page_count(dev);
	printk("number of pages: %zu\n", page);

	// cleaning data storage partition
	ret = flash_erase(dev, SPI_FLASH_OFFSET, SPI_FLASH_SECTOR_SIZE*SPI_FLASH_SECTOR_NB);
	if (ret != 0){
		printf("MX25R64 flash erase failed. error: %d ", ret);
	} else {
		printk("MX25R64 flash erase succeeded", size);
	}
	return 0;
}

//  ========== app_eeprom_write ============================================================
int8_t app_eeprom_write(const struct device *dev, int16_t data)
{
	int8_t ret = 0;
	
	// writing data in the first page of 4kbytes
	ret = flash_write(dev, SPI_FLASH_OFFSET, &data, sizeof(data));
	if (ret!=0) {
		printk("error writing data. error: %d\n", ret);
	} else {
		printk("wrote %zu bytes to address 0x00\n", sizeof(data));
	}
	// printing data
	// for (int8_t i = 0; i < MAX_RECORDS; i++) {
	// 	printk("wrt -> rom val: %d\n", data[i]);
	// }
	printk("wrt -> rom val: %d\n", data);

	return 0;
}

//  ========== app_rom_read ================================================================
int16_t* app_eeprom_read(const struct device *dev)
{
	int8_t ret = 0;
	int16_t data[MAX_RECORDS];

	// reading the first page
	ret = flash_read(dev, SPI_FLASH_OFFSET, &data, sizeof(data));
	if (ret) {
		printk("error reading data. error: %d\n", ret);
	} else {
		printk("read %zu bytes from address 0x00\n", sizeof(data));
	}

	// reading data
	for (int8_t i = 0; i < MAX_RECORDS; i++) {
		printk("rd -> rom val: %d\n", data[i]);
	}
	return data;
}




