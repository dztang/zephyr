/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <sys/util.h>

#include "disp.h"

const struct device *display;
const struct device *sensor_dev;

#include <logging/log.h>
LOG_MODULE_REGISTER(main);


#if CONFIG_LOG
#define sensor_log(...) LOG_INF(__VA_ARGS__)
#else
#define sensor_log(fmt, ...) printk(fmt, ##__VA_ARGS__)
#endif

static void process_sample(void)
{
	struct sensor_value gas, temp, hum, press;

	if (sensor_sample_fetch(sensor_dev) < 0) {
		LOG_ERR("Sensor sample update error");
		return;
	}
#if CONFIG_HAS_TEMPERATURE
	if (sensor_channel_get(sensor_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
		LOG_ERR("Cannot read temperature channel");
		return;
	}
#endif
#if CONFIG_HAS_HUMIDITY
	if (sensor_channel_get(sensor_dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
		LOG_ERR("Cannot read humidity channel");
		return;
	}
#endif
#if CONFIG_HAS_PRESSURE
	if (sensor_channel_get(sensor_dev, SENSOR_CHAN_PRESS, &press) < 0) {
		LOG_ERR("Cannot read pressure channel");
		return;
	}
#endif
#if CONFIG_HAS_GAS
	if (sensor_channel_get(sensor_dev, SENSOR_CHAN_GAS_RES, &gas) < 0) {
		LOG_ERR("Cannot read gas channel");
		return;
	}
#endif


#ifndef CONFIG_SAMPLE_DISPLAY_NONE
	update_display(temp, hum, press, gas);
#endif

	if (IS_ENABLED(CONFIG_HAS_TEMPERATURE)) {
		sensor_log("Temperature: %.1f C\n", sensor_value_to_double(&temp));
	}
	if (IS_ENABLED(CONFIG_HAS_HUMIDITY)) {
		sensor_log("Relative Humidity: %.1f%%\n", sensor_value_to_double(&hum));
	}
	if (IS_ENABLED(CONFIG_HAS_PRESSURE)) {
		sensor_log("Pressure: %.2f kPa", sensor_value_to_double(&press));
	}
	if (IS_ENABLED(CONFIG_HAS_GAS)) {
		sensor_log("Gas: %.2f", sensor_value_to_double(&gas));
	}
}

static void handler_(const struct device *dev,
			   struct sensor_trigger *trig)
{
	process_sample();
}


void main(void)
{

#if defined(CONFIG_HTS221)
	sensor_dev = DEVICE_DT_GET_ANY(st_hts221);
#elif defined(CONFIG_TH02)
	sensor_dev = DEVICE_DT_GET_ANY(hoperf_th02);
#elif defined(CONFIG_BME280)
	sensor_dev = DEVICE_DT_GET_ANY(bosch_bme280);
#elif defined(CONFIG_TMP007)
	sensor_dev = DEVICE_DT_GET_ANY(ti_tmp007);
#elif defined(CONFIG_SHT3XD)
	sensor_dev = DEVICE_DT_GET_ANY(sensirion_sht3xd);
#elif defined(CONFIG_BME680)
	sensor_dev = DEVICE_DT_GET_ANY(bosch_bme680);
#elif defined(CONFIG_MCP9808)
	sensor_dev = DEVICE_DT_GET_ANY(microchip_mcp9808);
#elif defined(CONFIG_DPS310)
	sensor_dev = DEVICE_DT_GET_ANY(infineon_dps310);
#elif defined(CONFIG_TI_HDC)
	sensor_dev = DEVICE_DT_GET_ANY(ti_hdc);
#elif defined(CONFIG_MAX6675)
	sensor_dev = DEVICE_DT_GET_ANY(maxim_max6675);
#elif defined(CONFIG_LPS22HB)
	sensor_dev = DEVICE_DT_GET_ANY(st_lps22hb_press);
#elif defined(CONFIG_LPS22HH)
	sensor_dev = DEVICE_DT_GET_ANY(st_lps22hh);
#elif defined(CONFIG_ENS210)
	sensor_dev = DEVICE_DT_GET_ANY(ams_ens210);
#elif defined(CONFIG_DHT)
	sensor_dev = DEVICE_DT_GET_ANY(aosong_dht);
#elif defined(CONFIG_SI7006)
	sensor_dev = DEVICE_DT_GET_ANY(silabs_si7006);
#else
#error Not defined
#endif

	if (sensor_dev == NULL) {
		LOG_ERR("Could not get sensor device");
		return;
	}
	if (!device_is_ready(sensor_dev)) {
		LOG_ERR("Device %s is not ready.", sensor_dev->name);
		return;
	}


#ifdef CONFIG_SAMPLE_SENSOR_TRIGGER
	rc = set_window_ucel(sensor_dev, TEMP_INITIAL_CEL * UCEL_PER_CEL);
	if (rc == 0) {
		trig.type = SENSOR_TRIG_THRESHOLD;
		trig.chan = SENSOR_CHAN_AMBIENT_TEMP;
		rc = sensor_trigger_set(dev, &trig, trigger_handler);
	}

	if (rc != 0) {
		printf("Trigger set failed: %d\n", rc);
		return;
	}
	printk("Trigger set got %d\n", rc);
#endif

#ifndef CONFIG_SAMPLE_DISPLAY_NONE
	init_display();
	reset_display();
#endif

	if (IS_ENABLED(CONFIG_SAMPLE_SENSOR_TRIGGER)) {
		struct sensor_trigger trig = {
			.type = SENSOR_TRIG_DATA_READY,
			.chan = SENSOR_CHAN_ALL,
		};
		if (sensor_trigger_set(sensor_dev, &trig, handler_) < 0) {
			LOG_ERR("Cannot configure trigger");
			return;
		}
	}

	while (!IS_ENABLED(CONFIG_SAMPLE_SENSOR_TRIGGER)) {
		process_sample();
		k_sleep(K_MSEC(2000));
	}
	k_sleep(K_FOREVER);
}
