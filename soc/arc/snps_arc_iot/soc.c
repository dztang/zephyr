
/*
 * Copyright (c) 2018 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * This module provides routines to initialize and support board-level hardware
 * for the IoT Development Kit board.
 *
 */
#include <device.h>
#include <init.h>
#include "sysconf.h"

static int arc_iot_init(struct device *dev)
{
	ARG_UNUSED(dev);

	if (arc_iot_pll_fout_config(
			CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC/1000000) < 0) {
		return -1;
	}

	return 0;
}

SYS_INIT(arc_iot_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
