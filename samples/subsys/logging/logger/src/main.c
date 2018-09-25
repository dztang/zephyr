/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <string.h>
#include <misc/printk.h>
#include <logging/log_ctrl.h>
#include <soc.h>
#include "sample_instance.h"
#include "sample_module.h"
#include "ext_log_system.h"
#include "ext_log_system_adapter.h"

#include <logging/log.h>

LOG_MODULE_REGISTER(main);

/* size of stack area used by each thread */
#define STACKSIZE 1024

extern void sample_module_func(void);

#define INST1_NAME STRINGIFY(SAMPLE_INSTANCE_NAME.inst1)
SAMPLE_INSTANCE_DEFINE(inst1);

#define INST2_NAME STRINGIFY(SAMPLE_INSTANCE_NAME.inst2)
SAMPLE_INSTANCE_DEFINE(inst2);

static u32_t timestamp_get(void)
{
#ifdef CONFIG_SOC_FAMILY_NRF
	return NRF_RTC1->COUNTER;
#else
	return k_cycle_get_32();
#endif
}

static u32_t timestamp_freq(void)
{
#ifdef CONFIG_SOC_FAMILY_NRF
	return 32768 / (NRF_RTC1->PRESCALER + 1);
#else
	return CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;
#endif
}

/**
 * @brief Function for finding source ID based on source name.
 *
 * @param name Source name
 *
 * @return Source ID.
 */
static int log_source_id_get(const char *name)
{

	for (int i = 0; i < log_src_cnt_get(CONFIG_LOG_DOMAIN_ID); i++) {
		if (strcmp(log_source_name_get(CONFIG_LOG_DOMAIN_ID, i), name)
		    == 0) {
			return i;
		}
	}
	return -1;
}

/**
 * @brief Function demonstrates module level filtering.
 *
 * Sample module API is called then logging for this module is disabled and
 * function is called again. It is expected that only logs generated by the
 * first call will be processed by the output.
 */
static void module_logging_showcase(void)
{
	printk("Module logging showcase.\n");

	sample_module_inline_func();
	sample_module_func();

	if (IS_ENABLED(CONFIG_LOG_RUNTIME_FILTERING)) {
		printk("Disabling logging in the %s module\n",
					sample_module_name_get());

		log_filter_set(NULL, 0,
			       log_source_id_get(sample_module_name_get()),
			       LOG_LEVEL_NONE);

		sample_module_inline_func();
		sample_module_func();

		printk("Function called again but with logging disabled.\n");

	} else {
		printk("%s option disabled.\n",
		       STRINGIFY(CONFIG_LOG_RUNTIME_FILTERING));
	}
}

/**
 * @brief Function demonstrates instance level filtering.
 *
 * Sample multi-instance module API on two instances is called then logging
 * level for one instance is reduced and function is called again on two
 * instances. It is expected that one instance will generate less logs.
 */
static void instance_logging_showcase(void)
{
	printk("Instance level logging showcase.\n");

	sample_instance_call(&inst1);
	sample_instance_call(&inst2);

	if (IS_ENABLED(CONFIG_LOG_RUNTIME_FILTERING)) {
		printk("Changing filter to warning on %s instance.\n",
								INST1_NAME);

		log_filter_set(NULL, 0,
			       log_source_id_get(INST1_NAME), LOG_LEVEL_WRN);

		sample_instance_call(&inst1);
		sample_instance_call(&inst2);

		printk("Disabling logging on both instances.\n");

		log_filter_set(NULL, 0,
			       log_source_id_get(INST1_NAME),
			       LOG_LEVEL_NONE);

		log_filter_set(NULL, 0,
			       log_source_id_get(INST2_NAME),
			       LOG_LEVEL_NONE);

		sample_instance_call(&inst1);
		sample_instance_call(&inst2);

		printk("Function call on both instances with logging disabled.\n");
	}
}

/**
 * @brief Function demonstrates supported severity logging level.
 */
static void severity_levels_showcase(void)
{
	printk("Severity levels showcase.\n");

	LOG_ERR("Error message example.");
	LOG_WRN("Warning message example.");
	LOG_INF("Info message example.");
	LOG_DBG("Debug message example.");
}

/**
 * @brief Function demonstrates how fast data can be logged.
 *
 * Messages are logged and counted in a loop for 2 ticks (same clock source as
 * the one used for logging timestamp). Based on that and known clock frequency,
 * logging bandwidth is calculated.
 */
static void performance_showcase(void)
{
	volatile u32_t current_timestamp;
	volatile u32_t start_timestamp;
	u32_t per_sec;
	u32_t cnt = 0;
	u32_t window = 2;

	printk("Logging performance showcase.\n");

	start_timestamp = timestamp_get();

	while (start_timestamp == timestamp_get()) {
#if (CONFIG_ARCH_POSIX)
		k_busy_wait(100);
#endif
	}

	start_timestamp = timestamp_get();

	do {
		LOG_INF("performance test - log message %d", cnt);
		cnt++;
		current_timestamp = timestamp_get();
#if (CONFIG_ARCH_POSIX)
		k_busy_wait(100);
#endif
	} while (current_timestamp < (start_timestamp + window));

	per_sec = (cnt * timestamp_freq()) / window;
	printk("Estimated logging capabilities: %d messages/second\n", per_sec);
}

static void external_log_system_showcase(void)
{
	printk("Logs from external logging system showcase.\n");

	ext_log_system_log_adapt();

	ext_log_system_foo();
}

static void wait_on_log_flushed(void)
{
	while (log_buffered_cnt()) {
		k_sleep(5);
	}
}

void log_demo_thread(void *dummy1, void *dummy2, void *dummy3)
{
	k_sleep(100);

	(void)log_set_timestamp_func(timestamp_get, timestamp_freq());

	module_logging_showcase();

	instance_logging_showcase();

	/* Re-enabling filters before processing.
	 * Note: Same filters are used to for gathering logs and processing.
	 */
	log_filter_set(NULL, CONFIG_LOG_DOMAIN_ID,
		       log_source_id_get(sample_module_name_get()),
		       CONFIG_LOG_DEFAULT_LEVEL);

	log_filter_set(NULL, CONFIG_LOG_DOMAIN_ID,
		       log_source_id_get(INST1_NAME),
		       CONFIG_LOG_DEFAULT_LEVEL);

	log_filter_set(NULL, CONFIG_LOG_DOMAIN_ID,
		       log_source_id_get(INST2_NAME),
		       CONFIG_LOG_DEFAULT_LEVEL);

	wait_on_log_flushed();

	severity_levels_showcase();

	wait_on_log_flushed();

	performance_showcase();

	wait_on_log_flushed();

	external_log_system_showcase();

	wait_on_log_flushed();
}

K_THREAD_DEFINE(log_demo_thread_id, STACKSIZE, log_demo_thread,
		NULL, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, 1);
