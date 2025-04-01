/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_sht31_bat_handler.h"

//  ========== app_sht31_bat_handler =======================================================
int8_t app_sht31_bat_handler ()
{
    int8_t i = 0;
    int8_t ret;
    int16_t payload[3];
    const struct device *dev;

    static const struct gpio_dt_spec led_tx = GPIO_DT_SPEC_GET(LED_TX, gpios);

    // configuration of LEDs
	gpio_pin_configure_dt(&led_tx, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set_dt(&led_tx, 0);

    // getting all sensor devices
	dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);

    payload[i] = app_nrf52_get_ain1();
    payload[i+1] = app_sht31_get_temp(dev);
    payload[i+2] = app_sht31_get_hum(dev);

    // initialization of LoRaWAN - TTN
	app_lorawan_init(dev);

    printk("sending random data...\n");

	gpio_pin_set_dt(&led_tx, 1);
	ret = lorawan_send(LORAWAN_PORT, (int8_t)payload, sizeof(payload), LORAWAN_MSG_UNCONFIRMED);

    if (ret == -EAGAIN) {
        printk("lorawan_send failed: %d. continuing...\n", ret);
        return 0;
    }
    
    if (ret < 0) {
        printk("lorawan_send failed: %d.\n", ret);
        return 0;
    } else {
        // flashing of the LED when a packet is transmitted
        ret = gpio_pin_toggle_dt(&led_tx);
        printk("data sent!\n");
    }
    gpio_pin_set_dt(&led_tx, 0);
    return 0;
}