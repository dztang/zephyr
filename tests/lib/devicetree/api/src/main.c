/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Override __DEPRECATED_MACRO so we don't get twister failures for
 * deprecated macros:
 * - DT_BUS_LABEL
 * - DT_SPI_DEV_CS_GPIOS_LABEL
 * - DT_GPIO_LABEL
 * - DT_GPIO_LABEL_BY_IDX
 * - DT_INST_BUS_LABEL
 * - DT_INST_SPI_DEV_CS_GPIOS_LABEL
 * - DT_INST_GPIO_LABEL
 * - DT_INST_GPIO_LABEL_BY_IDX
 */
#define __DEPRECATED_MACRO

#include <zephyr/ztest.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/mbox.h>

#define TEST_CHILDREN	DT_PATH(test, test_children)
#define TEST_DEADBEEF	DT_PATH(test, gpio_deadbeef)
#define TEST_ABCD1234	DT_PATH(test, gpio_abcd1234)
#define TEST_ALIAS	DT_ALIAS(test_alias)
#define TEST_NODELABEL	DT_NODELABEL(test_nodelabel)
#define TEST_INST	DT_INST(0, vnd_gpio_device)
#define TEST_ARRAYS	DT_NODELABEL(test_arrays)
#define TEST_PH	DT_NODELABEL(test_phandles)
#define TEST_IRQ	DT_NODELABEL(test_irq)
#define TEST_TEMP	DT_NODELABEL(test_temp_sensor)
#define TEST_REG	DT_NODELABEL(test_reg)
#define TEST_ENUM_0	DT_NODELABEL(test_enum_0)

#define TEST_I2C DT_NODELABEL(test_i2c)
#define TEST_I2C_DEV DT_PATH(test, i2c_11112222, test_i2c_dev_10)
#define TEST_I2C_BUS DT_BUS(TEST_I2C_DEV)

#define TEST_I2C_MUX DT_NODELABEL(test_i2c_mux)
#define TEST_I2C_MUX_CTLR_1 DT_CHILD(TEST_I2C_MUX, i2c_mux_ctlr_1)
#define TEST_I2C_MUX_CTLR_2 DT_CHILD(TEST_I2C_MUX, i2c_mux_ctlr_2)
#define TEST_MUXED_I2C_DEV_1 DT_NODELABEL(test_muxed_i2c_dev_1)
#define TEST_MUXED_I2C_DEV_2 DT_NODELABEL(test_muxed_i2c_dev_2)

#define TEST_GPIO_1 DT_NODELABEL(test_gpio_1)
#define TEST_GPIO_2 DT_NODELABEL(test_gpio_2)

#define TEST_SPI DT_NODELABEL(test_spi)

#define TEST_SPI_DEV_0 DT_PATH(test, spi_33334444, test_spi_dev_0)
#define TEST_SPI_BUS_0 DT_BUS(TEST_SPI_DEV_0)

#define TEST_SPI_DEV_1 DT_PATH(test, spi_33334444, test_spi_dev_1)
#define TEST_SPI_BUS_1 DT_BUS(TEST_SPI_DEV_1)

#define TEST_SPI_NO_CS DT_NODELABEL(test_spi_no_cs)
#define TEST_SPI_DEV_NO_CS DT_NODELABEL(test_spi_no_cs)

#define TEST_PWM_CTLR_1 DT_NODELABEL(test_pwm1)
#define TEST_PWM_CTLR_2 DT_NODELABEL(test_pwm2)

#define TEST_CAN_CTRL_0 DT_NODELABEL(test_can0)
#define TEST_CAN_CTRL_1 DT_NODELABEL(test_can1)

#define TEST_DMA_CTLR_1 DT_NODELABEL(test_dma1)
#define TEST_DMA_CTLR_2 DT_NODELABEL(test_dma2)

#define TEST_IO_CHANNEL_CTLR_1 DT_NODELABEL(test_adc_1)
#define TEST_IO_CHANNEL_CTLR_2 DT_NODELABEL(test_adc_2)

#define TEST_RANGES_PCIE  DT_NODELABEL(test_ranges_pcie)
#define TEST_RANGES_OTHER DT_NODELABEL(test_ranges_other)

#define TA_HAS_COMPAT(compat) DT_NODE_HAS_COMPAT(TEST_ARRAYS, compat)

#define TO_STRING(x) TO_STRING_(x)
#define TO_STRING_(x) #x

ZTEST(devicetree_api, test_path_props)
{
	zassert_true(!strcmp(DT_LABEL(TEST_DEADBEEF), "TEST_GPIO_1"), "");
	zassert_equal(DT_NUM_REGS(TEST_DEADBEEF), 1, "");
	zassert_equal(DT_REG_ADDR(TEST_DEADBEEF), 0xdeadbeef, "");
	zassert_equal(DT_REG_SIZE(TEST_DEADBEEF), 0x1000, "");
	zassert_equal(DT_PROP(TEST_DEADBEEF, gpio_controller), 1, "");
	zassert_equal(DT_PROP(TEST_DEADBEEF, ngpios), 32, "");
	zassert_true(!strcmp(DT_PROP(TEST_DEADBEEF, status), "okay"), "");
	zassert_equal(DT_PROP_LEN(TEST_DEADBEEF, compatible), 1, "");
	zassert_true(!strcmp(DT_PROP_BY_IDX(TEST_DEADBEEF, compatible, 0),
			     "vnd,gpio-device"), "");
	zassert_true(DT_NODE_HAS_PROP(TEST_DEADBEEF, status), "");
	zassert_false(DT_NODE_HAS_PROP(TEST_DEADBEEF, foobar), "");

	zassert_true(DT_SAME_NODE(TEST_ABCD1234, TEST_GPIO_2), "");
	zassert_equal(DT_NUM_REGS(TEST_ABCD1234), 2, "");
	zassert_equal(DT_PROP(TEST_ABCD1234, gpio_controller), 1, "");
	zassert_equal(DT_PROP(TEST_ABCD1234, ngpios), 32, "");
	zassert_true(!strcmp(DT_PROP(TEST_ABCD1234, status), "okay"), "");
	zassert_equal(DT_PROP_LEN(TEST_ABCD1234, compatible), 1, "");
	zassert_equal(DT_PROP_LEN_OR(TEST_ABCD1234, compatible, 4), 1, "");
	zassert_equal(DT_PROP_LEN_OR(TEST_ABCD1234, invalid_property, 0), 0, "");
	zassert_true(!strcmp(DT_PROP_BY_IDX(TEST_ABCD1234, compatible, 0),
			     "vnd,gpio-device"), "");
}

ZTEST(devicetree_api, test_alias_props)
{
	zassert_equal(DT_NUM_REGS(TEST_ALIAS), 1, "");
	zassert_equal(DT_REG_ADDR(TEST_ALIAS), 0xdeadbeef, "");
	zassert_equal(DT_REG_SIZE(TEST_ALIAS), 0x1000, "");
	zassert_true(DT_SAME_NODE(TEST_ALIAS, TEST_GPIO_1), "");
	zassert_equal(DT_PROP(TEST_ALIAS, gpio_controller), 1, "");
	zassert_equal(DT_PROP(TEST_ALIAS, ngpios), 32, "");
	zassert_true(!strcmp(DT_PROP(TEST_ALIAS, status), "okay"), "");
	zassert_equal(DT_PROP_LEN(TEST_ALIAS, compatible), 1, "");
	zassert_true(!strcmp(DT_PROP_BY_IDX(TEST_ALIAS, compatible, 0),
			     "vnd,gpio-device"), "");
}

ZTEST(devicetree_api, test_nodelabel_props)
{
	zassert_equal(DT_NUM_REGS(TEST_NODELABEL), 1, "");
	zassert_equal(DT_REG_ADDR(TEST_NODELABEL), 0xdeadbeef, "");
	zassert_equal(DT_REG_SIZE(TEST_NODELABEL), 0x1000, "");
	zassert_true(!strcmp(DT_LABEL(TEST_NODELABEL), "TEST_GPIO_1"), "");
	zassert_equal(DT_PROP(TEST_NODELABEL, gpio_controller), 1, "");
	zassert_equal(DT_PROP(TEST_NODELABEL, ngpios), 32, "");
	zassert_true(!strcmp(DT_PROP(TEST_NODELABEL, status), "okay"), "");
	zassert_equal(DT_PROP_LEN(TEST_NODELABEL, compatible), 1, "");
	zassert_true(!strcmp(DT_PROP_BY_IDX(TEST_NODELABEL, compatible, 0),
			     "vnd,gpio-device"), "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_gpio_device
ZTEST(devicetree_api, test_inst_props)
{
	const char *label_startswith = "TEST_GPIO_";

	/*
	 * Careful:
	 *
	 * We can only test properties that are shared across all
	 * instances of this compatible here. This includes instances
	 * with status "disabled".
	 */

	zassert_equal(DT_PROP(TEST_INST, gpio_controller), 1, "");
	zassert_true(!strcmp(DT_PROP(TEST_INST, status), "okay") ||
		     !strcmp(DT_PROP(TEST_INST, status), "disabled"), "");
	zassert_equal(DT_PROP_LEN(TEST_INST, compatible), 1, "");
	zassert_true(!strcmp(DT_PROP_BY_IDX(TEST_INST, compatible, 0),
			     "vnd,gpio-device"), "");

	zassert_equal(DT_INST_NODE_HAS_PROP(0, gpio_controller), 1, "");
	zassert_equal(DT_INST_PROP(0, gpio_controller), 1, "");
	zassert_equal(DT_INST_NODE_HAS_PROP(0, xxxx), 0, "");
	zassert_true(!strcmp(DT_INST_PROP(0, status), "okay") ||
		     !strcmp(DT_PROP(TEST_INST, status), "disabled"), "");
	zassert_equal(DT_INST_PROP_LEN(0, compatible), 1, "");
	zassert_true(!strcmp(DT_INST_PROP_BY_IDX(0, compatible, 0),
			     "vnd,gpio-device"), "");
	zassert_true(!strncmp(label_startswith, DT_INST_LABEL(0),
			      strlen(label_startswith)), "");
}

ZTEST(devicetree_api, test_default_prop_access)
{
	/*
	 * The APIs guarantee that the default_value is not expanded
	 * if the relevant property or cell is defined. This "X" macro
	 * is meant as poison which causes (hopefully) easy to
	 * understand build errors if this guarantee is not met due to
	 * a regression.
	 */
#undef X
#define X do.not.expand.this.argument

	/* Node identifier variants. */
	zassert_equal(DT_PROP_OR(TEST_REG, misc_prop, X), 1234, "");
	zassert_equal(DT_PROP_OR(TEST_REG, not_a_property, -1), -1, "");

	zassert_equal(DT_PHA_BY_IDX_OR(TEST_TEMP, dmas, 1, channel, X), 3, "");
	zassert_equal(DT_PHA_BY_IDX_OR(TEST_TEMP, dmas, 1, not_a_cell, -1), -1,
		      "");

	zassert_equal(DT_PHA_OR(TEST_TEMP, dmas, channel, X), 1, "");
	zassert_equal(DT_PHA_OR(TEST_TEMP, dmas, not_a_cell, -1), -1, "");

	zassert_equal(DT_PHA_BY_NAME_OR(TEST_TEMP, dmas, tx, channel, X), 1,
		      "");
	zassert_equal(DT_PHA_BY_NAME_OR(TEST_TEMP, dmas, tx, not_a_cell, -1),
		      -1, "");

	/* Instance number variants. */
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_reg_holder
	zassert_equal(DT_INST_PROP_OR(0, misc_prop, X), 1234, "");
	zassert_equal(DT_INST_PROP_OR(0, not_a_property, -1), -1, "");

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_adc_temp_sensor
	zassert_equal(DT_INST_PHA_BY_IDX_OR(0, dmas, 1, channel, X), 3, "");
	zassert_equal(DT_INST_PHA_BY_IDX_OR(0, dmas, 1, not_a_cell, -1), -1,
		      "");

	zassert_equal(DT_INST_PHA_OR(0, dmas, channel, X), 1, "");
	zassert_equal(DT_INST_PHA_OR(0, dmas, not_a_cell, -1), -1, "");

	zassert_equal(DT_INST_PHA_BY_NAME_OR(0, dmas, tx, channel, X), 1,
		      "");
	zassert_equal(DT_INST_PHA_BY_NAME_OR(0, dmas, tx, not_a_cell, -1), -1,
		      "");

#undef X
}

ZTEST(devicetree_api, test_has_path)
{
	zassert_equal(DT_NODE_HAS_STATUS(DT_PATH(test, gpio_0), okay), 0, "");
	zassert_equal(DT_NODE_HAS_STATUS(DT_PATH(test, gpio_deadbeef), okay), 1,
		      "");
	zassert_equal(DT_NODE_HAS_STATUS(DT_PATH(test, gpio_abcd1234), okay), 1,
		      "");
}

ZTEST(devicetree_api, test_has_alias)
{
	zassert_equal(DT_NODE_HAS_STATUS(DT_ALIAS(test_alias), okay), 1, "");
	zassert_equal(DT_NODE_HAS_STATUS(DT_ALIAS(test_undef), okay), 0, "");
}

ZTEST(devicetree_api, test_inst_checks)
{
	zassert_equal(DT_NODE_EXISTS(DT_INST(0, vnd_gpio_device)), 1, "");
	zassert_equal(DT_NODE_EXISTS(DT_INST(1, vnd_gpio_device)), 1, "");
	zassert_equal(DT_NODE_EXISTS(DT_INST(2, vnd_gpio_device)), 1, "");

	zassert_equal(DT_NUM_INST_STATUS_OKAY(vnd_gpio_device), 2, "");
	zassert_equal(DT_NUM_INST_STATUS_OKAY(xxxx), 0, "");
}

ZTEST(devicetree_api, test_has_nodelabel)
{
	zassert_equal(DT_NODE_HAS_STATUS(DT_NODELABEL(disabled_gpio), okay), 0,
		      "");
	zassert_equal(DT_NODE_HAS_STATUS(DT_NODELABEL(test_nodelabel), okay), 1,
		      "");
	zassert_equal(DT_NODE_HAS_STATUS(DT_NODELABEL(test_nodelabel_allcaps),
					 okay),
		      1, "");
}

ZTEST(devicetree_api, test_has_compat)
{
	unsigned int compats;

	zassert_true(DT_HAS_COMPAT_STATUS_OKAY(vnd_gpio_device), "");
	zassert_true(DT_HAS_COMPAT_STATUS_OKAY(vnd_gpio_device), "");
	zassert_false(DT_HAS_COMPAT_STATUS_OKAY(vnd_disabled_compat), "");

	zassert_equal(TA_HAS_COMPAT(vnd_array_holder), 1, "");
	zassert_equal(TA_HAS_COMPAT(vnd_undefined_compat), 1, "");
	zassert_equal(TA_HAS_COMPAT(vnd_not_a_test_array_compat), 0, "");
	compats = ((TA_HAS_COMPAT(vnd_array_holder) << 0) |
		   (TA_HAS_COMPAT(vnd_undefined_compat) << 1) |
		   (TA_HAS_COMPAT(vnd_not_a_test_array_compat) << 2));
	zassert_equal(compats, 0x3, "");
}

ZTEST(devicetree_api, test_has_status)
{
	zassert_equal(DT_NODE_HAS_STATUS(DT_NODELABEL(test_gpio_1), okay),
		      1, "");
	zassert_equal(DT_NODE_HAS_STATUS(DT_NODELABEL(test_gpio_1), disabled),
		      0, "");

	zassert_equal(DT_NODE_HAS_STATUS(DT_NODELABEL(test_no_status), okay),
		      1, "");
	zassert_equal(DT_NODE_HAS_STATUS(DT_NODELABEL(test_no_status), disabled),
		      0, "");

	zassert_equal(DT_NODE_HAS_STATUS(DT_NODELABEL(disabled_gpio), disabled),
		      1, "");
	zassert_equal(DT_NODE_HAS_STATUS(DT_NODELABEL(disabled_gpio), okay),
		      0, "");
}

ZTEST(devicetree_api, test_bus)
{
	/* common prefixes of expected labels: */
	const char *i2c_bus = "TEST_I2C_CTLR";
	const char *i2c_dev = "TEST_I2C_DEV";
	const char *spi_bus = "TEST_SPI_CTLR";
	const char *spi_dev = "TEST_SPI_DEV";
	const char *gpio = "TEST_GPIO_";
	int pin, flags;

	zassert_true(DT_SAME_NODE(TEST_I2C_BUS, TEST_I2C), "");
	zassert_true(DT_SAME_NODE(TEST_SPI_BUS_0, TEST_SPI), "");
	zassert_true(DT_SAME_NODE(TEST_SPI_BUS_1, TEST_SPI), "");

	zassert_equal(DT_SPI_DEV_HAS_CS_GPIOS(TEST_SPI_DEV_0), 1, "");
	zassert_equal(DT_SPI_DEV_HAS_CS_GPIOS(TEST_SPI_DEV_NO_CS), 0, "");

	/* Test a nested I2C bus using vnd,i2c-mux. */
	zassert_true(DT_SAME_NODE(TEST_I2C_MUX_CTLR_1,
				  DT_BUS(TEST_MUXED_I2C_DEV_1)), "");
	zassert_true(DT_SAME_NODE(TEST_I2C_MUX_CTLR_2,
				  DT_BUS(TEST_MUXED_I2C_DEV_2)), "");

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_spi_device_2
	/* there is only one instance, and it has no CS */
	zassert_equal(DT_INST_SPI_DEV_HAS_CS_GPIOS(0), 0, "");
	/* since there's only one instance, we also know its bus. */
	zassert_true(DT_SAME_NODE(TEST_SPI_NO_CS, DT_INST_BUS(0)),
		     "expected TEST_SPI_NO_CS as bus for vnd,spi-device-2");

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_spi_device
	/*
	 * DT_INST_SPI_DEV: use with care here. We could be matching
	 * either vnd,spi-device.
	 */
	zassert_equal(DT_INST_SPI_DEV_HAS_CS_GPIOS(0), 1, "");

#define CTLR_NODE DT_INST_SPI_DEV_CS_GPIOS_CTLR(0)
	zassert_true(DT_SAME_NODE(CTLR_NODE, DT_NODELABEL(test_gpio_1)) ||
		     DT_SAME_NODE(CTLR_NODE, DT_NODELABEL(test_gpio_2)), "");
#undef CTLR_NODE

	zassert_true(!strncmp(gpio, DT_INST_SPI_DEV_CS_GPIOS_LABEL(0),
			      strlen(gpio)), "");

	pin = DT_INST_SPI_DEV_CS_GPIOS_PIN(0);
	zassert_true((pin == 0x10) || (pin == 0x30), "");

	flags = DT_INST_SPI_DEV_CS_GPIOS_FLAGS(0);
	zassert_true((flags == 0x20) || (flags == 0x40), "");

	zassert_equal(DT_ON_BUS(TEST_SPI_DEV_0, spi), 1, "");
	zassert_equal(DT_ON_BUS(TEST_SPI_DEV_0, i2c), 0, "");

	zassert_equal(DT_ON_BUS(TEST_I2C_DEV, i2c), 1, "");
	zassert_equal(DT_ON_BUS(TEST_I2C_DEV, spi), 0, "");

	zassert_true(!strcmp(DT_BUS_LABEL(TEST_I2C_DEV), "TEST_I2C_CTLR"), "");

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_spi_device
	zassert_equal(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT), 2, "");

	zassert_equal(DT_INST_ON_BUS(0, spi), 1, "");
	zassert_equal(DT_INST_ON_BUS(0, i2c), 0, "");

	zassert_equal(DT_ANY_INST_ON_BUS_STATUS_OKAY(spi), 1, "");
	zassert_equal(DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c), 0, "");

	zassert_true(!strncmp(spi_dev, DT_INST_LABEL(0), strlen(spi_dev)), "");
	zassert_true(!strncmp(spi_bus, DT_INST_BUS_LABEL(0), strlen(spi_bus)),
		     "");

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_i2c_device
	zassert_equal(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT), 2, "");

	zassert_equal(DT_INST_ON_BUS(0, i2c), 1, "");
	zassert_equal(DT_INST_ON_BUS(0, spi), 0, "");

	zassert_equal(DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c), 1, "");
	zassert_equal(DT_ANY_INST_ON_BUS_STATUS_OKAY(spi), 0, "");

	zassert_true(!strncmp(i2c_dev, DT_INST_LABEL(0), strlen(i2c_dev)), "");
	zassert_true(!strncmp(i2c_bus, DT_INST_BUS_LABEL(0), strlen(i2c_bus)),
		     "");

