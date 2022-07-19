/*
 * Copyright (c) 2022 Intel Corporation
 * Copyright (c) 2022 Esco Medical ApS
 * Copyright (c) 2020 TDK Invensense
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT invensense_icm42688

#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/byteorder.h>
#include "icm42688.h"
#include "icm42688_reg.h"
#include "icm42688_spi.h"
#include "icm42688_trigger.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ICM42688, CONFIG_SENSOR_LOG_LEVEL);

struct icm42688_sensor_data {
	struct icm42688_dev_data dev_data;

	int16_t readings[7];
};

struct icm42688_sensor_config {
	struct icm42688_dev_cfg dev_cfg;
};

static void icm42688_convert_accel(struct sensor_value *val, int16_t raw_val,
				   struct icm42688_cfg *cfg)
{
	icm42688_accel_ms(cfg, (int32_t)raw_val, &val->val1, &val->val2);
}

static void icm42688_convert_gyro(struct sensor_value *val, int16_t raw_val,
				  struct icm42688_cfg *cfg)
{
	icm42688_gyro_rads(cfg, (int32_t)raw_val, &val->val1, &val->val2);
}

static inline void icm42688_convert_temp(struct sensor_value *val, int16_t raw_val)
{
	icm42688_temp_c((int32_t)raw_val, &val->val1, &val->val2);
}

static int icm42688_channel_get(const struct device *dev, enum sensor_channel chan,
				struct sensor_value *val)
{
	int res = 0;
	struct icm42688_sensor_data *data = dev->data;

	icm42688_lock(dev);

	switch (chan) {
	case SENSOR_CHAN_ACCEL_XYZ:
		icm42688_convert_accel(&val[0], data->readings[1], &data->dev_data.cfg);
		icm42688_convert_accel(&val[1], data->readings[2], &data->dev_data.cfg);
		icm42688_convert_accel(&val[2], data->readings[3], &data->dev_data.cfg);
		break;
	case SENSOR_CHAN_ACCEL_X:
		icm42688_convert_accel(val, data->readings[1], &data->dev_data.cfg);
		break;
	case SENSOR_CHAN_ACCEL_Y:
		icm42688_convert_accel(val, data->readings[2], &data->dev_data.cfg);
		break;
	case SENSOR_CHAN_ACCEL_Z:
		icm42688_convert_accel(val, data->readings[3], &data->dev_data.cfg);
		break;
	case SENSOR_CHAN_GYRO_XYZ:
		icm42688_convert_gyro(&val[0], data->readings[4], &data->dev_data.cfg);
		icm42688_convert_gyro(&val[1], data->readings[5], &data->dev_data.cfg);
		icm42688_convert_gyro(&val[2], data->readings[6], &data->dev_data.cfg);
		break;
	case SENSOR_CHAN_GYRO_X:
		icm42688_convert_gyro(val, data->readings[4], &data->dev_data.cfg);
		break;
	case SENSOR_CHAN_GYRO_Y:
		icm42688_convert_gyro(val, data->readings[5], &data->dev_data.cfg);
		break;
	case SENSOR_CHAN_GYRO_Z:
		icm42688_convert_gyro(val, data->readings[6], &data->dev_data.cfg);
		break;
	case SENSOR_CHAN_DIE_TEMP:
		icm42688_convert_temp(val, data->readings[0]);
		break;
	default:
		res = -ENOTSUP;
		break;
	}

	icm42688_unlock(dev);

	return res;
}

static int icm42688_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	uint8_t status;
	struct icm42688_sensor_data *data = dev->data;
	const struct icm42688_sensor_config *cfg = dev->config;

	icm42688_lock(dev);

	int res = icm42688_spi_read(&cfg->dev_cfg.spi, REG_INT_STATUS, &status, 1);

	if (res) {
		goto cleanup;
	}

	if (!FIELD_GET(BIT_INT_STATUS_DATA_RDY, status)) {
		res = -EBUSY;
		goto cleanup;
	}

	uint8_t readings[14];

	res = icm42688_read_all(dev, readings);

	if (res == 0) {
		for (int i = 0; i < 7; i++) {
			data->readings[i] =
				sys_le16_to_cpu((readings[i * 2] << 8) | readings[i * 2 + 1]);
		}
	}

cleanup:
	icm42688_unlock(dev);
	return res;
}

static int icm42688_attr_set(const struct device *dev, enum sensor_channel chan,
			     enum sensor_attribute attr, const struct sensor_value *val)
{
	return -ENOTSUP;
}

static int icm42688_attr_get(const struct device *dev, enum sensor_channel chan,
			     enum sensor_attribute attr, struct sensor_value *val)
{
	return -ENOTSUP;
}

static const struct sensor_driver_api icm42688_driver_api = {
#ifdef CONFIG_ICM42688_TRIGGER
	.trigger_set = icm42688_trigger_set,
#endif
	.sample_fetch = icm42688_sample_fetch,
	.channel_get = icm42688_channel_get,
	.attr_set = icm42688_attr_set,
	.attr_get = icm42688_attr_get,
};

int icm42688_init(const struct device *dev)
{
	struct icm42688_sensor_data *data = dev->data;
	const struct icm42688_sensor_config *cfg = dev->config;
	int res;

	if (!spi_is_ready(&cfg->dev_cfg.spi)) {
		LOG_ERR("SPI bus is not ready");
		return -ENODEV;
	}

	if (icm42688_reset(dev)) {
		LOG_ERR("could not initialize sensor");
		return -EIO;
	}

#ifdef CONFIG_ICM42688_TRIGGER
	if (icm42688_trigger_init(dev)) {
		LOG_ERR("Failed to initialize interrupts.");
		return -EIO;
	}
#endif

	data->dev_data.cfg.accel_mode = ICM42688_ACCEL_LN;
	data->dev_data.cfg.gyro_mode = ICM42688_GYRO_LN;
	data->dev_data.cfg.accel_fs = ICM42688_ACCEL_FS_2G;
	data->dev_data.cfg.gyro_fs = ICM42688_GYRO_FS_125;
	data->dev_data.cfg.accel_odr = ICM42688_ACCEL_ODR_1000;
	data->dev_data.cfg.gyro_odr = ICM42688_GYRO_ODR_1000;

	res = icm42688_configure(dev, &data->dev_data.cfg);
	if (res != 0) {
		LOG_ERR("Failed to configure");
	}

#ifdef CONFIG_ICM42688_TRIGGER
	if (icm42688_trigger_enable_interrupt(dev)) {
		LOG_ERR("Failed to enable interrupts");
		return -EIO;
	}
#endif

	return res;
}
/* device defaults to spi mode 0/3 support */
#define ICM42688_SPI_CFG                                                                           \
	SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_TRANSFER_MSB

#define ICM42688_INIT(inst)                                                                        \
	static struct icm42688_sensor_data icm42688_driver_##inst = {};                            \
                                                                                                   \
	static const struct icm42688_sensor_config icm42688_cfg_##inst = {                         \
		.dev_cfg =                                                                         \
			{                                                                          \
				.spi = SPI_DT_SPEC_INST_GET(inst, ICM42688_SPI_CFG, 0U),           \
				.gpio_int1 = GPIO_DT_SPEC_INST_GET_OR(inst, int_gpios, {0}),       \
			},                                                                         \
	};                                                                                         \
                                                                                                   \
	SENSOR_DEVICE_DT_INST_DEFINE(inst, icm42688_init, NULL, &icm42688_driver_##inst,           \
				     &icm42688_cfg_##inst, POST_KERNEL,                            \
				     CONFIG_SENSOR_INIT_PRIORITY, &icm42688_driver_api);

DT_INST_FOREACH_STATUS_OKAY(ICM42688_INIT)
