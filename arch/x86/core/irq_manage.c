/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Interrupt support for IA-32 arch
 *
 * INTERNAL
 * The _idt_base_address symbol is used to determine the base address of the IDT.
 * (It is generated by the linker script, and doesn't correspond to an actual
 * global variable.)
 */

#include <kernel.h>
#include <arch/cpu.h>
#include <kernel_structs.h>
#include <misc/__assert.h>
#include <misc/printk.h>
#include <irq.h>
#include <tracing.h>
#include <kswap.h>
#include <arch/x86/segmentation.h>

extern void _SpuriousIntHandler(void *handler);
extern void _SpuriousIntNoErrCodeHandler(void *handler);

/*
 * Place the addresses of the spurious interrupt handlers into the intList
 * section. The genIdt tool can then populate any unused vectors with
 * these routines.
 */
void *__attribute__((section(".spurIsr"))) MK_ISR_NAME(_SpuriousIntHandler) =
	&_SpuriousIntHandler;
void *__attribute__((section(".spurNoErrIsr")))
	MK_ISR_NAME(_SpuriousIntNoErrCodeHandler) =
		&_SpuriousIntNoErrCodeHandler;

/* FIXME: IRQ direct inline functions have to be placed here and not in
 * arch/cpu.h as inline functions due to nasty circular dependency between
 * arch/cpu.h and kernel_structs.h; the inline functions typically need to
 * perform operations on _kernel.  For now, leave as regular functions, a
 * future iteration will resolve this.
 *
 * See https://github.com/zephyrproject-rtos/zephyr/issues/3056
 */

#ifdef CONFIG_SYS_POWER_MANAGEMENT
void _arch_irq_direct_pm(void)
{
	if (_kernel.idle) {
		s32_t idle_val = _kernel.idle;

		_kernel.idle = 0;
		_sys_power_save_idle_exit(idle_val);
	}
}
#endif

void _arch_isr_direct_header(void)
{
	_int_latency_start();
	z_sys_trace_isr_enter();

	/* We're not going to unlock IRQs, but we still need to increment this
	 * so that _is_in_isr() works
	 */
	++_kernel.nested;
}

void _arch_isr_direct_footer(int swap)
{
	_irq_controller_eoi();
	_int_latency_stop();
	sys_trace_isr_exit();
	--_kernel.nested;

	/* Call swap if all the following is true:
	 *
	 * 1) swap argument was enabled to this function
	 * 2) We are not in a nested interrupt
	 * 3) Next thread to run in the ready queue is not this thread
	 */
	if (swap && !_kernel.nested &&
	    _kernel.ready_q.cache != _current) {
		unsigned int flags;

		/* Fetch EFLAGS argument to _Swap() */
		__asm__ volatile (
			"pushfl\n\t"
			"popl %0\n\t"
			: "=g" (flags)
			:
			: "memory"
			);
		(void)_Swap(flags);
	}
}

#if defined(CONFIG_X86_DYNAMIC_IRQ_STUBS) && (CONFIG_X86_DYNAMIC_IRQ_STUBS > 0)

/*
 * z_interrupt_vectors_allocated[] bitfield is generated by the 'gen_idt' tool.
 * It is initialized to identify which interrupts have been statically
 * connected and which interrupts are available to be dynamically connected at
 * run time, with a 1 bit indicating a free vector.  The variable itself is
 * defined in the linker file.
 */
extern unsigned int z_interrupt_vectors_allocated[];

struct dyn_irq_info {
	/** IRQ handler */
	void (*handler)(void *param);
	/** Parameter to pass to the handler */
	void *param;
};

/*
 * Instead of creating a large sparse table mapping all possible IDT vectors
 * to dyn_irq_info, the dynamic stubs push a "stub id" onto the stack
 * which is used by common_dynamic_handler() to fetch the appropriate
 * information out of this much smaller table
 */
static struct dyn_irq_info dyn_irq_list[CONFIG_X86_DYNAMIC_IRQ_STUBS];
static unsigned int next_irq_stub;

/* Memory address pointing to where in ROM the code for the dynamic stubs are.
 * Linker symbol.
 */
