/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_geo_handler.h"

//  ======== app_geo_handler =======================================
int8_t app_geo_handler(const struct device *dev)
{
	int8_t ret = 0;
	int16_t data[MAX_RECORDS];

	// getting eeprom device
	dev = DEVICE_DT_GET(SPI_FLASH_DEVICE);

	// putting n structures in fisrt page for this test
	for (int8_t i = 0; i < MAX_SAMPLES; i++) {
		data[i] = app_nrf52_get_ain0();
		// writing and reading stored data
		app_eeprom_write(dev, data);
	}

	// cleaning data storage partition
//	(void)flash_erase(dev, SPI_FLASH_OFFSET, sizeof(data));
	return 0;
}