#undef DT_DRV_COMPAT
	/*
	 * Make sure the underlying DT_COMPAT_ON_BUS_INTERNAL used by
	 * DT_ANY_INST_ON_BUS works without DT_DRV_COMPAT defined.
	 */
	zassert_equal(DT_COMPAT_ON_BUS_INTERNAL(vnd_spi_device, spi), 1, NULL);
	zassert_equal(DT_COMPAT_ON_BUS_INTERNAL(vnd_spi_device, i2c), 0, NULL);

	zassert_equal(DT_COMPAT_ON_BUS_INTERNAL(vnd_i2c_device, i2c), 1, NULL);
	zassert_equal(DT_COMPAT_ON_BUS_INTERNAL(vnd_i2c_device, spi), 0, NULL);

	zassert_equal(DT_COMPAT_ON_BUS_INTERNAL(vnd_gpio_expander, i2c), 1,
		      NULL);
	zassert_equal(DT_COMPAT_ON_BUS_INTERNAL(vnd_gpio_expander, spi), 1,
		      NULL);
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_reg_holder
ZTEST(devicetree_api, test_reg)
{
	/* DT_REG_HAS_IDX */
	zassert_true(DT_REG_HAS_IDX(TEST_ABCD1234, 0), "");
	zassert_true(DT_REG_HAS_IDX(TEST_ABCD1234, 1), "");
	zassert_false(DT_REG_HAS_IDX(TEST_ABCD1234, 2), "");

	/* DT_REG_ADDR_BY_IDX */
	zassert_equal(DT_REG_ADDR_BY_IDX(TEST_ABCD1234, 0), 0xabcd1234, "");
	zassert_equal(DT_REG_ADDR_BY_IDX(TEST_ABCD1234, 1), 0x98765432, "");

	/* DT_REG_SIZE_BY_IDX */
	zassert_equal(DT_REG_SIZE_BY_IDX(TEST_ABCD1234, 0), 0x500, "");
	zassert_equal(DT_REG_SIZE_BY_IDX(TEST_ABCD1234, 1), 0xff, "");

	/* DT_REG_ADDR */
	zassert_equal(DT_REG_ADDR(TEST_ABCD1234), 0xabcd1234, "");

	/* DT_REG_SIZE */
	zassert_equal(DT_REG_SIZE(TEST_ABCD1234), 0x500, "");

	/* DT_REG_ADDR_BY_NAME */
	zassert_equal(DT_REG_ADDR_BY_NAME(TEST_ABCD1234, one), 0xabcd1234, "");
	zassert_equal(DT_REG_ADDR_BY_NAME(TEST_ABCD1234, two), 0x98765432, "");

	/* DT_REG_SIZE_BY_NAME */
	zassert_equal(DT_REG_SIZE_BY_NAME(TEST_ABCD1234, one), 0x500, "");
	zassert_equal(DT_REG_SIZE_BY_NAME(TEST_ABCD1234, two), 0xff, "");

	/* DT_INST */
	zassert_equal(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT), 1, "");

	/* DT_INST_REG_HAS_IDX */
	zassert_true(DT_INST_REG_HAS_IDX(0, 0), "");
	zassert_true(DT_INST_REG_HAS_IDX(0, 1), "");
	zassert_false(DT_INST_REG_HAS_IDX(0, 2), "");

	/* DT_INST_REG_ADDR_BY_IDX */
	zassert_equal(DT_INST_REG_ADDR_BY_IDX(0, 0), 0x9999aaaa, "");
	zassert_equal(DT_INST_REG_ADDR_BY_IDX(0, 1), 0xbbbbcccc, "");

	/* DT_INST_REG_SIZE_BY_IDX */
	zassert_equal(DT_INST_REG_SIZE_BY_IDX(0, 0), 0x1000, "");
	zassert_equal(DT_INST_REG_SIZE_BY_IDX(0, 1), 0x3f, "");

	/* DT_INST_REG_ADDR */
	zassert_equal(DT_INST_REG_ADDR(0), 0x9999aaaa, "");

	/* DT_INST_REG_SIZE */
	zassert_equal(DT_INST_REG_SIZE(0), 0x1000, "");

	/* DT_INST_REG_ADDR_BY_NAME */
	zassert_equal(DT_INST_REG_ADDR_BY_NAME(0, first), 0x9999aaaa, "");
	zassert_equal(DT_INST_REG_ADDR_BY_NAME(0, second), 0xbbbbcccc, "");

	/* DT_INST_REG_SIZE_BY_NAME */
	zassert_equal(DT_INST_REG_SIZE_BY_NAME(0, first), 0x1000, "");
	zassert_equal(DT_INST_REG_SIZE_BY_NAME(0, second), 0x3f, "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_interrupt_holder
ZTEST(devicetree_api, test_irq)
{
	/* DT_NUM_IRQS */
	zassert_equal(DT_NUM_IRQS(TEST_DEADBEEF), 1, "");
	zassert_equal(DT_NUM_IRQS(TEST_I2C_BUS), 2, "");
	zassert_equal(DT_NUM_IRQS(TEST_SPI), 3, "");

	/* DT_IRQ_HAS_IDX */
	zassert_true(DT_IRQ_HAS_IDX(TEST_SPI_BUS_0, 0), "");
	zassert_true(DT_IRQ_HAS_IDX(TEST_SPI_BUS_0, 1), "");
	zassert_true(DT_IRQ_HAS_IDX(TEST_SPI_BUS_0, 2), "");
	zassert_false(DT_IRQ_HAS_IDX(TEST_SPI_BUS_0, 3), "");

	zassert_true(DT_IRQ_HAS_IDX(TEST_DEADBEEF, 0), "");
	zassert_false(DT_IRQ_HAS_IDX(TEST_DEADBEEF, 1), "");

	zassert_true(DT_IRQ_HAS_IDX(TEST_I2C_BUS, 0), "");
	zassert_true(DT_IRQ_HAS_IDX(TEST_I2C_BUS, 1), "");
	zassert_false(DT_IRQ_HAS_IDX(TEST_I2C_BUS, 2), "");

	/* DT_IRQ_BY_IDX */
	zassert_equal(DT_IRQ_BY_IDX(TEST_SPI_BUS_0, 0, irq), 8, "");
	zassert_equal(DT_IRQ_BY_IDX(TEST_SPI_BUS_0, 1, irq), 9, "");
	zassert_equal(DT_IRQ_BY_IDX(TEST_SPI_BUS_0, 2, irq), 10, "");
	zassert_equal(DT_IRQ_BY_IDX(TEST_SPI_BUS_0, 0, priority), 3, "");
	zassert_equal(DT_IRQ_BY_IDX(TEST_SPI_BUS_0, 1, priority), 0, "");
	zassert_equal(DT_IRQ_BY_IDX(TEST_SPI_BUS_0, 2, priority), 1, "");

	/* DT_IRQ_BY_NAME */
	zassert_equal(DT_IRQ_BY_NAME(TEST_I2C_BUS, status, irq), 6, "");
	zassert_equal(DT_IRQ_BY_NAME(TEST_I2C_BUS, error, irq), 7, "");
	zassert_equal(DT_IRQ_BY_NAME(TEST_I2C_BUS, status, priority), 2, "");
	zassert_equal(DT_IRQ_BY_NAME(TEST_I2C_BUS, error, priority), 1, "");

	/* DT_IRQ_HAS_CELL_AT_IDX */
	zassert_true(DT_IRQ_HAS_CELL_AT_IDX(TEST_IRQ, 0, irq), "");
	zassert_true(DT_IRQ_HAS_CELL_AT_IDX(TEST_IRQ, 0, priority), "");
	zassert_false(DT_IRQ_HAS_CELL_AT_IDX(TEST_IRQ, 0, foo), 0, "");
	zassert_true(DT_IRQ_HAS_CELL_AT_IDX(TEST_IRQ, 2, irq), "");
	zassert_true(DT_IRQ_HAS_CELL_AT_IDX(TEST_IRQ, 2, priority), "");
	zassert_false(DT_IRQ_HAS_CELL_AT_IDX(TEST_IRQ, 2, foo), "");

	/* DT_IRQ_HAS_CELL */
	zassert_true(DT_IRQ_HAS_CELL(TEST_IRQ, irq), "");
	zassert_true(DT_IRQ_HAS_CELL(TEST_IRQ, priority), "");
	zassert_false(DT_IRQ_HAS_CELL(TEST_IRQ, foo), "");

	/* DT_IRQ_HAS_NAME */
	zassert_true(DT_IRQ_HAS_NAME(TEST_IRQ, err), "");
	zassert_true(DT_IRQ_HAS_NAME(TEST_IRQ, stat), "");
	zassert_true(DT_IRQ_HAS_NAME(TEST_IRQ, done), "");
	zassert_false(DT_IRQ_HAS_NAME(TEST_IRQ, alpha), "");

	/* DT_IRQ */
	zassert_equal(DT_IRQ(TEST_I2C_BUS, irq), 6, "");
	zassert_equal(DT_IRQ(TEST_I2C_BUS, priority), 2, "");

	/* DT_IRQN */
	zassert_equal(DT_IRQN(TEST_I2C_BUS), 6, "");

	/* DT_INST */
	zassert_equal(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT), 1, "");

	/* DT_INST_IRQ_HAS_IDX */
	zassert_equal(DT_INST_IRQ_HAS_IDX(0, 0), 1, "");
	zassert_equal(DT_INST_IRQ_HAS_IDX(0, 1), 1, "");
	zassert_equal(DT_INST_IRQ_HAS_IDX(0, 2), 1, "");
	zassert_equal(DT_INST_IRQ_HAS_IDX(0, 3), 0, "");

	/* DT_INST_IRQ_BY_IDX */
	zassert_equal(DT_INST_IRQ_BY_IDX(0, 0, irq), 30, "");
	zassert_equal(DT_INST_IRQ_BY_IDX(0, 1, irq), 40, "");
	zassert_equal(DT_INST_IRQ_BY_IDX(0, 2, irq), 60, "");
	zassert_equal(DT_INST_IRQ_BY_IDX(0, 0, priority), 3, "");
	zassert_equal(DT_INST_IRQ_BY_IDX(0, 1, priority), 5, "");
	zassert_equal(DT_INST_IRQ_BY_IDX(0, 2, priority), 7, "");

	/* DT_INST_IRQ_BY_NAME */
	zassert_equal(DT_INST_IRQ_BY_NAME(0, err, irq), 30, "");
	zassert_equal(DT_INST_IRQ_BY_NAME(0, stat, irq), 40, "");
	zassert_equal(DT_INST_IRQ_BY_NAME(0, done, irq), 60, "");
	zassert_equal(DT_INST_IRQ_BY_NAME(0, err, priority), 3, "");
	zassert_equal(DT_INST_IRQ_BY_NAME(0, stat, priority), 5, "");
	zassert_equal(DT_INST_IRQ_BY_NAME(0, done, priority), 7, "");

	/* DT_INST_IRQ */
	zassert_equal(DT_INST_IRQ(0, irq), 30, "");
	zassert_equal(DT_INST_IRQ(0, priority), 3, "");

	/* DT_INST_IRQN */
	zassert_equal(DT_INST_IRQN(0), 30, "");

	/* DT_INST_IRQ_HAS_CELL_AT_IDX */
	zassert_true(DT_INST_IRQ_HAS_CELL_AT_IDX(0, 0, irq), "");
	zassert_true(DT_INST_IRQ_HAS_CELL_AT_IDX(0, 0, priority), "");
	zassert_false(DT_INST_IRQ_HAS_CELL_AT_IDX(0, 0, foo), "");
	zassert_true(DT_INST_IRQ_HAS_CELL_AT_IDX(0, 2, irq), "");
	zassert_true(DT_INST_IRQ_HAS_CELL_AT_IDX(0, 2, priority), "");
	zassert_false(DT_INST_IRQ_HAS_CELL_AT_IDX(0, 2, foo), "");

	/* DT_INST_IRQ_HAS_CELL */
	zassert_true(DT_INST_IRQ_HAS_CELL(0, irq), "");
	zassert_true(DT_INST_IRQ_HAS_CELL(0, priority), "");
	zassert_false(DT_INST_IRQ_HAS_CELL(0, foo), "");

	/* DT_INST_IRQ_HAS_NAME */
	zassert_true(DT_INST_IRQ_HAS_NAME(0, err), "");
	zassert_true(DT_INST_IRQ_HAS_NAME(0, stat), "");
	zassert_true(DT_INST_IRQ_HAS_NAME(0, done), "");
	zassert_false(DT_INST_IRQ_HAS_NAME(0, alpha), "");
}

