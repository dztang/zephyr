/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <string.h>
#include <sys/printk.h>
#include "sample_instance.h"
#include "sample_module.h"
#include "ext_log_system.h"
#include "ext_log_system_adapter.h"
#include <logging/log_ctrl.h>
#include <app_memory/app_memdomain.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(main);

#ifdef CONFIG_USERSPACE
K_APPMEM_PARTITION_DEFINE(app_part);
static struct k_mem_domain app_domain;
static struct k_mem_partition *app_parts[] = {
#ifdef Z_LIBC_PARTITION_EXISTS
		/* C library globals, stack canary storage, etc */
		&z_libc_partition,
#endif
		&app_part
};
#endif /* CONFIG_USERSPACE */

/* size of stack area used by each thread */
#define STACKSIZE (1024 + CONFIG_TEST_EXTRA_STACKSIZE)

extern void sample_module_func(void);

#define INST1_NAME STRINGIFY(SAMPLE_INSTANCE_NAME.inst1)
SAMPLE_INSTANCE_DEFINE(app_part, inst1);

#define INST2_NAME STRINGIFY(SAMPLE_INSTANCE_NAME.inst2)
SAMPLE_INSTANCE_DEFINE(app_part, inst2);

#if !defined(NRF_RTC1) && defined(CONFIG_SOC_FAMILY_NRF)
#include <soc.h>
#endif

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
	return sys_clock_hw_cycles_per_sec();
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

	for (int i = 0; i < log_sources_count(Z_LOG_LOCAL_DOMAIN_ID); i++) {
		const char *source_name;

		source_name = log_source_name_get(NULL, 0,
						  Z_LOG_LOCAL_DOMAIN_ID, i);
		if (strcmp(source_name, name) == 0) {
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

	sample_module_func();
	inline_func();

	if (IS_ENABLED(CONFIG_LOG_RUNTIME_FILTERING)) {
		printk("Disabling logging in the %s module\n",
					sample_module_name_get());

		log_filter_set(NULL, 0,
			       log_source_id_get(sample_module_name_get()),
			       LOG_LEVEL_NONE);

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

	sample_instance_inline_call(&inst1);
	sample_instance_call(&inst1);
	sample_instance_inline_call(&inst2);
	sample_instance_call(&inst2);

	if (IS_ENABLED(CONFIG_LOG_RUNTIME_FILTERING)) {
		printk("Changing filter to warning on %s instance.\n",
								INST1_NAME);

		log_filter_set(NULL, 0,
			       log_source_id_get(INST1_NAME), LOG_LEVEL_WRN);

		sample_instance_inline_call(&inst1);
		sample_instance_call(&inst1);
		sample_instance_inline_call(&inst2);
		sample_instance_call(&inst2);

		printk("Disabling logging on both instances.\n");

		log_filter_set(NULL, 0,
			       log_source_id_get(INST1_NAME),
			       LOG_LEVEL_NONE);

		log_filter_set(NULL, 0,
			       log_source_id_get(INST2_NAME),
			       LOG_LEVEL_NONE);

		sample_instance_inline_call(&inst1);
		sample_instance_call(&inst1);
		sample_instance_inline_call(&inst2);
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
 * @brief Function demonstrates how transient strings can be logged.
 *
 * Logger ensures that allocated buffers are freed when log message is
 * processed.
 */
static void log_strdup_showcase(void)
{
	char transient_str[] = "transient_string";

	printk("String logging showcase.\n");

	LOG_INF("Logging transient string:%s", log_strdup(transient_str));

	/* Overwrite transient string to show that the logger has a copy. */
	transient_str[0] = '\0';
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
	u32_t cnt = 0U;
	u32_t window = 2U;

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
		k_sleep(K_MSEC(5));
	}
}

static void log_demo_thread(void *p1, void *p2, void *p3)
{
	bool usermode = _is_user_context();

	k_sleep(K_MSEC(100));

	printk("\n\t---=< RUNNING LOGGER DEMO FROM %s THREAD >=---\n\n",
		(usermode) ? "USER" : "KERNEL");

	module_logging_showcase();

	instance_logging_showcase();

	/* Re-enabling filters before processing.
	 * Note: Same filters are used to for gathering logs and processing.
	 */
	log_filter_set(NULL, Z_LOG_LOCAL_DOMAIN_ID,
		       log_source_id_get(sample_module_name_get()),
		       CONFIG_LOG_DEFAULT_LEVEL);

	log_filter_set(NULL, Z_LOG_LOCAL_DOMAIN_ID,
		       log_source_id_get(INST1_NAME),
		       CONFIG_LOG_DEFAULT_LEVEL);

	log_filter_set(NULL, Z_LOG_LOCAL_DOMAIN_ID,
		       log_source_id_get(INST2_NAME),
		       CONFIG_LOG_DEFAULT_LEVEL);

	wait_on_log_flushed();

	log_strdup_showcase();

	severity_levels_showcase();

	wait_on_log_flushed();

	if (!usermode) {
		/*
		 * Logger performance in user mode cannot be demonstrated
		 * as precise timing API is accessible only from the kernel.
		 */
		performance_showcase();
		wait_on_log_flushed();

	}

	external_log_system_showcase();
	wait_on_log_flushed();
}

static void log_demo_supervisor(void *p1, void *p2, void *p3)
{
	/* Timestamp function could be set only from kernel thread. */
	(void)log_set_timestamp_func(timestamp_get, timestamp_freq());

	log_demo_thread(p1, p2, p3);

#ifdef CONFIG_USERSPACE
	k_mem_domain_init(&app_domain, ARRAY_SIZE(app_parts), app_parts);
	k_mem_domain_add_thread(&app_domain, k_current_get());
	k_thread_user_mode_enter(log_demo_thread, p1, p2, p3);
#endif
}

K_THREAD_DEFINE(log_demo_thread_id, STACKSIZE, log_demo_supervisor,
		NULL, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_MSEC(1));
