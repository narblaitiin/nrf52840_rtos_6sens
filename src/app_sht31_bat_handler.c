/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_sht31_bat_handler.h"

//  ========== app_sht31_bat_handler =======================================================
int8_t app_sht31_bat_handler()
{
    int8_t ret;
    int16_t raw_payload[RAW_PAYLOAD];
    uint8_t byte_payload[BYTE_PAYLOAD];

    // config of LEDs
    static const struct gpio_dt_spec led_tx = GPIO_DT_SPEC_GET(LED_TX, gpios);
    static const struct gpio_dt_spec led_rx = GPIO_DT_SPEC_GET(LED_RX, gpios);
    gpio_pin_configure_dt(&led_tx, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led_rx, GPIO_OUTPUT_ACTIVE);
  
    // LEDs off
	gpio_pin_set_dt(&led_tx, 0);
	gpio_pin_set_dt(&led_rx, 0);

    // initialize DS3231 RTC device via I2C (Pins: SDA -> P0.09, SCL -> P0.0)
    const struct device *rtc = DEVICE_DT_GET_ONE(maxim_ds3231);

    // retrieve the current timestamp from the RTC device 
    struct tm current_time;  
    int32_t timestamp = app_rtc_get_time(rtc, &current_time);

    // add the timestamp to the start of the data buffer
    raw_payload[0] = (timestamp >> 16) & 0xFF;      // most significant byte
    raw_payload[1] = timestamp & 0xFF;              // least significant byte
       
    // get sensor device
	const struct device *dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);
    if (!device_is_ready(dev)) {
        printk("%s: sensor device not ready\n", dev->name);
        return -ENODEV;
    }

    raw_payload[2] = app_nrf52_get_ain1();
    raw_payload[3] = app_sht31_get_temp(dev);
    k_msleep(2000);		// small delay  between reading
    raw_payload[4] = app_sht31_get_hum(dev);

    // no velocity
    raw_payload[5] = 0;

    // convert int16_t payload to byte array with big-endian representation
    for (int j = 0; j < 7; j++) {
        byte_payload[j * 2] = (raw_payload[j] >> 8) & 0xFF; // high byte
        byte_payload[j * 2 + 1] = raw_payload[j] & 0xFF;    // low byte
    }

    printk("sending battery level, temparature, humdity...\n");

    // blink LEDs when lorawan is activated
	gpio_pin_toggle_dt(&led_tx);
    gpio_pin_toggle_dt(&led_rx);

	ret = lorawan_send(LORAWAN_PORT, byte_payload, sizeof(byte_payload), LORAWAN_MSG_UNCONFIRMED);

    if (ret == -EAGAIN) {
        printk("lorawan_send failed: %d. continuing...\n", ret);
        return 0;
    }
    
    if (ret < 0) {
        printk("lorawan_send failed. error: %d.\n", ret);
        return 0;
    } else {
        printk("data sent!\n");
    }

    return 0;
}