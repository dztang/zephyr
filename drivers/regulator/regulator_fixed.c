/*
 * Copyright 2019-2020 Peter Bigot Consulting, LLC
 * Copyright 2022 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT regulator_fixed

#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(regulator_fixed, CONFIG_REGULATOR_LOG_LEVEL);

#define OPTION_ALWAYS_ON_POS 0
#define OPTION_ALWAYS_ON BIT(OPTION_ALWAYS_ON_POS)
#define OPTION_BOOT_ON_POS 1
#define OPTION_BOOT_ON BIT(OPTION_BOOT_ON_POS)

struct regulator_fixed_config {
	uint32_t startup_delay_us;
	uint32_t off_on_delay_us;
	struct gpio_dt_spec enable;
	uint8_t options;
};

struct regulator_fixed_data {
	struct onoff_sync_service srv;
};

static int regulator_fixed_enable(const struct device *dev,
				  struct onoff_client *cli)
{
	const struct regulator_fixed_config *cfg = dev->config;
	struct regulator_fixed_data *data = dev->data;
	k_spinlock_key_t key;
	int ret;

	if ((cfg->options & OPTION_ALWAYS_ON) != 0) {
		return 0;
	}

	ret = onoff_sync_lock(&data->srv, &key);
	if (ret > 0) {
		goto finalize;
	}

	ret = gpio_pin_set_dt(&cfg->enable, 1);
	if (ret < 0) {
		goto finalize;
	}

	if (cfg->off_on_delay_us > 0U) {
		k_sleep(K_USEC(cfg->off_on_delay_us));
	}

finalize:
	return onoff_sync_finalize(&data->srv, key, cli, ret, true);
}

static int regulator_fixed_disable(const struct device *dev)
{
	const struct regulator_fixed_config *cfg = dev->config;
	struct regulator_fixed_data *data = dev->data;
	k_spinlock_key_t key;
	int ret;

	if  ((cfg->options & OPTION_ALWAYS_ON) != 0) {
		return 0;
	}

	ret = onoff_sync_lock(&data->srv, &key);
	if (ret != 1) {
		goto finalize;
	}

	ret = gpio_pin_set_dt(&cfg->enable, 0);
	if (ret < 0) {
		return ret;
	}

finalize:
	return onoff_sync_finalize(&data->srv, key, NULL, ret, false);
}

static const struct regulator_driver_api regulator_fixed_api = {
	.enable = regulator_fixed_enable,
	.disable = regulator_fixed_disable,
};

static int regulator_fixed_init(const struct device *dev)
{
	const struct regulator_fixed_config *cfg = dev->config;
	int ret;

	if (!device_is_ready(cfg->enable.port)) {
		LOG_ERR("GPIO port: %s not ready", cfg->enable.port->name);
		return -ENODEV;
	}

	if ((cfg->options & (OPTION_ALWAYS_ON | OPTION_BOOT_ON)) != 0U) {
		ret = gpio_pin_configure_dt(&cfg->enable, GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			return ret;
		}

		k_busy_wait(cfg->startup_delay_us);
	} else {
		ret = gpio_pin_configure_dt(&cfg->enable, GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

#define REGULATOR_FIXED_DEFINE(inst)                                           \
	static const struct regulator_fixed_config config##inst = {            \
		.startup_delay_us = DT_INST_PROP(inst, startup_delay_us),      \
		.off_on_delay_us = DT_INST_PROP(inst, off_on_delay_us),        \
		.enable = GPIO_DT_SPEC_INST_GET(inst, enable_gpios),           \
		.options = (DT_INST_PROP(inst, regulator_boot_on)              \
			    << OPTION_BOOT_ON_POS) |                           \
			   (DT_INST_PROP(inst, regulator_always_on)            \
			    << OPTION_ALWAYS_ON_POS),                          \
	};                                                                     \
                                                                               \
	DEVICE_DT_INST_DEFINE(inst, regulator_fixed_init, NULL, NULL,          \
			      &config##inst, POST_KERNEL,                      \
			      CONFIG_REGULATOR_FIXED_INIT_PRIORITY,            \
			      &regulator_fixed_api);

DT_INST_FOREACH_STATUS_OKAY(REGULATOR_FIXED_DEFINE)
