/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_sht31_bat_handler.h"
#include "app_adc.h"
#include "app_eeprom.h"

//  ========== interrupt sub-routine =======================================================
void thb_work_handler(struct k_work *work_thb)
{
	app_sht31_bat_handler();
}
K_WORK_DEFINE(thb_work, thb_work_handler);

void thb_timer_handler(struct k_timer *timer)
{
	k_work_submit(&thb_work);
}
K_TIMER_DEFINE(thb_timer, thb_timer_handler, NULL);

//  ========== main ========================================================================
int8_t main(void)
{
	const struct device *dev;
	struct nvs_fs flash;

	printk("Initializtion of all Hardware Devices\n");

	// initialize ADC device
	int8_t ret = app_nrf52_adc_init();
	if (ret != 1) {
		printk("failed to initialize ADC device");
		return 0;
	}

	// initialize flash memory
	ret = app_flash_init(&flash);
	if (ret != 1) {
		printk("failed to initialize ADC device");
		return 0;
	}

	// initialize the EEPROM device
	const struct device *flash_dev = DEVICE_DT_GET(SPI_FLASH_DEVICE);
	ret = app_eeprom_init(flash_dev);
	if (ret != 1) {
		printk("failed to initialize QSPI Flash device\n");
		return 0;
	}

	// initialize DS3231 RTC device via I2C (Pins: SDA -> P0.09, SCL -> P0.0)
	const struct device *rtc_dev = app_rtc_init();
    if (!rtc_dev) {
        printk("failed to initialize RTC device\n");
        return 0;
    }

	// initialize LoRaWAN protocol and register the device
	ret = app_lorawan_init();
	if (ret != 1) {
		printk("failed to initialze LoRaWAN protocol\n");
		return 0;
	}

	printk("Geophone Measurement and Process Information\n");
	// start ADC sampling and STA/LTA threads
	adc_sampling_start();
    sta_lta_start();

	k_timer_start(&thb_timer, K_NO_WAIT, K_MINUTES(1));

	return 0;
}