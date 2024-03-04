/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <kswap.h>
#include <ksched.h>
#include <ipi.h>

#ifdef CONFIG_TRACE_SCHED_IPI
extern void z_trace_sched_ipi(void);
#endif


void flag_ipi(uint32_t ipi_mask)
{
#if defined(CONFIG_SCHED_IPI_SUPPORTED)
	if (arch_num_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(struct k_thread *thread)
{
#if defined(CONFIG_IPI_OPTIMIZE)
	uint32_t  ipi_mask = 0;
	uint32_t  num_cpus = (uint32_t)arch_num_cpus();
	uint32_t  id = _current_cpu->id;
	struct k_thread *cpu_thread;

	for (uint32_t i = 0; i < num_cpus; i++) {
		if (id == i) {
			continue;
		}

		/*
		 * An IPI does not need to be sent to the target CPU if ...
		 * 1. the CPU is not active, or
		 * 2. the target CPU's active thread is not preemptible, or
		 * 3. <thread> can not execute on the target CPU, or
		 * 4. the target CPU's active thread has a higher priority
		 */

		cpu_thread = _kernel.cpus[i].current;
		if ((cpu_thread != NULL) &&
		    (thread_is_preemptible(cpu_thread)) &&
#if defined(CONFIG_SCHED_CPU_MASK)
		    ((thread->base.cpu_mask & BIT(i)) != 0) &&
#endif
		    (z_sched_prio_cmp(cpu_thread, thread) < 0)) {
			ipi_mask |= BIT(i);
		}
	}

	return (atomic_val_t)ipi_mask;
#elif (CONFIG_MP_MAX_NUM_CPUS > 1)
	ARG_UNUSED(thread);

	return IPI_ALL_CPUS_MASK;  /* Broadcast IPIs */
#else
	ARG_UNUSED(thread);

	return 0;                                  /* No IPIs needed */
#endif
}

void signal_pending_ipi(void)
{
	/* Synchronization note: you might think we need to lock these
	 * two steps, but an IPI is idempotent.  It's OK if we do it
	 * twice.  All we require is that if a CPU sees the flag true,
	 * it is guaranteed to send the IPI, and if a core sets
	 * pending_ipi, the IPI will be sent the next time through
	 * this code.
	 */
#if defined(CONFIG_SCHED_IPI_SUPPORTED)
	if (arch_num_cpus() > 1) {
		uint32_t  cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_clear(&_kernel.pending_ipi);
		if (cpu_bitmap != 0) {
			arch_sched_ipi(cpu_bitmap);
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

void z_sched_ipi(void)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#ifdef CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#endif /* CONFIG_TRACE_SCHED_IPI */

#ifdef CONFIG_TIMESLICING
	if (thread_is_sliceable(_current)) {
		z_time_slice();
	}
#endif /* CONFIG_TIMESLICING */
}