extern char z_dynamic_stubs_begin[];

#ifndef CONFIG_X86_FIXED_IRQ_MAPPING
/**
 * @brief Allocate a free interrupt vector given <priority>
 *
 * This routine scans the z_interrupt_vectors_allocated[] array for a free vector
 * that satisfies the specified <priority>.
 *
 * This routine assumes that the relationship between interrupt priority and
 * interrupt vector is :
 *
 *      priority = (vector / 16) - 2;
 *
 * Vectors 0 to 31 are reserved for CPU exceptions and do NOT fall under
 * the priority scheme. The first vector used for priority level 0 will be 32.
 * Each interrupt priority level contains 16 vectors.
 *
 * It is also assumed that the interrupt controllers are capable of managing
 * interrupt requests on a per-vector level as opposed to a per-priority level.
 * For example, the local APIC on Pentium4 and later processors, the in-service
 * register (ISR) and the interrupt request register (IRR) are 256 bits wide.
 *
 * @return allocated interrupt vector
 */

static unsigned int priority_to_free_vector(unsigned int requested_priority)
{
	unsigned int entry;
	unsigned int fsb; /* first set bit in entry */
	unsigned int search_set;
	unsigned int vector_block;
	unsigned int vector;

	static unsigned int mask[2] = {0x0000ffff, 0xffff0000};

	vector_block = requested_priority + 2;

	__ASSERT(((vector_block << 4) + 15) <= CONFIG_IDT_NUM_VECTORS,
		 "IDT too small (%d entries) to use priority %d",
		 CONFIG_IDT_NUM_VECTORS, requested_priority);

	/*
	 * Atomically allocate a vector from the
	 * z_interrupt_vectors_allocated[] array to prevent race conditions
	 * with other threads attempting to allocate an interrupt
	 * vector.
	 *
	 * Note: As z_interrupt_vectors_allocated[] is initialized by the
	 * 'gen_idt.py' tool, it is critical that this routine use the same
	 * algorithm as the 'gen_idt.py' tool for allocating interrupt vectors.
	 */

	entry = vector_block >> 1;

	/*
	 * The z_interrupt_vectors_allocated[] entry indexed by 'entry'
	 * is a 32-bit quantity and thus represents the vectors for a pair of
	 * priority levels. Mask out the unwanted priority level and then use
	 * find_lsb_set() to scan for an available vector of the requested
	 * priority.
	 *
	 * Note that find_lsb_set() returns bit position from 1 to 32, or 0 if
	 * the argument is zero.
	 */
	search_set = mask[vector_block & 1] &
			z_interrupt_vectors_allocated[entry];
	fsb = find_lsb_set(search_set);

	__ASSERT(fsb != 0, "No remaning vectors for priority level %d",
		 requested_priority);

	/*
	 * An available vector of the requested priority was found.
	 * Mark it as allocated by clearing the bit.
	 */
	--fsb;
	z_interrupt_vectors_allocated[entry] &= ~(1 << fsb);

	/* compute vector given allocated bit within the priority level */
	vector = (entry << 5) + fsb;

	return vector;
}
#endif /* !CONFIG_X86_FIXED_IRQ_MAPPING */

/**
 * @brief Get the memory address of an unused dynamic IRQ or exception stub
 *
 * We generate at build time a set of dynamic stubs which push
 * a stub index onto the stack for use as an argument by
 * common handling code.
 *
 * @param stub_idx Stub number to fetch the corresponding stub function
 * @return Pointer to the stub code to install into the IDT
 */
static void *get_dynamic_stub(int stub_idx)
{
	u32_t offset;

	/*
	 * Because we want the sizes of the stubs to be consisent and minimized,
	 * stubs are grouped into blocks, each containing a push and subsequent
	 * 2-byte jump instruction to the end of the block, which then contains
	 * a larger jump instruction to common dynamic IRQ handling code
	 */
	offset = (stub_idx * Z_DYN_STUB_SIZE) +
		((stub_idx / Z_DYN_STUB_PER_BLOCK) *
		 Z_DYN_STUB_LONG_JMP_EXTRA_SIZE);

	return (void *)((u32_t)&z_dynamic_stubs_begin + offset);
}

