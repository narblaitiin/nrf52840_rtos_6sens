/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include "app_geo_handler.h"
#include "app_sht31_bat_handler.h"

//  ========== interrupt sub-routine =======================================================
void adc_work_handler(struct k_work *work_geo)
{
	app_sht31_bat_handler();
}
K_WORK_DEFINE(geo_work, adc_work_handler);

void adc_timer_handler(struct k_timer *timer)
{
	k_work_submit(&geo_work);
}
K_TIMER_DEFINE(adc_timer, adc_timer_handler, NULL);

//  ========== main ========================================================================
int8_t main(void)
{
	const struct device *dev;
	struct nvs_fs fs;
	int32_t *timestamp;
	int16_t geo_adc;
	uint8_t dev_eui[] = LORAWAN_DEV_EUI;
	int8_t ret;

	struct data {
		uint8_t *id;
		int32_t timestamp;
		int16_t val;
	} payload[MAX_SAMPLES]; // just for test

	static const struct gpio_dt_spec led_tx = GPIO_DT_SPEC_GET(LED_TX, gpios);
    static const struct gpio_dt_spec led_rx = GPIO_DT_SPEC_GET(LED_RX, gpios);

    // configuration of LEDs
	gpio_pin_configure_dt(&led_tx, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led_rx, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set_dt(&led_tx, 0);
	gpio_pin_set_dt(&led_rx, 0);

	// initialization of all devices
	app_sht31_init(dev);
	app_eeprom_init(dev);
	app_rtc_init(dev);

	// initialization of LoRaWAN
	app_lorawan_init(dev);

	printk("Geophone Measurement and Process Information\nBoard: %s\n", CONFIG_BOARD);
	
	// beginning of isr timer
	k_timer_start(&adc_timer, K_NO_WAIT, K_MINUTES(15));

	while(1) {
		geo_adc = app_nrf52_get_ain0();

		if (geo_adc > THRESHOLD) {

			payload->id = dev_eui;
			payload->timestamp = app_rtc_get_time (dev);

			// putting n structures in fisrt page for this test
			// for (int8_t i = 0; i < MAX_SAMPLES; i++) {
			// 	payload->val = app_nrf52_get_ain0();
			// }

			(void)app_geo_handler();

			payload->val = app_eeprom_read(dev);
			
			gpio_pin_set_dt(&led_tx, 1);
			ret = lorawan_send(LORAWAN_PORT, payload, sizeof(payload), LORAWAN_MSG_UNCONFIRMED);
			gpio_pin_set_dt(&led_tx, 0);

			if (ret == -EAGAIN) {
				printk("lorawan_send failed: %d. continuing...\n", ret);
				return 0;
			}
			
			if (ret < 0) {
				printk("lorawan_send failed: %d.\n", ret);
				return 0;
			} else {
				// flashing of the LED when a packet is transmitted
				printk("data sent!\n");
			}
		}
	}
	return 0;
}