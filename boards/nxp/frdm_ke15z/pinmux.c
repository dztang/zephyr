/*
 * Copyright (c) 2019 Vestas Wind Systems A/S
 * Copyright (c) 2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/drivers/pinctrl.h>

static int frdm_ke15z_pinmux_init(void)
{

	int err; /* Used by pinctrl functions */

	/* Declare pin configuration state for flexio pin here */
	PINCTRL_DT_DEFINE(DT_NODELABEL(flexio));

	/* Apply pinctrl state directly, since there is no flexio device driver */
	err = pinctrl_apply_state(PINCTRL_DT_DEV_CONFIG_GET(DT_NODELABEL(flexio)),
		PINCTRL_STATE_DEFAULT);
	if (err) {
		return err;
	}

	return 0;
}

SYS_INIT(frdm_ke15z_pinmux_init, PRE_KERNEL_1, 0);
