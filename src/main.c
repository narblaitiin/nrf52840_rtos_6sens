/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_sht31_bat_handler.h"
#include "app_adc.h"

//  ========== interrupt sub-routine =======================================================
// void thb_work_handler(struct k_work *work_thb)
// {
// 	app_sht31_bat_handler();
// }
// K_WORK_DEFINE(thb_work, thb_work_handler);

// void thb_timer_handler(struct k_timer *timer)
// {
// 	k_work_submit(&thb_work);
// }
// K_TIMER_DEFINE(thb_timer, thb_timer_handler, NULL);

//  ========== main ========================================================================
int8_t main(void)
{
	const struct device *dev;
	struct nvs_fs fs;

	printk("Initializtion of all Hardware Devices\n");
	app_nrf52_adc_init();
	app_sht31_init(dev);
	app_flash_init(&fs);
	app_eeprom_init(dev);;
	app_rtc_init(dev);

	printk("Initialization of LoRaWAN\n");
	app_lorawan_init(dev);

	printk("Geophone Measurement and Process Information\n");
	// start ADC sampling and STA/LTA threads
//	adc_sampling_start();
//  sta_lta_start();

//	k_timer_start(&thb_timer, K_NO_WAIT, K_MINUTES(1));

	return 0;
}