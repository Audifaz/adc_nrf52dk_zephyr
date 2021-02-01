/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <stdio.h>
#include <string.h>
#include <drivers/uart.h>
#include <drivers/adc.h>
#include <zephyr.h>

const struct device *adc_dev;

#include <hal/nrf_saadc.h>
#define ADC_DEVICE_NAME DT_ADC_0_NAME
#define ADC_RESOLUTION 10
#define ADC_GAIN ADC_GAIN_1_6
#define ADC_REFERENCE ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10)
#define ADC_1ST_CHANNEL_ID 7
#define ADC_1ST_CHANNEL_INPUT NRF_SAADC_INPUT_AIN7
#define ADC_2ND_CHANNEL_ID 2
#define ADC_2ND_CHANNEL_INPUT NRF_SAADC_INPUT_AIN2

static const struct adc_channel_cfg m_1st_channel_cfg = {
	.gain = ADC_GAIN,
	.reference = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id = ADC_1ST_CHANNEL_ID,
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
	.input_positive = ADC_1ST_CHANNEL_INPUT,
#endif
};

#define BUFFER_SIZE 1
static int16_t m_sample_buffer[BUFFER_SIZE];

static int adc_sample(void)
{
	int ret;

	const struct adc_sequence sequence = {
		.channels = BIT(ADC_1ST_CHANNEL_ID),
		.buffer = m_sample_buffer,
		.buffer_size = sizeof(m_sample_buffer),
		.resolution = ADC_RESOLUTION,
	};

	if (!adc_dev) {
		return -1;
	}

	ret = adc_read(adc_dev, &sequence);
	printk("ADC read err: %d\n", ret);

	/* Print the AIN0 values */
        
	for (int i = 0; i < BUFFER_SIZE; i++) {
		int adc_voltage = 0;
		adc_voltage = (int)(((float)m_sample_buffer[i] / 1023.0f) *3600.0f);
                //adc_voltage = (float) m_sample_buffer[i] ;
		printk("ADC raw value: %d\n", m_sample_buffer[i]);
		printf("Measured voltage: %i mV\n", adc_voltage);
	}
        /*
        printk("ADC raw value 0: %d\n", m_sample_buffer[0]);
        printk("ADC raw value 1: %d\n", m_sample_buffer[1]);
        printk("ADC raw value 3: %d\n", m_sample_buffer[2]);*/
	return ret;
}

int main(void)
{
	int err;

	printk("nrf91 saadc sampling AIN0 (P0.13)\n");
	printk("Example requires secure_boot to have ");
	printk("SAADC set to non-secure!\n");
	printk("If not; BusFault/UsageFault will be triggered\n");

	adc_dev = device_get_binding("ADC_0");
	if (!adc_dev) {
		printk("device_get_binding ADC_0 failed\n");
		while(1);
	}
	err = adc_channel_setup(adc_dev, &m_1st_channel_cfg);
	if (err) {
		printk("Error in adc setup: %d\n", err);
		while(1);
	}

	/* Trigger offset calibration
	 * As this generates a _DONE and _RESULT event
	 * the first result will be incorrect.
	 */
	NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
	while (1) {
		err = adc_sample();
		if (err) {
			printk("Error in adc sampling: %d\n", err);
		}
		k_sleep(K_MSEC(500));
	}
}
