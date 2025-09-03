/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_sensors.h"

//  ========== app_sensors_handler =======================================================
int8_t app_sensors_handler()
{
    int8_t ret;
    uint8_t byte_payload[BYTE_PAYLOAD] = {0};

    // Cconfiguration of LEDs
    static const struct gpio_dt_spec led_tx = GPIO_DT_SPEC_GET(LED_TX, gpios);
    static const struct gpio_dt_spec led_rx = GPIO_DT_SPEC_GET(LED_RX, gpios);
    gpio_pin_configure_dt(&led_tx, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led_rx, GPIO_OUTPUT_ACTIVE);

    // LEDs off
    gpio_pin_set_dt(&led_tx, 0);
    gpio_pin_set_dt(&led_rx, 0);

    // retrieve the current timestamp from the RTC device
    uint64_t timestamp = app_ds3231_get_time();
    //uint64_t timestamp = app_rtc_get_time();

    // add timestamp to byte payload (big-endian)
    for (int8_t i = 0; i < 8; i++) {
        byte_payload[i] = (timestamp >> (56 - i * 8)) & 0xFF;
    }

    // get sensor device
    const struct device *dev = DEVICE_DT_GET_ONE(sensirion_sht3xd);
    if (!device_is_ready(dev)) {
        printk("sensor device not ready\n");
        return -ENODEV;
    }

    // collect sensor data and add to byte payload
    int8_t index = 8; // Start after the timestamp
    int16_t ain1 = app_nrf52_get_ain1();
    int16_t temp = app_sht31_get_temp(dev);
    int16_t hum = app_sht31_get_hum(dev);
    int16_t velocity = 0;

    // Convert and append each sensor value to byte payload (big-endian)
    int16_t sensor_data[] = {ain1, temp, hum, velocity};
    for (int j = 0; j < sizeof(sensor_data) / sizeof(sensor_data[0]); j++) {
        byte_payload[index++] = (sensor_data[j] >> 8) & 0xFF;       // high byte
        byte_payload[index++] = sensor_data[j] & 0xFF;              // low byte
    }

    printk("sending battery level, temperature, humidity...\n");

    // blink LEDs when LoRaWAN is activated
    gpio_pin_toggle_dt(&led_tx);
    gpio_pin_toggle_dt(&led_rx);

    ret = lorawan_send(LORAWAN_PORT, byte_payload, index, LORAWAN_MSG_UNCONFIRMED);

    if (ret < 0) {
        printk("lorawan_send failed: %d\n", ret);
        return ret == -EAGAIN ? 0 : ret;
    }

    printk("data sent!\n");
    return 0;
}