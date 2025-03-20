/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_eeprom.h"
#include "app_rtc.h"

//  ========== globals =====================================================================
int8_t ind;		// declaration of isr index

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
	ret = flash_erase(dev, SPI_FLASH_OFFSET, SPI_FLASH_SECTOR_SIZE*SPI_FLASH_PAGE_SIZE);
	if (ret != 0){
		printf("MX25R64 flash erase failed. error: %d ", ret);
	} else {
		printk("MX25R64 flash erase succeeded", size);
	}

	// initialisation of isr index
	ind = 0;	
	return 0;
}

//  ========== app_eeprom_write ============================================================
int8_t app_eeprom_write(const struct device *dev, uint16_t data[])
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
	for (ind = 0; ind < MAX_RECORDS; ind++) {
		printk("wrt -> rom val: %"PRIu16"\n", data[ind]);
	}
	return 0;
}

//  ========== app_rom_read ================================================================
int8_t app_eeprom_read(const struct device *dev)
{
	int8_t ret = 0;
	uint16_t data[MAX_RECORDS];

	// reading the first page
	ret = flash_read(dev, SPI_FLASH_OFFSET, &data, sizeof(data));
	if (ret) {
		printk("error reading data. error: %d\n", ret);
	} else {
		printk("read %zu bytes from address 0x00\n", sizeof(data));
	}

	// reading data
	for (ind = 0; ind < MAX_RECORDS; ind++) {
		printk("rd -> rom val: %"PRIu16"\n", data[ind]);
	}
	return 0;
}

//  ======== app_rom_handler =======================================
int8_t app_rom_handler(const struct device *dev)
{
	int8_t ret = 0;
	int16_t data[MAX_RECORDS];
	int *timestamp;
	const struct device *rtc;

	// getting eeprom device
	dev = DEVICE_DT_GET(SPI_FLASH_DEVICE);

	// getting ds3231 rtc i2c device (SDA: P0.09, SCL:P0.0)
    rtc = DEVICE_DT_GET_ONE(maxim_ds3231);

	// getting time 
	timestamp = app_rtc_get_time(rtc);

	// putting timestamp at the beginning of the fisrt page for this test
	app_rom_write(dev, timestamp);

	// putting n structures in fisrt page for this test
	while (ind < MAX_RECORDS) {
		data[ind] = app_adc_get_val();
		ind++;
	}

	// writing and reading stored data
	app_rom_write(dev, data);
	app_rom_read(dev);

	// cleaning data storage partition
	(void)flash_erase(dev, SPI_FLASH_OFFSET, SPI_FLASH_SECTOR_SIZE*SPI_FLASH_PAGE_SIZE);
	ind = 0;
	return 0;
}


