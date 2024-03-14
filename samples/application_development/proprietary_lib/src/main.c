/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * Proprietary Library example:
 * calling functions from an out of tree static library
 */

#include <zephyr/kernel.h>
#include <stdio.h>

#include "math_ops.h"

int main(void)
{
	printk("\r\nProprietary library example!\r\n\n");

	test_vec_sum_int16_op();

	test_power_int16_op();

	test_power_int32_op();

	test_fft_op();

	test_iir_op();

	test_fir_blms_op();

	return 0;
}
