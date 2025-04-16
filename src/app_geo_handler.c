/*
 * Copyright (c) 2025
 * Regis Rousseau
 * Univ Lyon, INSA Lyon, Inria, CITI, EA3720
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app_geo_handler.h"

//  ========== globals =====================================================================
// circular buffer for ADC data
volatile uint16_t adcBuffer[BUFFER_SIZE];
volatile uint16_t bufferIndex = 0;

//  ========== app_adc_callback() ==========================================================
int8_t app_adc_callback(uint16_t sample)
{
	    // called by ADC interrupt or DMA
		adcBuffer[bufferIndex] = sample;
		bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
}

//  ========== app_int8_t app_send_to_lorawan ==============================================
int8_t app_send_to_lorawan(uint16_t *data, size_t length)
{
	int8_t ret;

	ret = lorawan_send(LORAWAN_PORT, data, length, LORAWAN_MSG_CONFIRMED);

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
}

//  ========== app_geo_handler =============================================================
int8_t app_geo_handler()
{
	static uint32_t staSum = 0;
    static uint32_t ltaSum = 0;
    static size_t staCount = 0;
    static size_t ltaCount = 0;
    static uint16_t processedIndex = 0;

    // process new samples since the last run
    while (processedIndex != bufferIndex) {
        uint16_t sample = adcBuffer[processedIndex];
        processedIndex = (processedIndex + 1) % BUFFER_SIZE;

        // update STA and LTA
        staSum += sample;
        ltaSum += sample;
        staCount++;
        ltaCount++;

        if (staCount > STA_WINDOW) {
            staSum -= adcBuffer[(processedIndex - STA_WINDOW + BUFFER_SIZE) % BUFFER_SIZE];
            staCount--;
        }

        if (ltaCount > LTA_WINDOW) {
            ltaSum -= adcBuffer[(processedIndex - LTA_WINDOW + BUFFER_SIZE) % BUFFER_SIZE];
            ltaCount--;
        }

        // calculate STA/LTA ratio
        if (staCount >= STA_WINDOW && ltaCount >= LTA_WINDOW) {
            float sta = (float)staSum / STA_WINDOW;
            float lta = (float)ltaSum / LTA_WINDOW;

            if (lta > 0 && (sta / lta) > THRESHOLD) {
                // trigger detection event
                size_t detectionLength = STA_WINDOW + LTA_WINDOW;
                uint16_t detectionData[detectionLength];
                for (size_t i = 0; i < detectionLength; i++) {
                    detectionData[i] = adcBuffer[(processedIndex - detectionLength + i + BUFFER_SIZE) % BUFFER_SIZE];
                }
                app_send_to_lorawan(detectionData, detectionLength);
            }
        }
    }
	return 0;
}