struct gpios_struct {
	const char *name;
	gpio_pin_t pin;
	gpio_flags_t flags;
};

/* Helper macro that UTIL_LISTIFY can use and produces an element with comma */
#define DT_PROP_ELEM_BY_PHANDLE(idx, node_id, ph_prop, prop) \
	DT_PROP_BY_PHANDLE_IDX(node_id, ph_prop, idx, prop)
#define DT_PHANDLE_LISTIFY(node_id, ph_prop, prop) \
	{ \
	  LISTIFY(DT_PROP_LEN(node_id, ph_prop), \
		  DT_PROP_ELEM_BY_PHANDLE, (,), \
		  node_id, \
		  ph_prop, \
		  label) \
	}

/* Helper macro that UTIL_LISTIFY can use and produces an element with comma */
#define DT_GPIO_ELEM(idx, node_id, prop) \
	{ \
		DT_PROP(DT_PHANDLE_BY_IDX(node_id, prop, idx), label), \
		DT_PHA_BY_IDX(node_id, prop, idx, pin),\
		DT_PHA_BY_IDX(node_id, prop, idx, flags),\
	}
#define DT_GPIO_LISTIFY(node_id, prop) \
	{ LISTIFY(DT_PROP_LEN(node_id, prop), DT_GPIO_ELEM, (,), \
		  node_id, prop) }

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_phandle_holder
ZTEST(devicetree_api, test_phandles)
{
	const char *ph_label = DT_PROP_BY_PHANDLE(TEST_PH, ph, label);
	const char *phs_labels[] = DT_PHANDLE_LISTIFY(TEST_PH, phs, label);
	struct gpios_struct gps[] = DT_GPIO_LISTIFY(TEST_PH, gpios);

	/* phandle */
	zassert_true(DT_NODE_HAS_PROP(TEST_PH, ph), "");
	zassert_true(DT_SAME_NODE(DT_PROP(TEST_PH, ph),
				  DT_NODELABEL(test_gpio_1)), "");
	zassert_true(DT_SAME_NODE(DT_PROP_BY_IDX(TEST_PH, ph, 0),
				  DT_NODELABEL(test_gpio_1)), "");
	/* DT_PROP_BY_PHANDLE */
	zassert_true(!strcmp(ph_label, "TEST_GPIO_1"), "");

	/* phandles */
	zassert_true(DT_NODE_HAS_PROP(TEST_PH, phs), "");
	zassert_equal(ARRAY_SIZE(phs_labels), 3, "");
	zassert_equal(DT_PROP_LEN(TEST_PH, phs), 3, "");
	zassert_true(DT_SAME_NODE(DT_PROP_BY_IDX(TEST_PH, phs, 1),
				  DT_NODELABEL(test_gpio_2)), "");

	/* DT_PROP_BY_PHANDLE_IDX */
	zassert_true(!strcmp(DT_PROP_BY_PHANDLE_IDX(TEST_PH, phs, 0, label),
			     "TEST_GPIO_1"), "");
	zassert_true(!strcmp(DT_PROP_BY_PHANDLE_IDX(TEST_PH, phs, 1, label),
			     "TEST_GPIO_2"), "");
	zassert_true(!strcmp(DT_PROP_BY_PHANDLE_IDX(TEST_PH, phs, 2, label),
			     "TEST_I2C_CTLR"), "");
	zassert_true(!strcmp(phs_labels[0], "TEST_GPIO_1"), "");
	zassert_true(!strcmp(phs_labels[1], "TEST_GPIO_2"), "");
	zassert_true(!strcmp(phs_labels[2], "TEST_I2C_CTLR"), "");
	zassert_equal(DT_PROP_BY_PHANDLE_IDX(TEST_PH, gpios, 0,
					     gpio_controller), 1, "");
	zassert_equal(DT_PROP_BY_PHANDLE_IDX(TEST_PH, gpios, 1,
					     gpio_controller), 1, "");
	zassert_true(!strcmp(DT_PROP_BY_PHANDLE_IDX(TEST_PH, gpios, 0, label),
			     "TEST_GPIO_1"), "");
	zassert_true(!strcmp(DT_PROP_BY_PHANDLE_IDX(TEST_PH, gpios, 1, label),
			     "TEST_GPIO_2"), "");

	/* DT_PROP_BY_PHANDLE_IDX_OR */
	zassert_true(!strcmp(DT_PROP_BY_PHANDLE_IDX_OR(TEST_PH, phs_or, 0,
						val, "zero"), "one"), "");
	zassert_true(!strcmp(DT_PROP_BY_PHANDLE_IDX_OR(TEST_PH, phs_or, 1,
						val, "zero"), "zero"), "");

	/* phandle-array */
	zassert_true(DT_NODE_HAS_PROP(TEST_PH, gpios), "");
	zassert_equal(ARRAY_SIZE(gps), 2, "");
	zassert_equal(DT_PROP_LEN(TEST_PH, gpios), 2, "");

	/* DT_PROP_HAS_IDX */
	zassert_true(DT_PROP_HAS_IDX(TEST_PH, gpios, 0), "");
	zassert_true(DT_PROP_HAS_IDX(TEST_PH, gpios, 1), "");
	zassert_false(DT_PROP_HAS_IDX(TEST_PH, gpios, 2), "");

	/* DT_PROP_HAS_NAME */
	zassert_false(DT_PROP_HAS_NAME(TEST_PH, foos, A), "");
	zassert_true(DT_PROP_HAS_NAME(TEST_PH, foos, a), "");
	zassert_false(DT_PROP_HAS_NAME(TEST_PH, foos, b-c), "");
	zassert_true(DT_PROP_HAS_NAME(TEST_PH, foos, b_c), "");
	zassert_false(DT_PROP_HAS_NAME(TEST_PH, bazs, jane), "");

	/* DT_PHA_HAS_CELL_AT_IDX */
	zassert_true(DT_PHA_HAS_CELL_AT_IDX(TEST_PH, gpios, 1, pin), "");
	zassert_true(DT_PHA_HAS_CELL_AT_IDX(TEST_PH, gpios, 1, flags), "");
	/* pha-gpios index 1 has nothing, not even a phandle */
	zassert_false(DT_PROP_HAS_IDX(TEST_PH, pha_gpios, 1), "");
	zassert_false(DT_PHA_HAS_CELL_AT_IDX(TEST_PH, pha_gpios, 1, pin), "");
	zassert_false(DT_PHA_HAS_CELL_AT_IDX(TEST_PH, pha_gpios, 1, flags),
		      "");
	/* index 2 only has a pin cell, no flags */
	zassert_true(DT_PHA_HAS_CELL_AT_IDX(TEST_PH, pha_gpios, 2, pin), "");
	zassert_false(DT_PHA_HAS_CELL_AT_IDX(TEST_PH, pha_gpios, 2, flags),
		      "");
	/* index 3 has both pin and flags cells*/
	zassert_true(DT_PHA_HAS_CELL_AT_IDX(TEST_PH, pha_gpios, 3, pin), "");
	zassert_true(DT_PHA_HAS_CELL_AT_IDX(TEST_PH, pha_gpios, 3, flags), "");
	/* even though index 1 has nothing, the length is still 4 */
	zassert_equal(DT_PROP_LEN(TEST_PH, pha_gpios), 4, "");

	/* DT_PHA_HAS_CELL */
	zassert_true(DT_PHA_HAS_CELL(TEST_PH, gpios, flags), "");
	zassert_false(DT_PHA_HAS_CELL(TEST_PH, gpios, bar), "");

	/* DT_PHANDLE_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_PHANDLE_BY_IDX(TEST_PH, gpios, 0), TEST_GPIO_1), "");
	zassert_true(DT_SAME_NODE(DT_PHANDLE_BY_IDX(TEST_PH, gpios, 1), TEST_GPIO_2), "");

	/* DT_PHANDLE */
	zassert_true(DT_SAME_NODE(DT_PHANDLE(TEST_PH, gpios), TEST_GPIO_1), "");

	/* DT_PHA */
	zassert_equal(DT_PHA(TEST_PH, gpios, pin), 10, "");
	zassert_equal(DT_PHA(TEST_PH, gpios, flags), 20, "");

	/* DT_PHA_BY_IDX */
	zassert_equal(DT_PHA_BY_IDX(TEST_PH, gpios, 0, pin), 10, "");
	zassert_equal(DT_PHA_BY_IDX(TEST_PH, gpios, 0, flags), 20, "");

	zassert_equal(DT_PHA_BY_IDX(TEST_PH, gpios, 1, pin), 30, "");
	zassert_equal(DT_PHA_BY_IDX(TEST_PH, gpios, 1, flags), 40, "");

	/* DT_PHA_BY_NAME */
	zassert_equal(DT_PHA_BY_NAME(TEST_PH, foos, a, foocell), 100, "");
	zassert_equal(DT_PHA_BY_NAME(TEST_PH, foos, b_c, foocell), 110, "");

	/* DT_PHANDLE_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_PHANDLE_BY_NAME(TEST_PH, foos, a), TEST_GPIO_1), "");
	zassert_true(DT_SAME_NODE(DT_PHANDLE_BY_NAME(TEST_PH, foos, b_c), TEST_GPIO_2), "");

	/* array initializers */
	zassert_true(!strcmp(gps[0].name, "TEST_GPIO_1"), "");
	zassert_equal(gps[0].pin, 10, "");
	zassert_equal(gps[0].flags, 20, "");

	zassert_true(!strcmp(gps[1].name, "TEST_GPIO_2"), "");
	zassert_equal(gps[1].pin, 30, "");
	zassert_equal(gps[1].flags, 40, "");

	/* DT_INST */
	zassert_equal(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT), 1, "");

	/* DT_INST_PROP_BY_PHANDLE */
	zassert_true(!strcmp(DT_INST_PROP_BY_PHANDLE(0, ph, label),
			     "TEST_GPIO_1"), "");

	zassert_true(!strcmp(ph_label, "TEST_GPIO_1"), "");

	/* DT_INST_PROP_BY_PHANDLE_IDX */
	zassert_true(!strcmp(DT_INST_PROP_BY_PHANDLE_IDX(0, phs, 0, label),
			     "TEST_GPIO_1"), "");
	zassert_true(!strcmp(DT_INST_PROP_BY_PHANDLE_IDX(0, phs, 1, label),
			     "TEST_GPIO_2"), "");
	zassert_true(!strcmp(DT_INST_PROP_BY_PHANDLE_IDX(0, phs, 2, label),
			     "TEST_I2C_CTLR"), "");
	zassert_true(!strcmp(phs_labels[0], "TEST_GPIO_1"), "");
	zassert_true(!strcmp(phs_labels[1], "TEST_GPIO_2"), "");
	zassert_true(!strcmp(phs_labels[2], "TEST_I2C_CTLR"), "");
	zassert_equal(DT_INST_PROP_BY_PHANDLE_IDX(0, gpios, 0,
					     gpio_controller),
		      1, "");
	zassert_equal(DT_INST_PROP_BY_PHANDLE_IDX(0, gpios, 1,
					     gpio_controller),
		      1, "");
	zassert_true(!strcmp(DT_INST_PROP_BY_PHANDLE_IDX(0, gpios, 0, label),
			     "TEST_GPIO_1"), "");
	zassert_true(!strcmp(DT_INST_PROP_BY_PHANDLE_IDX(0, gpios, 1, label),
			     "TEST_GPIO_2"), "");

	/* DT_INST_PROP_HAS_IDX */
	zassert_true(DT_INST_PROP_HAS_IDX(0, gpios, 0), "");
	zassert_true(DT_INST_PROP_HAS_IDX(0, gpios, 1), "");
	zassert_false(DT_INST_PROP_HAS_IDX(0, gpios, 2), "");

	/* DT_INST_PROP_HAS_NAME */
	zassert_false(DT_INST_PROP_HAS_NAME(0, foos, A), "");
	zassert_true(DT_INST_PROP_HAS_NAME(0, foos, a), "");
	zassert_false(DT_INST_PROP_HAS_NAME(0, foos, b-c), "");
	zassert_true(DT_INST_PROP_HAS_NAME(0, foos, b_c), "");
	zassert_false(DT_INST_PROP_HAS_NAME(0, bazs, jane), "");

	/* DT_INST_PHA_HAS_CELL_AT_IDX */
	zassert_true(DT_INST_PHA_HAS_CELL_AT_IDX(0, gpios, 1, pin), "");
	zassert_true(DT_INST_PHA_HAS_CELL_AT_IDX(0, gpios, 1, flags), "");
	/* index 1 has nothing, not even a phandle */
	zassert_false(DT_INST_PROP_HAS_IDX(0, pha_gpios, 1), "");
	zassert_false(DT_INST_PHA_HAS_CELL_AT_IDX(0, pha_gpios, 1, pin), "");
	zassert_false(DT_INST_PHA_HAS_CELL_AT_IDX(0, pha_gpios, 1, flags), "");
	/* index 2 only has pin, no flags */
	zassert_true(DT_INST_PHA_HAS_CELL_AT_IDX(0, pha_gpios, 2, pin), "");
	zassert_false(DT_INST_PHA_HAS_CELL_AT_IDX(0, pha_gpios, 2, flags), "");
	/* index 3 has both pin and flags */
	zassert_true(DT_INST_PHA_HAS_CELL_AT_IDX(0, pha_gpios, 3, pin), "");
	zassert_true(DT_INST_PHA_HAS_CELL_AT_IDX(0, pha_gpios, 3, flags), "");
	/* even though index 1 has nothing, the length is still 4 */
	zassert_equal(DT_INST_PROP_LEN(0, pha_gpios), 4, "");

	/* DT_INST_PHA_HAS_CELL */
	zassert_true(DT_INST_PHA_HAS_CELL(0, gpios, flags), "");
	zassert_false(DT_INST_PHA_HAS_CELL(0, gpios, bar), "");

	/* DT_INST_PHANDLE_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_INST_PHANDLE_BY_IDX(0, gpios, 0), TEST_GPIO_1), "");
	zassert_true(DT_SAME_NODE(DT_INST_PHANDLE_BY_IDX(0, gpios, 1), TEST_GPIO_2), "");

	/* DT_INST_PHANDLE */
	zassert_true(DT_SAME_NODE(DT_INST_PHANDLE(0, gpios), TEST_GPIO_1), "");

	/* DT_INST_PHA */
	zassert_equal(DT_INST_PHA(0, gpios, pin), 10, "");
	zassert_equal(DT_INST_PHA(0, gpios, flags), 20, "");

	/* DT_INST_PHA_BY_IDX */
	zassert_equal(DT_INST_PHA_BY_IDX(0, gpios, 0, pin), 10, "");
	zassert_equal(DT_INST_PHA_BY_IDX(0, gpios, 0, flags), 20, "");

	zassert_equal(DT_INST_PHA_BY_IDX(0, gpios, 1, pin), 30, "");
	zassert_equal(DT_INST_PHA_BY_IDX(0, gpios, 1, flags), 40, "");

	/* DT_INST_PHA_BY_NAME */
	zassert_equal(DT_INST_PHA_BY_NAME(0, foos, a, foocell), 100, "");
	zassert_equal(DT_INST_PHA_BY_NAME(0, foos, b_c, foocell), 110, "");

	/* DT_INST_PHANDLE_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_INST_PHANDLE_BY_NAME(0, foos, a), TEST_GPIO_1), "");
	zassert_true(DT_SAME_NODE(DT_INST_PHANDLE_BY_NAME(0, foos, b_c), TEST_GPIO_2), "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_phandle_holder
ZTEST(devicetree_api, test_gpio)
{
	/* DT_GPIO_CTLR_BY_IDX */
	zassert_true(!strcmp(TO_STRING(DT_GPIO_CTLR_BY_IDX(TEST_PH, gpios, 0)),
			     TO_STRING(DT_NODELABEL(test_gpio_1))), "");
	zassert_true(!strcmp(TO_STRING(DT_GPIO_CTLR_BY_IDX(TEST_PH, gpios, 1)),
			     TO_STRING(DT_NODELABEL(test_gpio_2))), "");

	/* DT_GPIO_CTLR */
	zassert_true(!strcmp(TO_STRING(DT_GPIO_CTLR(TEST_PH, gpios)),
			     TO_STRING(DT_NODELABEL(test_gpio_1))), "");

	/* DT_GPIO_LABEL_BY_IDX */
	zassert_true(!strcmp(DT_GPIO_LABEL_BY_IDX(TEST_PH, gpios, 0),
			     "TEST_GPIO_1"), "");
	zassert_true(!strcmp(DT_GPIO_LABEL_BY_IDX(TEST_PH, gpios, 1),
			     "TEST_GPIO_2"), "");

	/* DT_GPIO_LABEL */
	zassert_true(!strcmp(DT_GPIO_LABEL(TEST_PH, gpios), "TEST_GPIO_1"), "");

	/* DT_GPIO_PIN_BY_IDX */
	zassert_equal(DT_GPIO_PIN_BY_IDX(TEST_PH, gpios, 0), 10, "");
	zassert_equal(DT_GPIO_PIN_BY_IDX(TEST_PH, gpios, 1), 30, "");

	/* DT_GPIO_PIN */
	zassert_equal(DT_GPIO_PIN(TEST_PH, gpios), 10, "");

	/* DT_GPIO_FLAGS_BY_IDX */
	zassert_equal(DT_GPIO_FLAGS_BY_IDX(TEST_PH, gpios, 0), 20, "");
	zassert_equal(DT_GPIO_FLAGS_BY_IDX(TEST_PH, gpios, 1), 40, "");

	/* DT_GPIO_FLAGS */
	zassert_equal(DT_GPIO_FLAGS(TEST_PH, gpios), 20, "");

	/* DT_INST */
	zassert_equal(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT), 1, "");

	/* DT_INST_GPIO_LABEL_BY_IDX */
	zassert_true(!strcmp(DT_INST_GPIO_LABEL_BY_IDX(0, gpios, 0),
			     "TEST_GPIO_1"), "");
	zassert_true(!strcmp(DT_INST_GPIO_LABEL_BY_IDX(0, gpios, 1),
			     "TEST_GPIO_2"), "");

	/* DT_INST_GPIO_LABEL */
	zassert_true(!strcmp(DT_INST_GPIO_LABEL(0, gpios), "TEST_GPIO_1"), "");

	/* DT_INST_GPIO_PIN_BY_IDX */
	zassert_equal(DT_INST_GPIO_PIN_BY_IDX(0, gpios, 0), 10, "");
	zassert_equal(DT_INST_GPIO_PIN_BY_IDX(0, gpios, 1), 30, "");

	/* DT_INST_GPIO_PIN */
	zassert_equal(DT_INST_GPIO_PIN(0, gpios), 10, "");

	/* DT_INST_GPIO_FLAGS_BY_IDX */
	zassert_equal(DT_INST_GPIO_FLAGS_BY_IDX(0, gpios, 0), 20, "");
	zassert_equal(DT_INST_GPIO_FLAGS_BY_IDX(0, gpios, 1), 40, "");

	/* DT_INST_GPIO_FLAGS */
	zassert_equal(DT_INST_GPIO_FLAGS(0, gpios), 20, "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_adc_temp_sensor
ZTEST(devicetree_api, test_io_channels)
{
	/* DT_IO_CHANNELS_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_IO_CHANNELS_CTLR_BY_IDX(TEST_TEMP, 0),
				  TEST_IO_CHANNEL_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_IO_CHANNELS_CTLR_BY_IDX(TEST_TEMP, 1),
				  TEST_IO_CHANNEL_CTLR_2), "");

	/* DT_IO_CHANNELS_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_IO_CHANNELS_CTLR_BY_NAME(TEST_TEMP, ch1),
				  TEST_IO_CHANNEL_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_IO_CHANNELS_CTLR_BY_NAME(TEST_TEMP, ch2),
				  TEST_IO_CHANNEL_CTLR_2), "");

	/* DT_IO_CHANNELS_CTLR */
	zassert_true(DT_SAME_NODE(DT_IO_CHANNELS_CTLR(TEST_TEMP),
				  TEST_IO_CHANNEL_CTLR_1), "");

	/* DT_INST_IO_CHANNELS_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_INST_IO_CHANNELS_CTLR_BY_IDX(0, 0),
				  TEST_IO_CHANNEL_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_INST_IO_CHANNELS_CTLR_BY_IDX(0, 1),
				  TEST_IO_CHANNEL_CTLR_2), "");

	/* DT_INST_IO_CHANNELS_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_INST_IO_CHANNELS_CTLR_BY_NAME(0, ch1),
				  TEST_IO_CHANNEL_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_INST_IO_CHANNELS_CTLR_BY_NAME(0, ch2),
				  TEST_IO_CHANNEL_CTLR_2), "");

	/* DT_INST_IO_CHANNELS_CTLR */
	zassert_true(DT_SAME_NODE(DT_INST_IO_CHANNELS_CTLR(0),
				  TEST_IO_CHANNEL_CTLR_1), "");

	zassert_equal(DT_IO_CHANNELS_INPUT_BY_IDX(TEST_TEMP, 0), 10, "");
	zassert_equal(DT_IO_CHANNELS_INPUT_BY_IDX(TEST_TEMP, 1), 20, "");
	zassert_equal(DT_IO_CHANNELS_INPUT_BY_NAME(TEST_TEMP, ch1), 10, "");
	zassert_equal(DT_IO_CHANNELS_INPUT_BY_NAME(TEST_TEMP, ch2), 20, "");
	zassert_equal(DT_IO_CHANNELS_INPUT(TEST_TEMP), 10, "");

	zassert_equal(DT_INST_IO_CHANNELS_INPUT_BY_IDX(0, 0), 10, "");
	zassert_equal(DT_INST_IO_CHANNELS_INPUT_BY_IDX(0, 1), 20, "");
	zassert_equal(DT_INST_IO_CHANNELS_INPUT_BY_NAME(0, ch1), 10, "");
	zassert_equal(DT_INST_IO_CHANNELS_INPUT_BY_NAME(0, ch2), 20, "");
	zassert_equal(DT_INST_IO_CHANNELS_INPUT(0), 10, "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_adc_temp_sensor
ZTEST(devicetree_api, test_dma)
{
	/* DT_DMAS_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_DMAS_CTLR_BY_IDX(TEST_TEMP, 0),
				  TEST_DMA_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_DMAS_CTLR_BY_IDX(TEST_TEMP, 1),
				  TEST_DMA_CTLR_2), "");

	/* DT_DMAS_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_DMAS_CTLR_BY_NAME(TEST_TEMP, tx),
				  TEST_DMA_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_DMAS_CTLR_BY_NAME(TEST_TEMP, rx),
				  TEST_DMA_CTLR_2), "");

	/* DT_DMAS_CTLR */
	zassert_true(DT_SAME_NODE(DT_DMAS_CTLR(TEST_TEMP),
				  TEST_DMA_CTLR_1), "");

	/* DT_INST_DMAS_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_INST_DMAS_CTLR_BY_IDX(0, 0),
				  TEST_DMA_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_INST_DMAS_CTLR_BY_IDX(0, 1),
				  TEST_DMA_CTLR_2), "");

	/* DT_INST_DMAS_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_INST_DMAS_CTLR_BY_NAME(0, tx),
				  TEST_DMA_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_INST_DMAS_CTLR_BY_NAME(0, rx),
				  TEST_DMA_CTLR_2), "");

	/* DT_INST_DMAS_CTLR */
	zassert_true(DT_SAME_NODE(DT_INST_DMAS_CTLR(0), TEST_DMA_CTLR_1), "");

	zassert_equal(DT_DMAS_CELL_BY_NAME(TEST_TEMP, rx, channel), 3, "");
	zassert_equal(DT_INST_DMAS_CELL_BY_NAME(0, rx, channel), 3, "");
	zassert_equal(DT_DMAS_CELL_BY_NAME(TEST_TEMP, rx, slot), 4, "");
	zassert_equal(DT_INST_DMAS_CELL_BY_NAME(0, rx, slot), 4, "");

	zassert_equal(DT_DMAS_CELL_BY_IDX(TEST_TEMP, 1, channel), 3, "");
	zassert_equal(DT_INST_DMAS_CELL_BY_IDX(0, 1, channel), 3, "");
	zassert_equal(DT_DMAS_CELL_BY_IDX(TEST_TEMP, 1, slot), 4, "");
	zassert_equal(DT_INST_DMAS_CELL_BY_IDX(0, 1, slot), 4, "");

	zassert_true(DT_DMAS_HAS_NAME(TEST_TEMP, tx), "");
	zassert_true(DT_INST_DMAS_HAS_NAME(0, tx), "");
	zassert_false(DT_DMAS_HAS_NAME(TEST_TEMP, output), "");
	zassert_false(DT_INST_DMAS_HAS_NAME(0, output), "");

	zassert_true(DT_DMAS_HAS_IDX(TEST_TEMP, 1), "");
	zassert_true(DT_INST_DMAS_HAS_IDX(0, 1), "");
	zassert_false(DT_DMAS_HAS_IDX(TEST_TEMP, 2), "");
	zassert_false(DT_INST_DMAS_HAS_IDX(0, 2), "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_phandle_holder
ZTEST(devicetree_api, test_pwms)
{
	/* DT_PWMS_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_PWMS_CTLR_BY_IDX(TEST_PH, 0),
				  TEST_PWM_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_PWMS_CTLR_BY_IDX(TEST_PH, 1),
				  TEST_PWM_CTLR_2), "");

	/* DT_PWMS_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_PWMS_CTLR_BY_NAME(TEST_PH, red),
				  TEST_PWM_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_PWMS_CTLR_BY_NAME(TEST_PH, green),
				  TEST_PWM_CTLR_2), "");

	/* DT_PWMS_CTLR */
	zassert_true(DT_SAME_NODE(DT_PWMS_CTLR(TEST_PH),
				  TEST_PWM_CTLR_1), "");

	/* DT_PWMS_CELL_BY_IDX */
	zassert_equal(DT_PWMS_CELL_BY_IDX(TEST_PH, 1, channel), 5, "");
	zassert_equal(DT_PWMS_CELL_BY_IDX(TEST_PH, 1, period), 100, "");
	zassert_equal(DT_PWMS_CELL_BY_IDX(TEST_PH, 1, flags), 1, "");

	/* DT_PWMS_CELL_BY_NAME */
	zassert_equal(DT_PWMS_CELL_BY_NAME(TEST_PH, red, channel), 8, "");
	zassert_equal(DT_PWMS_CELL_BY_NAME(TEST_PH, red, period), 200, "");
	zassert_equal(DT_PWMS_CELL_BY_NAME(TEST_PH, red, flags), 3, "");

	/* DT_PWMS_CELL */
	zassert_equal(DT_PWMS_CELL(TEST_PH, channel), 8, "");
	zassert_equal(DT_PWMS_CELL(TEST_PH, period), 200, "");
	zassert_equal(DT_PWMS_CELL(TEST_PH, flags), 3, "");

	/* DT_PWMS_CHANNEL_BY_IDX */
	zassert_equal(DT_PWMS_CHANNEL_BY_IDX(TEST_PH, 1), 5, "");

	/* DT_PWMS_CHANNEL_BY_NAME */
	zassert_equal(DT_PWMS_CHANNEL_BY_NAME(TEST_PH, green), 5, "");

	/* DT_PWMS_CHANNEL */
	zassert_equal(DT_PWMS_CHANNEL(TEST_PH), 8, "");

	/* DT_PWMS_PERIOD_BY_IDX */
	zassert_equal(DT_PWMS_PERIOD_BY_IDX(TEST_PH, 1), 100, "");

	/* DT_PWMS_PERIOD_BY_NAME */
	zassert_equal(DT_PWMS_PERIOD_BY_NAME(TEST_PH, green), 100, "");

	/* DT_PWMS_PERIOD */
	zassert_equal(DT_PWMS_PERIOD(TEST_PH), 200, "");

	/* DT_PWMS_FLAGS_BY_IDX */
	zassert_equal(DT_PWMS_FLAGS_BY_IDX(TEST_PH, 1), 1, "");

	/* DT_PWMS_FLAGS_BY_NAME */
	zassert_equal(DT_PWMS_FLAGS_BY_NAME(TEST_PH, green), 1, "");

	/* DT_PWMS_FLAGS */
	zassert_equal(DT_PWMS_FLAGS(TEST_PH), 3, "");

	/* DT_INST */
	zassert_equal(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT), 1, "");

	/* DT_INST_PWMS_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_INST_PWMS_CTLR_BY_IDX(0, 0),
				  TEST_PWM_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_INST_PWMS_CTLR_BY_IDX(0, 1),
				  TEST_PWM_CTLR_2), "");

	/* DT_INST_PWMS_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_INST_PWMS_CTLR_BY_NAME(0, red),
				  TEST_PWM_CTLR_1), "");
	zassert_true(DT_SAME_NODE(DT_INST_PWMS_CTLR_BY_NAME(0, green),
				  TEST_PWM_CTLR_2), "");

	/* DT_INST_PWMS_CTLR */
	zassert_true(DT_SAME_NODE(DT_INST_PWMS_CTLR(0), TEST_PWM_CTLR_1), "");

	/* DT_INST_PWMS_CELL_BY_IDX */
	zassert_equal(DT_INST_PWMS_CELL_BY_IDX(0, 1, channel), 5, "");
	zassert_equal(DT_INST_PWMS_CELL_BY_IDX(0, 1, period), 100, "");
	zassert_equal(DT_INST_PWMS_CELL_BY_IDX(0, 1, flags), 1, "");

	/* DT_INST_PWMS_CELL_BY_NAME */
	zassert_equal(DT_INST_PWMS_CELL_BY_NAME(0, green, channel), 5, "");
	zassert_equal(DT_INST_PWMS_CELL_BY_NAME(0, green, period), 100, "");
	zassert_equal(DT_INST_PWMS_CELL_BY_NAME(0, green, flags), 1, "");

	/* DT_INST_PWMS_CELL */
	zassert_equal(DT_INST_PWMS_CELL(0, channel), 8, "");
	zassert_equal(DT_INST_PWMS_CELL(0, period), 200, "");
	zassert_equal(DT_INST_PWMS_CELL(0, flags), 3, "");

	/* DT_INST_PWMS_CHANNEL_BY_IDX */
	zassert_equal(DT_INST_PWMS_CHANNEL_BY_IDX(0, 1), 5, "");

	/* DT_INST_PWMS_CHANNEL_BY_NAME */
	zassert_equal(DT_INST_PWMS_CHANNEL_BY_NAME(0, green), 5, "");

	/* DT_INST_PWMS_CHANNEL */
	zassert_equal(DT_INST_PWMS_CHANNEL(0), 8, "");

	/* DT_INST_PWMS_PERIOD_BY_IDX */
	zassert_equal(DT_INST_PWMS_PERIOD_BY_IDX(0, 1), 100, "");

	/* DT_INST_PWMS_PERIOD_BY_NAME */
	zassert_equal(DT_INST_PWMS_PERIOD_BY_NAME(0, red), 200, "");

	/* DT_INST_PWMS_PERIOD */
	zassert_equal(DT_INST_PWMS_PERIOD(0), 200, "");

	/* DT_INST_PWMS_FLAGS_BY_IDX */
	zassert_equal(DT_INST_PWMS_FLAGS_BY_IDX(0, 1), 1, "");

	/* DT_INST_PWMS_FLAGS_BY_NAME */
	zassert_equal(DT_INST_PWMS_FLAGS_BY_NAME(0, red), 3, "");

	/* DT_INST_PWMS_FLAGS */
	zassert_equal(DT_INST_PWMS_FLAGS(0), 3, "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_can_controller
ZTEST(devicetree_api, test_can)
{
	/* DT_CAN_TRANSCEIVER_MAX_BITRATE */
	zassert_equal(DT_CAN_TRANSCEIVER_MAX_BITRATE(TEST_CAN_CTRL_0, 1000000), 1000000, "");
	zassert_equal(DT_CAN_TRANSCEIVER_MAX_BITRATE(TEST_CAN_CTRL_0, 5000000), 5000000, "");
	zassert_equal(DT_CAN_TRANSCEIVER_MAX_BITRATE(TEST_CAN_CTRL_0, 8000000), 5000000, "");
	zassert_equal(DT_CAN_TRANSCEIVER_MAX_BITRATE(TEST_CAN_CTRL_1, 1250000), 1250000, "");
	zassert_equal(DT_CAN_TRANSCEIVER_MAX_BITRATE(TEST_CAN_CTRL_1, 2000000), 2000000, "");
	zassert_equal(DT_CAN_TRANSCEIVER_MAX_BITRATE(TEST_CAN_CTRL_1, 5000000), 2000000, "");

	/* DT_INST_CAN_TRANSCEIVER_MAX_BITRATE */
	zassert_equal(DT_INST_CAN_TRANSCEIVER_MAX_BITRATE(0, 1000000), 1000000, "");
	zassert_equal(DT_INST_CAN_TRANSCEIVER_MAX_BITRATE(0, 5000000), 5000000, "");
	zassert_equal(DT_INST_CAN_TRANSCEIVER_MAX_BITRATE(0, 8000000), 5000000, "");
	zassert_equal(DT_INST_CAN_TRANSCEIVER_MAX_BITRATE(1, 1250000), 1250000, "");
	zassert_equal(DT_INST_CAN_TRANSCEIVER_MAX_BITRATE(1, 2000000), 2000000, "");
	zassert_equal(DT_INST_CAN_TRANSCEIVER_MAX_BITRATE(1, 5000000), 2000000, "");
}

ZTEST(devicetree_api, test_macro_names)
{
	/* white box */
	zassert_true(!strcmp(TO_STRING(DT_PATH(test, gpio_deadbeef)),
			     "DT_N_S_test_S_gpio_deadbeef"), "");
	zassert_true(!strcmp(TO_STRING(DT_ALIAS(test_alias)),
			     "DT_N_S_test_S_gpio_deadbeef"), "");
	zassert_true(!strcmp(TO_STRING(DT_NODELABEL(test_nodelabel)),
			     "DT_N_S_test_S_gpio_deadbeef"), "");
	zassert_true(!strcmp(TO_STRING(DT_NODELABEL(test_nodelabel_allcaps)),
			     "DT_N_S_test_S_gpio_deadbeef"), "");

#define CHILD_NODE_ID DT_CHILD(DT_PATH(test, i2c_11112222), test_i2c_dev_10)
#define FULL_PATH_ID DT_PATH(test, i2c_11112222, test_i2c_dev_10)

	zassert_true(!strcmp(TO_STRING(CHILD_NODE_ID),
			     TO_STRING(FULL_PATH_ID)), "");

#undef CHILD_NODE_ID
#undef FULL_PATH_ID
}

static int a[] = DT_PROP(TEST_ARRAYS, a);
static unsigned char b[] = DT_PROP(TEST_ARRAYS, b);
static char *c[] = DT_PROP(TEST_ARRAYS, c);

ZTEST(devicetree_api, test_arrays)
{
	int ok;

	zassert_equal(ARRAY_SIZE(a), 3, "");
	zassert_equal(ARRAY_SIZE(b), 4, "");
	zassert_equal(ARRAY_SIZE(c), 2, "");

	zassert_equal(a[0], 1000, "");
	zassert_equal(a[1], 2000, "");
	zassert_equal(a[2], 3000, "");

	zassert_true(DT_PROP_HAS_IDX(TEST_ARRAYS, a, 0), "");
	zassert_true(DT_PROP_HAS_IDX(TEST_ARRAYS, a, 1), "");
	zassert_true(DT_PROP_HAS_IDX(TEST_ARRAYS, a, 2), "");
	zassert_false(DT_PROP_HAS_IDX(TEST_ARRAYS, a, 3), "");

	/*
	 * Verify that DT_PROP_HAS_IDX can be used with COND_CODE_1()
	 * and COND_CODE_0(), i.e. its expansion is a literal 1 or 0,
	 * not an equivalent expression that evaluates to 1 or 0.
	 */
	ok = 0;
	COND_CODE_1(DT_PROP_HAS_IDX(TEST_ARRAYS, a, 0), (ok = 1;), ());
	zassert_equal(ok, 1, "");
	ok = 0;
	COND_CODE_0(DT_PROP_HAS_IDX(TEST_ARRAYS, a, 3), (ok = 1;), ());
	zassert_equal(ok, 1, "");

	zassert_equal(DT_PROP_BY_IDX(TEST_ARRAYS, a, 0), a[0], "");
	zassert_equal(DT_PROP_BY_IDX(TEST_ARRAYS, a, 1), a[1], "");
	zassert_equal(DT_PROP_BY_IDX(TEST_ARRAYS, a, 2), a[2], "");

	zassert_equal(DT_PROP_LEN(TEST_ARRAYS, a), 3, "");

	zassert_equal(b[0], 0xaa, "");
	zassert_equal(b[1], 0xbb, "");
	zassert_equal(b[2], 0xcc, "");
	zassert_equal(b[3], 0xdd, "");

	zassert_true(DT_PROP_HAS_IDX(TEST_ARRAYS, b, 0), "");
	zassert_true(DT_PROP_HAS_IDX(TEST_ARRAYS, b, 1), "");
	zassert_true(DT_PROP_HAS_IDX(TEST_ARRAYS, b, 2), "");
	zassert_true(DT_PROP_HAS_IDX(TEST_ARRAYS, b, 3), "");
	zassert_false(DT_PROP_HAS_IDX(TEST_ARRAYS, b, 4), "");

	zassert_equal(DT_PROP_BY_IDX(TEST_ARRAYS, b, 0), b[0], "");
	zassert_equal(DT_PROP_BY_IDX(TEST_ARRAYS, b, 1), b[1], "");
	zassert_equal(DT_PROP_BY_IDX(TEST_ARRAYS, b, 2), b[2], "");
	zassert_equal(DT_PROP_BY_IDX(TEST_ARRAYS, b, 3), b[3], "");

	zassert_equal(DT_PROP_LEN(TEST_ARRAYS, b), 4, "");

	zassert_true(!strcmp(c[0], "bar"), "");
	zassert_true(!strcmp(c[1], "baz"), "");

	zassert_true(DT_PROP_HAS_IDX(TEST_ARRAYS, c, 0), "");
	zassert_true(DT_PROP_HAS_IDX(TEST_ARRAYS, c, 1), "");
	zassert_false(DT_PROP_HAS_IDX(TEST_ARRAYS, c, 2), "");

	zassert_true(!strcmp(DT_PROP_BY_IDX(TEST_ARRAYS, c, 0), c[0]), "");
	zassert_true(!strcmp(DT_PROP_BY_IDX(TEST_ARRAYS, c, 1), c[1]), "");

	zassert_equal(DT_PROP_LEN(TEST_ARRAYS, c), 2, "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_gpio_device
ZTEST(devicetree_api, test_foreach_status_okay)
{
	/*
	 * For-each-node type macro tests.
	 *
	 * See test_foreach_prop_elem*() for tests of
	 * for-each-property type macros.
	 */
	unsigned int val;
	const char *str;

	/* This should expand to something like:
	 *
	 * "/test/enum-0" "/test/enum-1"
	 *
	 * but there is no guarantee about the order of nodes in the
	 * expansion, so we test both.
	 */
	str = DT_FOREACH_STATUS_OKAY(vnd_enum_holder, DT_NODE_PATH);
	zassert_true(!strcmp(str, "/test/enum-0/test/enum-1") ||
		     !strcmp(str, "/test/enum-1/test/enum-0"), "");

#undef MY_FN
#define MY_FN(node_id, operator) DT_ENUM_IDX(node_id, val) operator
	/* This should expand to something like:
	 *
	 * 0 + 2 + 3
	 *
	 * and order of expansion doesn't matter, since we're adding
	 * the values all up.
	 */
	val = DT_FOREACH_STATUS_OKAY_VARGS(vnd_enum_holder, MY_FN, +) 3;
	zassert_equal(val, 5, "");

	/*
	 * Make sure DT_INST_FOREACH_STATUS_OKAY can be called from functions
	 * using macros with side effects in the current scope.
	 */
	val = 0;
#define INC(inst_ignored) do { val++; } while (0);
	DT_INST_FOREACH_STATUS_OKAY(INC)
	zassert_equal(val, 2, "");
#undef INC

	val = 0;
#define INC_ARG(arg) do { val++; val += arg; } while (0)
#define INC(inst_ignored, arg) INC_ARG(arg);
	DT_INST_FOREACH_STATUS_OKAY_VARGS(INC, 1)
	zassert_equal(val, 4, "");
#undef INC_ARG
#undef INC

	/*
	 * Make sure DT_INST_FOREACH_STATUS_OKAY works with 0 instances, and does
	 * not expand its argument at all.
	 */
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT xxxx
#define BUILD_BUG_ON_EXPANSION (there is a bug in devicetree.h)
	DT_INST_FOREACH_STATUS_OKAY(BUILD_BUG_ON_EXPANSION)
#undef BUILD_BUG_ON_EXPANSION

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT xxxx
#define BUILD_BUG_ON_EXPANSION(arg) (there is a bug in devicetree.h)
	DT_INST_FOREACH_STATUS_OKAY_VARGS(BUILD_BUG_ON_EXPANSION, 1)
#undef BUILD_BUG_ON_EXPANSION
}

ZTEST(devicetree_api, test_foreach_prop_elem)
{
#define TIMES_TWO(node_id, prop, idx) \
	(2 * DT_PROP_BY_IDX(node_id, prop, idx)),

	int array[] = {
		DT_FOREACH_PROP_ELEM(TEST_ARRAYS, a, TIMES_TWO)
	};

	zassert_equal(ARRAY_SIZE(array), 3, "");
	zassert_equal(array[0], 2000, "");
	zassert_equal(array[1], 4000, "");
	zassert_equal(array[2], 6000, "");

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_array_holder

	int inst_array[] = {
		DT_INST_FOREACH_PROP_ELEM(0, a, TIMES_TWO)
	};

	zassert_equal(ARRAY_SIZE(inst_array), ARRAY_SIZE(array), "");
	zassert_equal(inst_array[0], array[0], "");
	zassert_equal(inst_array[1], array[1], "");
	zassert_equal(inst_array[2], array[2], "");
#undef TIMES_TWO
}

ZTEST(devicetree_api, test_foreach_prop_elem_varg)
{
#define TIMES_TWO_ADD(node_id, prop, idx, arg) \
	((2 * DT_PROP_BY_IDX(node_id, prop, idx)) + arg),

	int array[] = {
		DT_FOREACH_PROP_ELEM_VARGS(TEST_ARRAYS, a, TIMES_TWO_ADD, 3)
	};

	zassert_equal(ARRAY_SIZE(array), 3, "");
	zassert_equal(array[0], 2003, "");
	zassert_equal(array[1], 4003, "");
	zassert_equal(array[2], 6003, "");

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_array_holder

	int inst_array[] = {
		DT_INST_FOREACH_PROP_ELEM_VARGS(0, a, TIMES_TWO_ADD, 3)
	};

	zassert_equal(ARRAY_SIZE(inst_array), ARRAY_SIZE(array), "");
	zassert_equal(inst_array[0], array[0], "");
	zassert_equal(inst_array[1], array[1], "");
	zassert_equal(inst_array[2], array[2], "");
#undef TIMES_TWO
}

struct test_gpio_info {
	uint32_t reg_addr;
	uint32_t reg_len;
};

struct test_gpio_data {
	bool init_called;
	bool is_gpio_ctlr;
};

static int test_gpio_init(const struct device *dev)
{
	struct test_gpio_data *data = dev->data;

	data->init_called = 1;
	return 0;
}

#define INST(num) DT_INST(num, vnd_gpio_device)
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_gpio_device

static const struct gpio_driver_api test_api;

#define TEST_GPIO_INIT(num)					\
	static struct test_gpio_data gpio_data_##num = {	\
		.is_gpio_ctlr = DT_PROP(INST(num),		\
					gpio_controller),	\
	};							\
	static const struct test_gpio_info gpio_info_##num = {	\
		.reg_addr = DT_REG_ADDR(INST(num)),		\
		.reg_len = DT_REG_SIZE(INST(num)),		\
	};							\
	DEVICE_DT_DEFINE(INST(num),				\
			    test_gpio_init,			\
			    NULL,				\
			    &gpio_data_##num,			\
			    &gpio_info_##num,			\
			    POST_KERNEL,			\
			    CONFIG_APPLICATION_INIT_PRIORITY,	\
			    &test_api);

DT_INST_FOREACH_STATUS_OKAY(TEST_GPIO_INIT)

ZTEST(devicetree_api, test_devices)
{
	const struct device *devs[3];
	int i = 0;
	const struct device *dev_abcd;
	struct test_gpio_data *data_dev0;
	struct test_gpio_data *data_dev1;
	const struct test_gpio_info *config_abdc;

	zassert_equal(DT_NUM_INST_STATUS_OKAY(vnd_gpio_device), 2, "");

	devs[i] = device_get_binding(DT_LABEL(INST(0)));
	if (devs[i]) {
		i++;
	}
	devs[i] = device_get_binding(DT_LABEL(INST(1)));
	if (devs[i]) {
		i++;
	}
	devs[i] = device_get_binding(DT_LABEL(INST(2)));
	if (devs[i]) {
		i++;
	}

	data_dev0 = devs[0]->data;
	data_dev1 = devs[1]->data;

	zassert_not_null(devs[0], "");
	zassert_not_null(devs[1], "");
	zassert_true(devs[2] == NULL, "");

	zassert_true(data_dev0->is_gpio_ctlr, "");
	zassert_true(data_dev1->is_gpio_ctlr, "");
	zassert_true(data_dev0->init_called, "");
	zassert_true(data_dev1->init_called, "");

	dev_abcd = DEVICE_DT_GET(TEST_ABCD1234);
	config_abdc = dev_abcd->config;
	zassert_not_null(dev_abcd, "");
	zassert_equal(config_abdc->reg_addr, 0xabcd1234, "");
	zassert_equal(config_abdc->reg_len, 0x500, "");
}

ZTEST(devicetree_api, test_cs_gpios)
{
	zassert_equal(DT_SPI_HAS_CS_GPIOS(TEST_SPI_NO_CS), 0, "");
	zassert_equal(DT_SPI_NUM_CS_GPIOS(TEST_SPI_NO_CS), 0, "");

	zassert_equal(DT_SPI_HAS_CS_GPIOS(TEST_SPI), 1, "");
	zassert_equal(DT_SPI_NUM_CS_GPIOS(TEST_SPI), 3, "");

	zassert_equal(DT_DEP_ORD(DT_SPI_DEV_CS_GPIOS_CTLR(TEST_SPI_DEV_0)),
		      DT_DEP_ORD(DT_NODELABEL(test_gpio_1)),
		     "dev 0 cs gpio controller");
	zassert_true(!strcmp(DT_SPI_DEV_CS_GPIOS_LABEL(TEST_SPI_DEV_0),
			     "TEST_GPIO_1"), "");
	zassert_equal(DT_SPI_DEV_CS_GPIOS_PIN(TEST_SPI_DEV_0), 0x10, "");
	zassert_equal(DT_SPI_DEV_CS_GPIOS_FLAGS(TEST_SPI_DEV_0), 0x20, "");
}

ZTEST(devicetree_api, test_chosen)
{
	zassert_equal(DT_HAS_CHOSEN(ztest_xxxx), 0, "");
	zassert_equal(DT_HAS_CHOSEN(ztest_gpio), 1, "");
	zassert_true(!strcmp(TO_STRING(DT_CHOSEN(ztest_gpio)),
			     "DT_N_S_test_S_gpio_deadbeef"), "");
}

#define TO_MY_ENUM(token) TO_MY_ENUM_2(token) /* force another expansion */
#define TO_MY_ENUM_2(token) MY_ENUM_ ## token
ZTEST(devicetree_api, test_enums)
{
	enum {
		MY_ENUM_zero = 0xff,
		MY_ENUM_ZERO = 0xaa,
	};

	zassert_equal(DT_ENUM_IDX(TEST_ENUM_0, val), 0, "0");
}
#undef TO_MY_ENUM
#undef TO_MY_ENUM_2

ZTEST(devicetree_api, test_enums_required_false)
{
	/* DT_ENUM_IDX_OR on string value */
	zassert_equal(DT_ENUM_IDX_OR(DT_NODELABEL(test_enum_default_0), val, 2),
		      1, "");
	zassert_equal(DT_ENUM_IDX_OR(DT_NODELABEL(test_enum_default_1), val, 2),
		      2, "");
	/* DT_ENUM_IDX_OR on int value */
	zassert_equal(DT_ENUM_IDX_OR(DT_NODELABEL(test_enum_int_default_0),
				     val, 4),
		      0, "");
	zassert_equal(DT_ENUM_IDX_OR(DT_NODELABEL(test_enum_int_default_1),
				     val, 4),
		      4, "");
}

ZTEST(devicetree_api, test_inst_enums)
{
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_enum_holder_inst
	zassert_equal(DT_INST_ENUM_IDX(0, val), 0, "");
	zassert_equal(DT_INST_ENUM_IDX_OR(0, val, 2), 0, "");

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_enum_required_false_holder_inst
	zassert_equal(DT_INST_ENUM_IDX_OR(0, val, 2), 2, "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_adc_temp_sensor
ZTEST(devicetree_api, test_clocks)
{
	/* DT_CLOCKS_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_CLOCKS_CTLR_BY_IDX(TEST_TEMP, 1),
				  DT_NODELABEL(test_fixed_clk)), "");

	/* DT_CLOCKS_CTLR */
	zassert_true(DT_SAME_NODE(DT_CLOCKS_CTLR(TEST_TEMP),
				  DT_NODELABEL(test_clk)), "");

	/* DT_CLOCKS_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_CLOCKS_CTLR_BY_NAME(TEST_TEMP, clk_b),
				  DT_NODELABEL(test_clk)), "");

	/* DT_NUM_CLOCKS */
	zassert_equal(DT_NUM_CLOCKS(TEST_TEMP), 3, "");

	/* DT_CLOCKS_HAS_IDX */
	zassert_true(DT_CLOCKS_HAS_IDX(TEST_TEMP, 2), "");
	zassert_false(DT_CLOCKS_HAS_IDX(TEST_TEMP, 3), "");

	/* DT_CLOCKS_HAS_NAME */
	zassert_true(DT_CLOCKS_HAS_NAME(TEST_TEMP, clk_a), "");
	zassert_false(DT_CLOCKS_HAS_NAME(TEST_TEMP, clk_z), "");

	/* DT_CLOCKS_CELL_BY_IDX */
	zassert_equal(DT_CLOCKS_CELL_BY_IDX(TEST_TEMP, 2, bits), 2, "");
	zassert_equal(DT_CLOCKS_CELL_BY_IDX(TEST_TEMP, 2, bus), 8, "");

	/* DT_CLOCKS_CELL_BY_NAME */
	zassert_equal(DT_CLOCKS_CELL_BY_NAME(TEST_TEMP, clk_a, bits), 7, "");
	zassert_equal(DT_CLOCKS_CELL_BY_NAME(TEST_TEMP, clk_b, bus), 8, "");

	/* DT_CLOCKS_CELL */
	zassert_equal(DT_CLOCKS_CELL(TEST_TEMP, bits), 7, "");
	zassert_equal(DT_CLOCKS_CELL(TEST_TEMP, bus), 3, "");

	/* clock-freq on fixed clock */
	zassert_equal(DT_PROP_BY_PHANDLE_IDX(TEST_TEMP, clocks, 1,
					     clock_frequency),
		      25000000, "");

	/* DT_INST */
	zassert_equal(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT), 1, "");

	/* DT_INST_CLOCKS_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_INST_CLOCKS_CTLR_BY_IDX(0, 1),
				  DT_NODELABEL(test_fixed_clk)), "");

	/* DT_INST_CLOCKS_CTLR */
	zassert_true(DT_SAME_NODE(DT_INST_CLOCKS_CTLR(0),
				  DT_NODELABEL(test_clk)), "");

	/* DT_INST_CLOCKS_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_INST_CLOCKS_CTLR_BY_NAME(0, clk_b),
				  DT_NODELABEL(test_clk)), "");

	/* DT_INST_NUM_CLOCKS */
	zassert_equal(DT_INST_NUM_CLOCKS(0), 3, "");

	/* DT_INST_CLOCKS_HAS_IDX */
	zassert_true(DT_INST_CLOCKS_HAS_IDX(0, 2), "");
	zassert_false(DT_INST_CLOCKS_HAS_IDX(0, 3), "");

	/* DT_INST_CLOCKS_HAS_NAME */
	zassert_true(DT_INST_CLOCKS_HAS_NAME(0, clk_a), "");
	zassert_false(DT_INST_CLOCKS_HAS_NAME(0, clk_z), "");

	/* DT_INST_CLOCKS_CELL_BY_IDX */
	zassert_equal(DT_INST_CLOCKS_CELL_BY_IDX(0, 2, bits), 2, "");
	zassert_equal(DT_INST_CLOCKS_CELL_BY_IDX(0, 2, bus), 8, "");

	/* DT_INST_CLOCKS_CELL_BY_NAME */
	zassert_equal(DT_INST_CLOCKS_CELL_BY_NAME(0, clk_a, bits), 7, "");
	zassert_equal(DT_INST_CLOCKS_CELL_BY_NAME(0, clk_b, bus), 8, "");

	/* DT_INST_CLOCKS_CELL */
	zassert_equal(DT_INST_CLOCKS_CELL(0, bits), 7, "");
	zassert_equal(DT_INST_CLOCKS_CELL(0, bus), 3, "");

	/* clock-freq on fixed clock */
	zassert_equal(DT_INST_PROP_BY_PHANDLE_IDX(0, clocks, 1,
						  clock_frequency),
		      25000000, "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_spi_device
ZTEST(devicetree_api, test_parent)
{
	zassert_true(DT_SAME_NODE(DT_PARENT(TEST_SPI_DEV_0), TEST_SPI_BUS_0), "");

	/*
	 * The parent's label for the first instance of vnd,spi-device,
	 * child of TEST_SPI, is the same as TEST_SPI.
	 */
	zassert_true(DT_SAME_NODE(DT_INST_PARENT(0), TEST_SPI), "");
	/*
	 * We should be able to use DT_PARENT() even with nodes, like /test,
	 * that have no matching compatible.
	 */
	zassert_true(DT_SAME_NODE(DT_CHILD(DT_PARENT(TEST_SPI_BUS_0), spi_33334444),
				  TEST_SPI_BUS_0), "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_child_bindings
ZTEST(devicetree_api, test_child_nodes_list)
{
	#define TEST_FUNC(child) { DT_PROP(child, val) },
	#define TEST_PARENT DT_PARENT(DT_NODELABEL(test_child_a))

	struct vnd_child_binding {
		int val;
	};

	struct vnd_child_binding vals[] = {
		DT_FOREACH_CHILD(TEST_PARENT, TEST_FUNC)
	};

	struct vnd_child_binding vals_inst[] = {
		DT_INST_FOREACH_CHILD(0, TEST_FUNC)
	};

	struct vnd_child_binding vals_status_okay[] = {
		DT_FOREACH_CHILD_STATUS_OKAY(TEST_PARENT, TEST_FUNC)
	};

	zassert_equal(ARRAY_SIZE(vals), 3, "");
	zassert_equal(ARRAY_SIZE(vals_inst), 3, "");
	zassert_equal(ARRAY_SIZE(vals_status_okay), 2, "");

	zassert_equal(vals[0].val, 0, "");
	zassert_equal(vals[1].val, 1, "");
	zassert_equal(vals[2].val, 2, "");
	zassert_equal(vals_inst[0].val, 0, "");
	zassert_equal(vals_inst[1].val, 1, "");
	zassert_equal(vals_inst[2].val, 2, "");
	zassert_equal(vals_status_okay[0].val, 0, "");
	zassert_equal(vals_status_okay[1].val, 1, "");

	#undef TEST_PARENT
	#undef TEST_FUNC
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_child_bindings
ZTEST(devicetree_api, test_child_nodes_list_varg)
{
	#define TEST_FUNC(child, arg) { DT_PROP(child, val) + arg },
	#define TEST_PARENT DT_PARENT(DT_NODELABEL(test_child_a))

	struct vnd_child_binding {
		int val;
	};

	struct vnd_child_binding vals[] = {
		DT_FOREACH_CHILD_VARGS(TEST_PARENT, TEST_FUNC, 1)
	};

	struct vnd_child_binding vals_inst[] = {
		DT_INST_FOREACH_CHILD_VARGS(0, TEST_FUNC, 1)
	};

	struct vnd_child_binding vals_status_okay[] = {
		DT_FOREACH_CHILD_STATUS_OKAY_VARGS(TEST_PARENT, TEST_FUNC, 1)
	};

	zassert_equal(ARRAY_SIZE(vals), 3, "");
	zassert_equal(ARRAY_SIZE(vals_inst), 3, "");
	zassert_equal(ARRAY_SIZE(vals_status_okay), 2, "");

	zassert_equal(vals[0].val, 1, "");
	zassert_equal(vals[1].val, 2, "");
	zassert_equal(vals[2].val, 3, "");
	zassert_equal(vals_inst[0].val, 1, "");
	zassert_equal(vals_inst[1].val, 2, "");
	zassert_equal(vals_inst[2].val, 3, "");
	zassert_equal(vals_status_okay[0].val, 1, "");
	zassert_equal(vals_status_okay[1].val, 2, "");

	#undef TEST_PARENT
	#undef TEST_FUNC
}

ZTEST(devicetree_api, test_great_grandchild)
{
	zassert_equal(DT_PROP(DT_NODELABEL(test_ggc), ggc_prop), 42, "");
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_test_ranges_pcie
ZTEST(devicetree_api, test_ranges_pcie)
{
#define FLAGS(node_id, idx)				\
	DT_RANGES_CHILD_BUS_FLAGS_BY_IDX(node_id, idx),
#define CHILD_BUS_ADDR(node_id, idx)				\
	DT_RANGES_CHILD_BUS_ADDRESS_BY_IDX(node_id, idx),
#define PARENT_BUS_ADDR(node_id, idx)				\
	DT_RANGES_PARENT_BUS_ADDRESS_BY_IDX(node_id, idx),
#define LENGTH(node_id, idx) DT_RANGES_LENGTH_BY_IDX(node_id, idx),

	unsigned int count = DT_NUM_RANGES(TEST_RANGES_PCIE);

	const uint64_t ranges_pcie_flags[] = {
		DT_FOREACH_RANGE(TEST_RANGES_PCIE, FLAGS)
	};

	const uint64_t ranges_child_bus_addr[] = {
		DT_FOREACH_RANGE(TEST_RANGES_PCIE, CHILD_BUS_ADDR)
	};

	const uint64_t ranges_parent_bus_addr[] = {
		DT_FOREACH_RANGE(TEST_RANGES_PCIE, PARENT_BUS_ADDR)
	};

	const uint64_t ranges_length[] = {
		DT_FOREACH_RANGE(TEST_RANGES_PCIE, LENGTH)
	};

	zassert_equal(count, 3, "");

	zassert_equal(DT_RANGES_HAS_IDX(TEST_RANGES_PCIE, 0), 1, "");
	zassert_equal(DT_RANGES_HAS_IDX(TEST_RANGES_PCIE, 1), 1, "");
	zassert_equal(DT_RANGES_HAS_IDX(TEST_RANGES_PCIE, 2), 1, "");
	zassert_equal(DT_RANGES_HAS_IDX(TEST_RANGES_PCIE, 3), 0, "");

	zassert_equal(DT_RANGES_HAS_CHILD_BUS_FLAGS_AT_IDX(TEST_RANGES_PCIE, 0),
		      1, "");
	zassert_equal(DT_RANGES_HAS_CHILD_BUS_FLAGS_AT_IDX(TEST_RANGES_PCIE, 1),
		      1, "");
	zassert_equal(DT_RANGES_HAS_CHILD_BUS_FLAGS_AT_IDX(TEST_RANGES_PCIE, 2),
		      1, "");
	zassert_equal(DT_RANGES_HAS_CHILD_BUS_FLAGS_AT_IDX(TEST_RANGES_PCIE, 3),
		      0, "");

	zassert_equal(ranges_pcie_flags[0], 0x1000000, "");
	zassert_equal(ranges_pcie_flags[1], 0x2000000, "");
	zassert_equal(ranges_pcie_flags[2], 0x3000000, "");
	zassert_equal(ranges_child_bus_addr[0], 0, "");
	zassert_equal(ranges_child_bus_addr[1], 0x10000000, "");
	zassert_equal(ranges_child_bus_addr[2], 0x8000000000, "");
	zassert_equal(ranges_parent_bus_addr[0], 0x3eff0000, "");
	zassert_equal(ranges_parent_bus_addr[1], 0x10000000, "");
	zassert_equal(ranges_parent_bus_addr[2], 0x8000000000, "");
	zassert_equal(ranges_length[0], 0x10000, "");
	zassert_equal(ranges_length[1], 0x2eff0000, "");
	zassert_equal(ranges_length[2], 0x8000000000, "");

#undef FLAGS
#undef CHILD_BUS_ADDR
#undef PARENT_BUS_ADDR
#undef LENGTH
}

ZTEST(devicetree_api, test_ranges_other)
{
#define HAS_FLAGS(node_id, idx) \
	DT_RANGES_HAS_CHILD_BUS_FLAGS_AT_IDX(node_id, idx)
#define FLAGS(node_id, idx) \
	DT_RANGES_CHILD_BUS_FLAGS_BY_IDX(node_id, idx),
#define CHILD_BUS_ADDR(node_id, idx) \
	DT_RANGES_CHILD_BUS_ADDRESS_BY_IDX(node_id, idx),
#define PARENT_BUS_ADDR(node_id, idx) \
	DT_RANGES_PARENT_BUS_ADDRESS_BY_IDX(node_id, idx),
#define LENGTH(node_id, idx) DT_RANGES_LENGTH_BY_IDX(node_id, idx),

	unsigned int count = DT_NUM_RANGES(TEST_RANGES_OTHER);

	const uint32_t ranges_child_bus_addr[] = {
		DT_FOREACH_RANGE(TEST_RANGES_OTHER, CHILD_BUS_ADDR)
	};

	const uint32_t ranges_parent_bus_addr[] = {
		DT_FOREACH_RANGE(TEST_RANGES_OTHER, PARENT_BUS_ADDR)
	};

	const uint32_t ranges_length[] = {
		DT_FOREACH_RANGE(TEST_RANGES_OTHER, LENGTH)
	};

	zassert_equal(count, 2, "");

	zassert_equal(DT_RANGES_HAS_IDX(TEST_RANGES_OTHER, 0), 1, "");
	zassert_equal(DT_RANGES_HAS_IDX(TEST_RANGES_OTHER, 1), 1, "");
	zassert_equal(DT_RANGES_HAS_IDX(TEST_RANGES_OTHER, 2), 0, "");
	zassert_equal(DT_RANGES_HAS_IDX(TEST_RANGES_OTHER, 3), 0, "");

	zassert_equal(HAS_FLAGS(TEST_RANGES_OTHER, 0), 0, "");
	zassert_equal(HAS_FLAGS(TEST_RANGES_OTHER, 1), 0, "");
	zassert_equal(HAS_FLAGS(TEST_RANGES_OTHER, 2), 0, "");
	zassert_equal(HAS_FLAGS(TEST_RANGES_OTHER, 3), 0, "");

	zassert_equal(ranges_child_bus_addr[0], 0, "");
	zassert_equal(ranges_child_bus_addr[1], 0x10000000, "");
	zassert_equal(ranges_parent_bus_addr[0], 0x3eff0000, "");
	zassert_equal(ranges_parent_bus_addr[1], 0x10000000, "");
	zassert_equal(ranges_length[0], 0x10000, "");
	zassert_equal(ranges_length[1], 0x2eff0000, "");

#undef HAS_FLAGS
#undef FLAGS
#undef CHILD_BUS_ADDR
#undef PARENT_BUS_ADDR
#undef LENGTH
}

ZTEST(devicetree_api, test_compat_get_any_status_okay)
{
	zassert_true(
		DT_SAME_NODE(
			DT_COMPAT_GET_ANY_STATUS_OKAY(vnd_reg_holder),
			TEST_REG),
		"");

	/*
	 * DT_SAME_NODE requires that both its arguments are valid
	 * node identifiers, so we can't pass it DT_INVALID_NODE,
	 * which is what this DT_COMPAT_GET_ANY_STATUS_OKAY() expands to.
	 */
	zassert_false(
		DT_NODE_EXISTS(
			DT_COMPAT_GET_ANY_STATUS_OKAY(this_is_not_a_real_compat)),
		"");
}

static bool ord_in_array(unsigned int ord, unsigned int *array,
			 size_t array_size)
{
	size_t i;

	for (i = 0; i < array_size; i++) {
		if (array[i] == ord) {
			return true;
		}
	}

	return false;
}

/* Magic numbers used by COMBINED_ORD_ARRAY. Must be invalid dependency
 * ordinals.
 */
#define ORD_LIST_SEP		0xFFFF0000
#define ORD_LIST_END		0xFFFF0001
#define INJECTED_DEP_0		0xFFFF0002
#define INJECTED_DEP_1		0xFFFF0003

#define DEP_ORD_AND_COMMA(node_id) DT_DEP_ORD(node_id),
#define CHILD_ORDINALS(node_id) DT_FOREACH_CHILD(node_id, DEP_ORD_AND_COMMA)

#define COMBINED_ORD_ARRAY(node_id)		\
	{					\
		DT_DEP_ORD(node_id),		\
		DT_DEP_ORD(DT_PARENT(node_id)),	\
		CHILD_ORDINALS(node_id)		\
		ORD_LIST_SEP,			\
		DT_REQUIRES_DEP_ORDS(node_id)	\
		INJECTED_DEP_0,			\
		INJECTED_DEP_1,			\
		ORD_LIST_SEP,			\
		DT_SUPPORTS_DEP_ORDS(node_id)	\
		ORD_LIST_END			\
	}

ZTEST(devicetree_api, test_dep_ord)
{
#define ORD_IN_ARRAY(ord, array) ord_in_array(ord, array, ARRAY_SIZE(array))

	unsigned int root_ord = DT_DEP_ORD(DT_ROOT),
		test_ord = DT_DEP_ORD(DT_PATH(test)),
		root_requires[] = { DT_REQUIRES_DEP_ORDS(DT_ROOT) },
		test_requires[] = { DT_REQUIRES_DEP_ORDS(DT_PATH(test)) },
		root_supports[] = { DT_SUPPORTS_DEP_ORDS(DT_ROOT) },
		test_supports[] = { DT_SUPPORTS_DEP_ORDS(DT_PATH(test)) },
		children_ords[] = {
			DT_FOREACH_CHILD(TEST_CHILDREN, DEP_ORD_AND_COMMA)
		},
		children_combined_ords[] = COMBINED_ORD_ARRAY(TEST_CHILDREN),
		child_a_combined_ords[] =
			COMBINED_ORD_ARRAY(DT_NODELABEL(test_child_a));
	size_t i;

	/* DT_DEP_ORD */
	zassert_equal(root_ord, 0, "");
	zassert_true(DT_DEP_ORD(DT_NODELABEL(test_child_a)) >
		     DT_DEP_ORD(DT_NODELABEL(test_children)), "");
	zassert_true(DT_DEP_ORD(DT_NODELABEL(test_irq)) >
		     DT_DEP_ORD(DT_NODELABEL(test_intc)), "");
	zassert_true(DT_DEP_ORD(DT_NODELABEL(test_phandles)) >
		     DT_DEP_ORD(DT_NODELABEL(test_gpio_1)), "");

	/* DT_REQUIRES_DEP_ORDS */
	zassert_equal(ARRAY_SIZE(root_requires), 0, "");
	zassert_true(ORD_IN_ARRAY(root_ord, test_requires), "");

	/* DT_SUPPORTS_DEP_ORDS */
	zassert_true(ORD_IN_ARRAY(test_ord, root_supports), "");
	zassert_false(ORD_IN_ARRAY(root_ord, test_supports), "");

	unsigned int children_combined_ords_expected[] = {
		/*
		 * Combined ordinals for /test/test-children are from
		 * these nodes in this order:
		 */
		DT_DEP_ORD(TEST_CHILDREN),		/* node */
		DT_DEP_ORD(DT_PATH(test)),		/* parent */
		DT_DEP_ORD(DT_NODELABEL(test_child_a)),	/* children */
		DT_DEP_ORD(DT_NODELABEL(test_child_b)),
		DT_DEP_ORD(DT_NODELABEL(test_child_c)),
		ORD_LIST_SEP,				/* separator */
		DT_DEP_ORD(DT_PATH(test)),		/* requires */
		INJECTED_DEP_0,				/* injected
							 * dependencies
							 */
		INJECTED_DEP_1,
		ORD_LIST_SEP,				/* separator */
		DT_DEP_ORD(DT_NODELABEL(test_child_a)),	/* supports */
		DT_DEP_ORD(DT_NODELABEL(test_child_b)),
		DT_DEP_ORD(DT_NODELABEL(test_child_c)),
		ORD_LIST_END,				/* terminator */
	};
	zassert_equal(ARRAY_SIZE(children_combined_ords),
		      ARRAY_SIZE(children_combined_ords_expected),
		      "%zu", ARRAY_SIZE(children_combined_ords));
	for (i = 0; i < ARRAY_SIZE(children_combined_ords); i++) {
		zassert_equal(children_combined_ords[i],
			      children_combined_ords_expected[i],
			      "test-children at %zu", i);
	}

	unsigned int child_a_combined_ords_expected[] = {
		/*
		 * Combined ordinals for /test/test-children/child-a
		 * are from these nodes in this order:
		 */
		DT_DEP_ORD(DT_NODELABEL(test_child_a)), /* node */
		DT_DEP_ORD(TEST_CHILDREN),		/* parent */
		/* children (none) */
		ORD_LIST_SEP,				/* separator */
		DT_DEP_ORD(TEST_CHILDREN),		/* requires */
		INJECTED_DEP_0,				/* injected
							 * dependencies
							 */
		INJECTED_DEP_1,
		ORD_LIST_SEP,				/* separator */
		/* supports (none) */
		ORD_LIST_END,				/* terminator */
	};
	zassert_equal(ARRAY_SIZE(child_a_combined_ords),
		      ARRAY_SIZE(child_a_combined_ords_expected),
		      "%zu", ARRAY_SIZE(child_a_combined_ords));
	for (i = 0; i < ARRAY_SIZE(child_a_combined_ords); i++) {
		zassert_equal(child_a_combined_ords[i],
			      child_a_combined_ords_expected[i],
			      "child-a at %zu", i);
	}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_child_bindings

	/* DT_INST_DEP_ORD */
	zassert_equal(DT_INST_DEP_ORD(0),
		      DT_DEP_ORD(DT_NODELABEL(test_children)), "");

	/* DT_INST_REQUIRES_DEP_ORDS */
	unsigned int inst_requires[] = { DT_INST_REQUIRES_DEP_ORDS(0) };

	zassert_equal(ARRAY_SIZE(inst_requires), 1, "");
	zassert_equal(inst_requires[0], test_ord, "");

	/* DT_INST_SUPPORTS_DEP_ORDS */
	unsigned int inst_supports[] = { DT_INST_SUPPORTS_DEP_ORDS(0) };

	zassert_equal(ARRAY_SIZE(inst_supports), 3, "");
	for (i = 0; i < ARRAY_SIZE(inst_supports); i++) {
		zassert_true(ORD_IN_ARRAY(inst_supports[i], children_ords), "");
	}
}

ZTEST(devicetree_api, test_path)
{
	zassert_true(!strcmp(DT_NODE_PATH(DT_ROOT), "/"), "");
	zassert_true(!strcmp(DT_NODE_PATH(TEST_DEADBEEF),
			     "/test/gpio@deadbeef"), "");
}

ZTEST(devicetree_api, test_node_name)
{
	zassert_true(!strcmp(DT_NODE_FULL_NAME(DT_ROOT), "/"), "");
	zassert_true(!strcmp(DT_NODE_FULL_NAME(TEST_DEADBEEF),
			     "gpio@deadbeef"), "");
	zassert_true(!strcmp(DT_NODE_FULL_NAME(TEST_TEMP),
			     "temperature-sensor"), "");
	zassert_true(strcmp(DT_NODE_FULL_NAME(TEST_REG),
			     "reg-holder"), "");
}

ZTEST(devicetree_api, test_node_child_idx)
{
	zassert_equal(DT_NODE_CHILD_IDX(DT_NODELABEL(test_child_a)), 0, "");
	zassert_equal(DT_NODE_CHILD_IDX(DT_NODELABEL(test_child_b)), 1, "");
	zassert_equal(DT_NODE_CHILD_IDX(DT_NODELABEL(test_child_c)), 2, "");
}

ZTEST(devicetree_api, test_same_node)
{
	zassert_true(DT_SAME_NODE(TEST_DEADBEEF, TEST_DEADBEEF), "");
	zassert_false(DT_SAME_NODE(TEST_DEADBEEF, TEST_ABCD1234), "");
}

ZTEST(devicetree_api, test_pinctrl)
{
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_adc_temp_sensor
	/*
	 * Tests when a node does have pinctrl properties.
	 */

	/*
	 * node_id versions:
	 */

	zassert_true(DT_SAME_NODE(DT_PINCTRL_BY_IDX(TEST_TEMP, 0, 1),
				  DT_NODELABEL(test_pincfg_b)), "");
	zassert_true(DT_SAME_NODE(DT_PINCTRL_BY_IDX(TEST_TEMP, 1, 0),
				  DT_NODELABEL(test_pincfg_c)), "");

	zassert_true(DT_SAME_NODE(DT_PINCTRL_0(TEST_TEMP, 0),
				  DT_NODELABEL(test_pincfg_a)), "");

	zassert_true(DT_SAME_NODE(DT_PINCTRL_BY_NAME(TEST_TEMP, default, 1),
				  DT_NODELABEL(test_pincfg_b)), "");
	zassert_true(DT_SAME_NODE(DT_PINCTRL_BY_NAME(TEST_TEMP, sleep, 0),
				  DT_NODELABEL(test_pincfg_c)), "");
	zassert_true(DT_SAME_NODE(DT_PINCTRL_BY_NAME(TEST_TEMP, f_o_o2, 0),
				  DT_NODELABEL(test_pincfg_d)), "");

	zassert_equal(DT_PINCTRL_NAME_TO_IDX(TEST_TEMP, default), 0, "");
	zassert_equal(DT_PINCTRL_NAME_TO_IDX(TEST_TEMP, sleep), 1, "");
	zassert_equal(DT_PINCTRL_NAME_TO_IDX(TEST_TEMP, f_o_o2), 2, "");

	zassert_equal(DT_NUM_PINCTRLS_BY_IDX(TEST_TEMP, 0), 2, "");

	zassert_equal(DT_NUM_PINCTRLS_BY_NAME(TEST_TEMP, default), 2, "");
	zassert_equal(DT_NUM_PINCTRLS_BY_NAME(TEST_TEMP, f_o_o2), 1, "");

	zassert_equal(DT_NUM_PINCTRL_STATES(TEST_TEMP), 3, "");

	zassert_equal(DT_PINCTRL_HAS_IDX(TEST_TEMP, 0), 1, "");
	zassert_equal(DT_PINCTRL_HAS_IDX(TEST_TEMP, 1), 1, "");
	zassert_equal(DT_PINCTRL_HAS_IDX(TEST_TEMP, 2), 1, "");
	zassert_equal(DT_PINCTRL_HAS_IDX(TEST_TEMP, 3), 0, "");

	zassert_equal(DT_PINCTRL_HAS_NAME(TEST_TEMP, default), 1, "");
	zassert_equal(DT_PINCTRL_HAS_NAME(TEST_TEMP, sleep), 1, "");
	zassert_equal(DT_PINCTRL_HAS_NAME(TEST_TEMP, f_o_o2), 1, "");
	zassert_equal(DT_PINCTRL_HAS_NAME(TEST_TEMP, bar), 0, "");

#undef MAKE_TOKEN
#define MAKE_TOKEN(pc_idx)						\
	_CONCAT(NODE_ID_ENUM_,						\
		DT_PINCTRL_IDX_TO_NAME_TOKEN(TEST_TEMP, pc_idx))
#undef MAKE_UPPER_TOKEN
#define MAKE_UPPER_TOKEN(pc_idx)					\
	_CONCAT(NODE_ID_ENUM_,						\
		DT_PINCTRL_IDX_TO_NAME_UPPER_TOKEN(TEST_TEMP, pc_idx))
	enum {
		MAKE_TOKEN(0) = 10,
		MAKE_TOKEN(1) = 11,
		MAKE_TOKEN(2) = 12,
		MAKE_TOKEN(3) = 13,

		MAKE_UPPER_TOKEN(0) = 20,
		MAKE_UPPER_TOKEN(1) = 21,
		MAKE_UPPER_TOKEN(2) = 22,
		MAKE_UPPER_TOKEN(3) = 23,
	};

	zassert_equal(NODE_ID_ENUM_default, 10, "");
	zassert_equal(NODE_ID_ENUM_sleep, 11, "");
	zassert_equal(NODE_ID_ENUM_f_o_o2, 12, "");

	zassert_equal(NODE_ID_ENUM_DEFAULT, 20, "");
	zassert_equal(NODE_ID_ENUM_SLEEP, 21, "");
	zassert_equal(NODE_ID_ENUM_F_O_O2, 22, "");

	/*
	 * inst versions:
	 */

	zassert_true(DT_SAME_NODE(DT_INST_PINCTRL_BY_IDX(0, 0, 1),
				  DT_NODELABEL(test_pincfg_b)), "");
	zassert_true(DT_SAME_NODE(DT_INST_PINCTRL_BY_IDX(0, 1, 0),
				  DT_NODELABEL(test_pincfg_c)), "");

	zassert_true(DT_SAME_NODE(DT_INST_PINCTRL_0(0, 0),
				  DT_NODELABEL(test_pincfg_a)), "");

	zassert_true(DT_SAME_NODE(DT_INST_PINCTRL_BY_NAME(0, default, 1),
				  DT_NODELABEL(test_pincfg_b)), "");
	zassert_true(DT_SAME_NODE(DT_INST_PINCTRL_BY_NAME(0, sleep, 0),
				  DT_NODELABEL(test_pincfg_c)), "");
	zassert_true(DT_SAME_NODE(DT_INST_PINCTRL_BY_NAME(0, f_o_o2, 0),
				  DT_NODELABEL(test_pincfg_d)), "");

	zassert_equal(DT_INST_PINCTRL_NAME_TO_IDX(0, default), 0, "");
	zassert_equal(DT_INST_PINCTRL_NAME_TO_IDX(0, sleep), 1, "");
	zassert_equal(DT_INST_PINCTRL_NAME_TO_IDX(0, f_o_o2), 2, "");

	zassert_equal(DT_INST_NUM_PINCTRLS_BY_IDX(0, 0), 2, "");

	zassert_equal(DT_INST_NUM_PINCTRLS_BY_NAME(0, default), 2, "");
	zassert_equal(DT_INST_NUM_PINCTRLS_BY_NAME(0, f_o_o2), 1, "");

	zassert_equal(DT_INST_NUM_PINCTRL_STATES(0), 3, "");

	zassert_equal(DT_INST_PINCTRL_HAS_IDX(0, 0), 1, "");
	zassert_equal(DT_INST_PINCTRL_HAS_IDX(0, 1), 1, "");
	zassert_equal(DT_INST_PINCTRL_HAS_IDX(0, 2), 1, "");
	zassert_equal(DT_INST_PINCTRL_HAS_IDX(0, 3), 0, "");

	zassert_equal(DT_INST_PINCTRL_HAS_NAME(0, default), 1, "");
	zassert_equal(DT_INST_PINCTRL_HAS_NAME(0, sleep), 1, "");
	zassert_equal(DT_INST_PINCTRL_HAS_NAME(0, f_o_o2), 1, "");
	zassert_equal(DT_INST_PINCTRL_HAS_NAME(0, bar), 0, "");

#undef MAKE_TOKEN
#define MAKE_TOKEN(pc_idx)						\
	_CONCAT(INST_ENUM_,						\
			DT_INST_PINCTRL_IDX_TO_NAME_TOKEN(0, pc_idx))
#undef MAKE_UPPER_TOKEN
#define MAKE_UPPER_TOKEN(pc_idx)					\
	_CONCAT(INST_ENUM_,						\
			DT_INST_PINCTRL_IDX_TO_NAME_UPPER_TOKEN(0, pc_idx))
	enum {
		MAKE_TOKEN(0) = 10,
		MAKE_TOKEN(1) = 11,
		MAKE_TOKEN(2) = 12,

		MAKE_UPPER_TOKEN(0) = 20,
		MAKE_UPPER_TOKEN(1) = 21,
		MAKE_UPPER_TOKEN(2) = 22,
	};

	zassert_equal(INST_ENUM_default, 10, "");
	zassert_equal(INST_ENUM_sleep, 11, "");
	zassert_equal(INST_ENUM_f_o_o2, 12, "");

	zassert_equal(INST_ENUM_DEFAULT, 20, "");
	zassert_equal(INST_ENUM_SLEEP, 21, "");
	zassert_equal(INST_ENUM_F_O_O2, 22, "");

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_reg_holder
	/*
	 * Tests when a node does NOT have any pinctrl properties.
	 */

	/* node_id versions */
	zassert_equal(DT_NUM_PINCTRL_STATES(TEST_REG), 0, "");
	zassert_equal(DT_PINCTRL_HAS_IDX(TEST_REG, 0), 0, "");
	zassert_equal(DT_PINCTRL_HAS_NAME(TEST_REG, f_o_o2), 0, "");

	/* inst versions */
	zassert_equal(DT_INST_NUM_PINCTRL_STATES(0), 0, "");
	zassert_equal(DT_INST_PINCTRL_HAS_IDX(0, 0), 0, "");
	zassert_equal(DT_INST_PINCTRL_HAS_NAME(0, f_o_o2), 0, "");
}

static int test_mbox_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	return 0;
}

DEVICE_DT_DEFINE(DT_NODELABEL(test_mbox), test_mbox_init, NULL,
		 NULL, NULL, POST_KERNEL, 90, NULL);
DEVICE_DT_DEFINE(DT_NODELABEL(test_mbox_zero_cell), test_mbox_init, NULL,
		 NULL, NULL, POST_KERNEL, 90, NULL);

ZTEST(devicetree_api, test_mbox)
{
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_adc_temp_sensor

	const struct mbox_channel channel_tx = MBOX_DT_CHANNEL_GET(TEST_TEMP, tx);
	const struct mbox_channel channel_rx = MBOX_DT_CHANNEL_GET(TEST_TEMP, rx);

	zassert_equal(channel_tx.id, 1, "");
	zassert_equal(channel_rx.id, 2, "");

	zassert_equal(DT_MBOX_CHANNEL_BY_NAME(TEST_TEMP, tx), 1, "");
	zassert_equal(DT_MBOX_CHANNEL_BY_NAME(TEST_TEMP, rx), 2, "");

	zassert_true(DT_SAME_NODE(DT_MBOX_CTLR_BY_NAME(TEST_TEMP, tx),
				  DT_NODELABEL(test_mbox)), "");
	zassert_true(DT_SAME_NODE(DT_MBOX_CTLR_BY_NAME(TEST_TEMP, rx),
				  DT_NODELABEL(test_mbox)), "");

	zassert_equal(DT_MBOX_CHANNEL_BY_NAME(TEST_TEMP, tx), 1, "");
	zassert_equal(DT_MBOX_CHANNEL_BY_NAME(TEST_TEMP, rx), 2, "");

	const struct mbox_channel channel_zero = MBOX_DT_CHANNEL_GET(TEST_TEMP, zero);

	zassert_equal(channel_zero.id, 0, "");

	zassert_equal(DT_MBOX_CHANNEL_BY_NAME(TEST_TEMP, zero), 0, "");

	zassert_true(DT_SAME_NODE(DT_MBOX_CTLR_BY_NAME(TEST_TEMP, zero),
				  DT_NODELABEL(test_mbox_zero_cell)), "");
}

ZTEST(devicetree_api, test_string_token)
{
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_string_token
	enum {
		token_zero,
		token_one,
		token_two,
		token_no_inst,
	};
	enum {
		TOKEN_ZERO = token_no_inst + 1,
		TOKEN_ONE,
		TOKEN_TWO,
		TOKEN_NO_INST,
	};
	int i;

	/* Test DT_INST_STRING_TOKEN */
#define STRING_TOKEN_TEST_INST_EXPANSION(inst) \
	DT_INST_STRING_TOKEN(inst, val),
	int array_inst[] = {
		DT_INST_FOREACH_STATUS_OKAY(STRING_TOKEN_TEST_INST_EXPANSION)
	};

	for (i = 0; i < ARRAY_SIZE(array_inst); i++) {
		zassert_between_inclusive(array_inst[i], token_zero, token_two, "");
	}

	/* Test DT_INST_STRING_UPPER_TOKEN */
#define STRING_UPPER_TOKEN_TEST_INST_EXPANSION(inst) \
	DT_INST_STRING_UPPER_TOKEN(inst, val),
	int array_inst_upper[] = {
		DT_INST_FOREACH_STATUS_OKAY(STRING_UPPER_TOKEN_TEST_INST_EXPANSION)
	};

	for (i = 0; i < ARRAY_SIZE(array_inst_upper); i++) {
		zassert_between_inclusive(array_inst_upper[i], TOKEN_ZERO, TOKEN_TWO, "");
	}

	/* Test DT_INST_STRING_TOKEN_OR when property is found */
#define STRING_TOKEN_OR_TEST_INST_EXPANSION(inst) \
	DT_INST_STRING_TOKEN_OR(inst, val, token_no_inst),
	int array_inst_or[] = {
		DT_INST_FOREACH_STATUS_OKAY(STRING_TOKEN_OR_TEST_INST_EXPANSION)
	};

	for (i = 0; i < ARRAY_SIZE(array_inst_or); i++) {
		zassert_between_inclusive(array_inst_or[i], token_zero, token_two, "");
	}

	/* Test DT_INST_STRING_UPPER_TOKEN_OR when property is found */
#define STRING_UPPER_TOKEN_OR_TEST_INST_EXPANSION(inst)	\
	DT_INST_STRING_UPPER_TOKEN_OR(inst, val, TOKEN_NO_INST),
	int array_inst_upper_or[] = {
		DT_INST_FOREACH_STATUS_OKAY(STRING_UPPER_TOKEN_OR_TEST_INST_EXPANSION)
	};

	for (i = 0; i < ARRAY_SIZE(array_inst_upper_or); i++) {
		zassert_between_inclusive(array_inst_upper_or[i], TOKEN_ZERO, TOKEN_TWO, "");
	}

	/* Test DT_STRING_TOKEN_OR when property is found */
	zassert_equal(DT_STRING_TOKEN_OR(DT_NODELABEL(test_string_token_0),
					 val, token_one), token_zero, "");
	zassert_equal(DT_STRING_TOKEN_OR(DT_NODELABEL(test_string_token_1),
					 val, token_two), token_one, "");

	/* Test DT_STRING_TOKEN_OR is not found */
	zassert_equal(DT_STRING_TOKEN_OR(DT_NODELABEL(test_string_token_1),
					 no_inst, token_zero), token_zero, "");

	/* Test DT_STRING_UPPER_TOKEN_OR when property is found */
	zassert_equal(DT_STRING_UPPER_TOKEN_OR(DT_NODELABEL(test_string_token_0),
					       val, TOKEN_ONE), TOKEN_ZERO, "");
	zassert_equal(DT_STRING_UPPER_TOKEN_OR(DT_NODELABEL(test_string_token_1),
					       val, TOKEN_TWO), TOKEN_ONE, "");

	/* Test DT_STRING_UPPER_TOKEN_OR is not found */
	zassert_equal(DT_STRING_UPPER_TOKEN_OR(DT_NODELABEL(test_string_token_1),
					       no_inst, TOKEN_ZERO), TOKEN_ZERO, "");

	/* Test DT_INST_STRING_TOKEN_OR when property is not found */
#define STRING_TOKEN_TEST_NO_INST_EXPANSION(inst) \
	DT_INST_STRING_TOKEN_OR(inst, no_inst, token_no_inst),
	int array_no_inst_or[] = {
		DT_INST_FOREACH_STATUS_OKAY(STRING_TOKEN_TEST_NO_INST_EXPANSION)
	};
	for (i = 0; i < ARRAY_SIZE(array_no_inst_or); i++) {
		zassert_equal(array_no_inst_or[i], token_no_inst, "");
	}

	/* Test DT_INST_STRING_UPPER_TOKEN_OR when property is not found */
#define STRING_UPPER_TOKEN_TEST_NO_INST_EXPANSION(inst)	\
	DT_INST_STRING_UPPER_TOKEN_OR(inst, no_inst, TOKEN_NO_INST),
	int array_no_inst_upper_or[] = {
		DT_INST_FOREACH_STATUS_OKAY(STRING_UPPER_TOKEN_TEST_NO_INST_EXPANSION)
	};
	for (i = 0; i < ARRAY_SIZE(array_no_inst_upper_or); i++) {
		zassert_equal(array_no_inst_upper_or[i], TOKEN_NO_INST, "");
	}
}

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT vnd_adc_temp_sensor
ZTEST(devicetree_api, test_reset)
{
	/* DT_RESET_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_RESET_CTLR_BY_IDX(TEST_TEMP, 1),
				  DT_NODELABEL(test_reset)), "");

	/* DT_RESET_CTLR */
	zassert_true(DT_SAME_NODE(DT_RESET_CTLR(TEST_TEMP),
				  DT_NODELABEL(test_reset)), "");

	/* DT_RESET_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_RESET_CTLR_BY_NAME(TEST_TEMP, reset_b),
				  DT_NODELABEL(test_reset)), "");

	/* DT_RESET_CELL_BY_IDX */
	zassert_equal(DT_RESET_CELL_BY_IDX(TEST_TEMP, 1, id), 20, "");
	zassert_equal(DT_RESET_CELL_BY_IDX(TEST_TEMP, 0, id), 10, "");

	/* DT_RESET_CELL_BY_NAME */
	zassert_equal(DT_RESET_CELL_BY_NAME(TEST_TEMP, reset_a, id), 10, "");
	zassert_equal(DT_RESET_CELL_BY_NAME(TEST_TEMP, reset_b, id), 20, "");

	/* DT_RESET_CELL */
	zassert_equal(DT_RESET_CELL(TEST_TEMP, id), 10, "");

	/* reg-width on reset */
	zassert_equal(DT_PROP_BY_PHANDLE_IDX(TEST_TEMP, resets, 1, reg_width), 4, "");

	/* DT_INST */
	zassert_equal(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT), 1, "");

	/* DT_INST_RESET_CTLR_BY_IDX */
	zassert_true(DT_SAME_NODE(DT_INST_RESET_CTLR_BY_IDX(0, 1),
				  DT_NODELABEL(test_reset)), "");

	/* DT_INST_RESET_CTLR */
	zassert_true(DT_SAME_NODE(DT_INST_RESET_CTLR(0),
				  DT_NODELABEL(test_reset)), "");

	/* DT_INST_RESET_CTLR_BY_NAME */
	zassert_true(DT_SAME_NODE(DT_INST_RESET_CTLR_BY_NAME(0, reset_b),
				  DT_NODELABEL(test_reset)), "");

	/* DT_INST_RESET_CELL_BY_IDX */
	zassert_equal(DT_INST_RESET_CELL_BY_IDX(0, 1, id), 20, "");
	zassert_equal(DT_INST_RESET_CELL_BY_IDX(0, 0, id), 10, "");

	/* DT_INST_RESET_CELL_BY_NAME */
	zassert_equal(DT_INST_RESET_CELL_BY_NAME(0, reset_a, id), 10, "");
	zassert_equal(DT_INST_RESET_CELL_BY_NAME(0, reset_b, id), 20, "");

	/* DT_INST_RESET_CELL */
	zassert_equal(DT_INST_RESET_CELL(0, id), 10, "");

	/* reg-width on reset */
	zassert_equal(DT_INST_PROP_BY_PHANDLE_IDX(0, resets, 1, reg_width), 4, "");

	/* DT_RESET_ID_BY_IDX */
	zassert_equal(DT_RESET_ID_BY_IDX(TEST_TEMP, 0), 10, "");
	zassert_equal(DT_RESET_ID_BY_IDX(TEST_TEMP, 1), 20, "");

	/* DT_RESET_ID */
	zassert_equal(DT_RESET_ID(TEST_TEMP), 10, "");

	/* DT_INST_RESET_ID_BY_IDX */
	zassert_equal(DT_INST_RESET_ID_BY_IDX(0, 0), 10, "");
	zassert_equal(DT_INST_RESET_ID_BY_IDX(0, 1), 20, "");

	/* DT_INST_RESET_ID */
	zassert_equal(DT_INST_RESET_ID(0), 10, "");
}

ZTEST_SUITE(devicetree_api, NULL, NULL, NULL, NULL, NULL);
