/*
 * Copyright (c) 2024 Analog Devices, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_device, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/led.h>

#include "device.h"

#define SENSOR_CHAN     SENSOR_CHAN_AMBIENT_TEMP
#define SENSOR_UNIT     "Celsius"

/* Devices */
static const struct device *sensor = DEVICE_DT_GET(DT_ALIAS(ambient_temp0));
static const struct device *leds = DEVICE_DT_GET_ONE(gpio_leds);

/* Command handlers */
static void led_on_handler(void)
{
	device_write_led(LED_USER, LED_ON);
}

static void led_off_handler(void)
{
	device_write_led(LED_USER, LED_OFF);
}

/* Supported device commands */
struct device_cmd device_commands[] = {
	{"led_on", led_on_handler},
	{"led_off", led_off_handler}
};

const size_t num_device_commands = ARRAY_SIZE(device_commands);

/* Command dispatcher */
void device_command_handler(uint8_t *command)
{
	for (int i = 0; i < num_device_commands; i++) {
		if (strcmp(command, device_commands[i].command) == 0) {
			LOG_INF("Executing device command: %s", device_commands[i].command);
			return device_commands[i].handler();
		}
	}
	LOG_ERR("Unknown command: %s", command);
}

int device_read_sensor(struct sensor_sample *sample)
{
	int rc;
	struct sensor_value sensor_val;

	rc = sensor_sample_fetch(sensor);
	if (rc) {
		LOG_ERR("Failed to fetch sensor sample [%d]", rc);
		return rc;
	}

	rc = sensor_channel_get(sensor, SENSOR_CHAN, &sensor_val);
	if (rc) {
		LOG_ERR("Failed to get sensor channel [%d]", rc);
		return rc;
	}

	sample->unit = SENSOR_UNIT;
	sample->value = sensor_value_to_double(&sensor_val);

	return rc;
}

int device_write_led(enum led_id led_idx, enum led_state state)
{
	int rc;

	switch (state) {
	case LED_OFF:
		rc = led_off(leds, led_idx);
		break;
	case LED_ON:
		rc = led_on(leds, led_idx);
		break;
	default:
		LOG_ERR("Invalid LED state setting");
		rc = -EINVAL;
		break;
	}

	if (rc != 0) {
		LOG_ERR("Error updating LED [%d]", rc);
	}

	return rc;
}

bool devices_ready(void)
{
	bool rc = true;

	if (!device_is_ready(sensor)) {
		LOG_ERR("Device %s is not ready", sensor->name);
		rc = false;
	} else {
		LOG_INF("Device %s is ready", sensor->name);
	}

	if (!device_is_ready(leds)) {
		LOG_ERR("Device %s is not ready", leds->name);
		rc = false;
	} else {
		LOG_INF("Device %s is ready", leds->name);
	}

	return rc;
}
