/*
 * Copyright (c) 2015-2016 Intel Corporation
 * Copyright (c) 2017-2019 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "kernel.h"

#include "bs_types.h"
#include "bs_tracing.h"
#include "time_machine.h"
#include "bstests.h"

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr.h>
#include <misc/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <misc/byteorder.h>

#include <gatt/hrs.h>
#include <gatt/bas.h>

static struct bt_conn *default_conn;

/*
 * Basic connection test:
 *   We expect a central to connect to us.
 *
 *   The thread code is mostly a copy of the peripheral_hr sample device
 */

#define WAIT_TIME 5 /*seconds*/
extern enum bst_result_t bst_result;

static void test_con2_init(void)
{
	bst_ticker_set_next_tick_absolute(WAIT_TIME*1e6);
	bst_result = In_progress;
}

static void test_con2_tick(bs_time_t HW_device_time)
{
	/*
	 * If in WAIT_TIME seconds the testcase did not already pass
	 * (and finish) we consider it failed
	 */
	if (bst_result != Passed) {
		bst_result = Failed;
		bs_trace_error_line("test: connect2 failed (not finished "
				    "after %i seconds)\n", WAIT_TIME);
	}
}

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0d, 0x18, 0x0f, 0x18, 0x05, 0x18),
};

static void connected(struct bt_conn *conn, u8_t err)
{
	if (err) {
		printk("Connection failed (err %u)\n", err);
	} else {
		default_conn = bt_conn_ref(conn);
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, u8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);

	if (default_conn) {
		bt_conn_unref(default_conn);
		default_conn = NULL;
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	hrs_init(0x01);
	bas_init();

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}

static void test_con2_main(void)
{
	static int notify_count;
	int err;

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	bt_conn_cb_register(&conn_callbacks);

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(MSEC_PER_SEC);

		/* Heartrate measurements simulation */
		hrs_notify();

		/* Battery level simulation */
		bas_notify();

		if (notify_count++ == 2) { /* We consider it passed */
			bst_result = Passed;
			printk("Testcase passed\n");
		}
	}
}

static const struct bst_test_instance test_connect[] = {
	{
		.test_id = "peripheral",
		.test_descr = "Basic connection test. It expects that a "
			      "central device can be found. The test will "
			      "pass if notifications can be sent without "
			      "crash.",
		.test_post_init_f = test_con2_init,
		.test_tick_f = test_con2_tick,
		.test_main_f = test_con2_main
	},
	BSTEST_END_MARKER
};

struct bst_test_list *test_connect2_install(struct bst_test_list *tests)
{
	tests = bst_add_tests(tests, test_connect);
	return tests;
}