extern const struct pseudo_descriptor z_x86_idt;

static void idt_vector_install(int vector, void *irq_handler)
{
	int key;

	key = irq_lock();
	_init_irq_gate(&z_x86_idt.entries[vector], CODE_SEG,
		       (u32_t)irq_handler, 0);
#ifdef CONFIG_MVIC
	/* MVIC requires IDT be reloaded if the entries table is ever changed */
	_set_idt(&z_x86_idt);
#endif
	irq_unlock(key);
}

/**
 *
 * @brief Connect a C routine to a hardware interrupt
 *
 * @param irq virtualized IRQ to connect to
 * @param priority requested priority of interrupt
 * @param routine the C interrupt handler
 * @param parameter parameter passed to C routine
 * @param flags IRQ flags
 *
 * This routine connects an interrupt service routine (ISR) coded in C to
 * the specified hardware <irq>.  An interrupt vector will be allocated to
 * satisfy the specified <priority>.
 *
 * The specified <irq> represents a virtualized IRQ, i.e. it does not
 * necessarily represent a specific IRQ line on a given interrupt controller
 * device.  The platform presents a virtualized set of IRQs from 0 to N, where
 * N is the total number of IRQs supported by all the interrupt controller
 * devices on the board.  See the platform's documentation for the mapping of
 * virtualized IRQ to physical IRQ.
 *
 * When the device asserts an interrupt on the specified <irq>, a switch to
 * the interrupt stack is performed (if not already executing on the interrupt
 * stack), followed by saving the integer (i.e. non-floating point) thread of
 * the currently executing thread or ISR.  The ISR specified by <routine>
 * will then be invoked with the single <parameter>.  When the ISR returns, a
 * context switch may occur.
 *
 * On some platforms <flags> parameter needs to be specified to indicate if
 * the irq is triggered by low or high level or by rising or falling edge.
 *
 * The routine searches for the first available element in the dynamic_stubs
 * array and uses it for the stub.
 *
 * @return the allocated interrupt vector
 *
 * WARNINGS
 * This routine does not perform range checking on the requested <priority>
 * and thus, depending on the underlying interrupt controller, may result
 * in the assignment of an interrupt vector located in the reserved range of
 * the processor.
 */

int _arch_irq_connect_dynamic(unsigned int irq, unsigned int priority,
		void (*routine)(void *parameter), void *parameter,
		u32_t flags)
{
	int vector, stub_idx, key;

	key = irq_lock();

#ifdef CONFIG_X86_FIXED_IRQ_MAPPING
	vector = _IRQ_TO_INTERRUPT_VECTOR(irq);
#else
	vector = priority_to_free_vector(priority);
	/* 0 indicates not used, vectors for interrupts start at 32 */
	__ASSERT(_irq_to_interrupt_vector[irq] == 0,
		 "IRQ %d already configured", irq);
	_irq_to_interrupt_vector[irq] = vector;
#endif
	_irq_controller_irq_config(vector, irq, flags);

	stub_idx = next_irq_stub++;
	__ASSERT(stub_idx < CONFIG_X86_DYNAMIC_IRQ_STUBS,
		 "No available interrupt stubs found");

	dyn_irq_list[stub_idx].handler = routine;
	dyn_irq_list[stub_idx].param = parameter;
	idt_vector_install(vector, get_dynamic_stub(stub_idx));

	irq_unlock(key);

	return vector;
}

/**
 * @brief Common dynamic IRQ handler function
 *
 * This gets called by the IRQ entry asm code with the stub index supplied as
 * an argument. Look up the required information in dyn_irq_list and
 * execute it.
 *
 * @param stub_idx Index into the dyn_irq_list array
 */
void z_x86_dynamic_irq_handler(u8_t stub_idx)
{
	dyn_irq_list[stub_idx].handler(dyn_irq_list[stub_idx].param);
}
#endif /* CONFIG_X86_DYNAMIC_IRQ_STUBS > 0 */
