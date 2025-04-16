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

	static const struct gpio_dt_spec led_tx = GPIO_DT_SPEC_GET(LED_TX, gpios);
    static const struct gpio_dt_spec led_rx = GPIO_DT_SPEC_GET(LED_RX, gpios);

    // configuration of LEDs
	gpio_pin_configure_dt(&led_tx, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led_rx, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set_dt(&led_tx, 0);
	gpio_pin_set_dt(&led_rx, 0);

	// initialization of all devices
	app_nrf52_adc_init();
	app_sht31_init(dev);
	app_flash_init(&fs);
	app_eeprom_init(dev);;
	app_rtc_init(dev);

	// initialization of LoRaWAN
	app_lorawan_init(dev);

	printk("Geophone Measurement and Process Information\nBoard: %s\n", CONFIG_BOARD);

	// start ADC sampling and STA/LTA threads
	adc_sampling_start();
    sta_lta_start();

//	k_timer_start(&thb_timer, K_NO_WAIT, K_MINUTES(30));

	return 0;
}