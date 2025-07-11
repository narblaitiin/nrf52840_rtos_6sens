/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

//  ========== includes ====================================================================
#include <zephyr/kernel.h>
#include "app_sensors.h"
#include "app_adc.h"
#include "app_eeprom.h"
#include "app_rtc.h"
#include <stdbool.h>
#include <stdio.h>

//  ========== defines =====================================================================
#define STACK_SIZE			2048
#define PRIORITY_RTC		2
#define PRIORITY_LORAWAN	3

//  ========== globals =====================================================================
// define GPIO specifications for the LEDs used to indicate transmission (TX) and reception (RX)
static const struct gpio_dt_spec led_tx = GPIO_DT_SPEC_GET(LED_TX, gpios);
static const struct gpio_dt_spec led_rx = GPIO_DT_SPEC_GET(LED_RX, gpios);

static void dl_callback(uint8_t port, bool data_pending,
			int16_t rssi, int8_t snr,
			uint8_t len, const uint8_t *hex_data)
{
	printk("Port %d, Pending %d, RSSI %ddB, SNR %ddBm\n", port, data_pending, rssi, snr);
}

// thread to have a periodic sync
bool rtc_thread_flag = true;
void rtc_thread_func(void)
{
	printk("periodic sync thread started\n");

	const struct device *rtc_dev = DEVICE_DT_GET(DT_NODELABEL(rtc0));
	while (rtc_thread_flag == true) {
        printk("performing periodic action\n");
        (void)app_rtc_periodic_sync(rtc_dev);
		//(void)app_ds3231_periodic_sync(rtc_dev);
        k_sleep(K_SECONDS(5));		
	}
}
K_THREAD_DEFINE(rtc_thread_id, STACK_SIZE, rtc_thread_func, NULL, NULL, NULL, PRIORITY_RTC, 0, 0);

// thread to send environment value when no activity
bool lorawan_thread_flag = true;
void lorawan_thread_func(void)
{
	printk("LoRaWAN thread started\n");
    while (lorawan_thread_flag == true) {
        printk("performing periodic action\n");
		// perform your task: get battery level, temperature and humidity
        (void)app_sensors_handler();	
        k_sleep(K_SECONDS(120));		// sleep for 2 minutes -> test
    }
}
K_THREAD_DEFINE(lorawan_thread_id, STACK_SIZE, lorawan_thread_func, NULL, NULL, NULL, PRIORITY_LORAWAN, 0, 0);

static void lorwan_datarate_changed(enum lorawan_datarate dr)
{
	uint8_t unused, max_size;

	lorawan_get_payload_sizes(&unused, &max_size);
	printk("New Datarate: DR_%d, Max Payload %d\n", dr, max_size);
}

//  ========== main ========================================================================
int8_t main(void)
{
	const struct device *dev;
	int8_t ret;

	printk("Initializtion of all Hardware Devices\n");

	// initialize ADC device
	ret = app_nrf52_adc_init();
	if (ret != 1) {
		printk("failed to initialize ADC device\n");
		return 0;
	}

	// initialize partition flash memory
	ret = app_flash_init();
	if (ret != 1) {
		printk("failed to initialize internal Flash device\n");
		return 0;
	}

	// initialize the EEPROM device
	const struct device *eeprom_dev = DEVICE_DT_GET(SPI_FLASH_DEVICE);
	ret = app_eeprom_init(eeprom_dev);
	if (ret != 1) {
		printk("failed to initialize QSPI flash device\n");
		return 0;
	}

	// initialize DS3231 RTC device via I2C (Pins: SDA -> P0.09, SCL -> P0.0)
	const struct device *rtc_dev = app_rtc_init();
    if (!rtc_dev) {
        printk("failed to initialize RTC device\n");
        return 0;
    }

	// initialize LoRaWAN protocol and register the device
	const struct device *lora_dev;
	struct lorawan_join_config join_cfg;
	uint8_t dev_eui[] = LORAWAN_DEV_EUI;
	uint8_t join_eui[] = LORAWAN_JOIN_EUI;
	uint8_t app_key[] = LORAWAN_APP_KEY;

	struct lorawan_downlink_cb downlink_cb = {
		.port = LW_RECV_PORT_ANY,
		.cb = dl_callback
	};

	lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
	if (!device_is_ready(lora_dev)) {
		printk("%s: device not ready\n", lora_dev->name);
		return 0;
	}

	ret = lorawan_start();
	if (ret < 0) {
		printk("lorawan_start failed: %d", ret);
		return 0;
	}

	lorawan_register_downlink_callback(&downlink_cb);
	lorawan_register_dr_changed_callback(lorwan_datarate_changed);

	join_cfg.mode = LORAWAN_ACT_OTAA;
	join_cfg.dev_eui = dev_eui;
	join_cfg.otaa.join_eui = join_eui;
	join_cfg.otaa.app_key = app_key;
	join_cfg.otaa.nwk_key = app_key;
	join_cfg.otaa.dev_nonce = 0u;

	printk("Joining network over OTAA\n");
	ret = lorawan_join(&join_cfg);
	if (ret < 0) {
		printk("lorawan_join_network failed: %d\n", ret);
		return 0;
	}

	printk("Geophone Measurement and Process Information\n");

	// enable environmental sensor and battery level thread
	lorawan_thread_flag = true;

	// enable periodic rtc sync thread
	rtc_thread_flag = true;

	// start ADC sampling and STA/LTA threads
	app_adc_sampling_start();
    app_sta_lta_start();
	return 0;
}