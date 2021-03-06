/*
 * Copyright (c) 2021 BrainCo Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/ipm.h>
#include <drivers/gpio.h>
#include <sys/printk.h>

#define SLEEP_TIME_MS   1000

#define IPM  DT_LABEL(DT_NODELABEL(mailbox))

struct ipm_data {
	const struct gpio_dt_spec *led;
	bool led_is_on;
};

static struct ipm_data data;

static const struct gpio_dt_spec led0 =
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

void new_message_callback(const struct device *dev, void *user_data,
						uint32_t id, volatile void *data)
{
	struct ipm_data *ipm_data = (struct ipm_data *)user_data;

	ipm_data->led_is_on = !ipm_data->led_is_on;

	gpio_pin_set(ipm_data->led->port, ipm_data->led->pin, (int)ipm_data->led_is_on);
}

void main()
{
	printk("STM32 h7_dual_core application\n");
	
	const struct device *ipm = device_get_binding(IPM);
	if (!device_is_ready(ipm)) {
		printk("ipm device not ready\n");
		return;
	}

	if (!device_is_ready(led0.port)) {
		printk("led device not ready\n");
		return;
	}

	data.led = &led0;
	data.led_is_on = false;

	ipm_register_callback(ipm, new_message_callback, &data);

	ipm_set_enabled(ipm, 1);

	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

	while (1) {
		ipm_send(ipm, 0, 0, NULL, 0);
		k_msleep(SLEEP_TIME_MS);
	}
}