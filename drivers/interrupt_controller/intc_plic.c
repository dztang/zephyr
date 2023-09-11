/*
 * Copyright (c) 2017 Jean-Paul Etienne <fractalclone@gmail.com>
 * Contributors: 2018 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT sifive_plic_1_0_0

/**
 * @brief Platform Level Interrupt Controller (PLIC) driver
 *        for RISC-V processors
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/init.h>
#include <soc.h>

#include <zephyr/sw_isr_table.h>
#include <zephyr/drivers/interrupt_controller/riscv_plic.h>
#include <zephyr/irq.h>
#include <zephyr/sys/__assert.h>

#define PLIC_BASE_ADDR(n) DT_INST_REG_ADDR(n)
#define PLIC_REG_PRIO_OFFSET 0x0
#define PLIC_REG_TRIG_TYPE_OFFSET 0x1080
#define PLIC_REG_IRQ_EN_OFFSET 0x2000
#define PLIC_REG_REGS_OFFSET 0x200000

#define PLIC_EDGE_TRIG_SHIFT  5

struct plic_regs_t {
	uint32_t threshold_prio;
	uint32_t claim_complete;
};

typedef void (*riscv_plic_irq_config_func_t)(void);
struct plic_config {
	mem_addr_t prio;
	mem_addr_t irq_en;
	mem_addr_t reg;
	mem_addr_t trig;
	uint32_t max_prio;
	uint32_t num_irqs;
	uint32_t isr_offset;
	riscv_plic_irq_config_func_t irq_config_func;
};

static uint32_t save_irq;
static struct device *save_dev;

static inline uint32_t get_plic_enabled_size(const struct device *dev)
{
	const struct plic_config *config = dev != NULL ? dev->config : DEVICE_DT_INST_GET(0);

	return (config->num_irqs >> 5) + 1;
}

/**
 * @brief Get the ISR table's offset for the PLIC device
 *
 * @param dev PLIC device to get the offset
 *
 * @return ISR table's offset
 */
static inline uint32_t get_isr_table_offset(const struct device *dev)
{
	const struct plic_config *config = dev != NULL ? dev->config : DEVICE_DT_INST_GET(0);

	return config->isr_offset;
}

/**
 * @brief return edge irq value or zero
 *
 * In the event edge irq is enable this will return the trigger
 * value of the irq. In the event edge irq is not supported this
 * routine will return 0
 *
 * @param irq IRQ number to add to the trigger
 *
 * @return irq value when enabled 0 otherwise
 */
static int riscv_plic_is_edge_irq(uint32_t irq, const struct device *dev)
{
	const struct plic_config *config = dev != NULL ? dev->config : DEVICE_DT_INST_GET(0);

	if (config->trig != 0) {
		volatile uint32_t *trig = (volatile uint32_t *) config->trig;

		trig += (irq >> PLIC_EDGE_TRIG_SHIFT);
		return *trig & BIT(irq);
	}
	return 0;
}

void z_riscv_plic_isr_intc_install(unsigned int irq, unsigned int priority,
			      void (*routine)(const void *), const void *param,
			      const struct device *dev)
{
	__ASSERT(!irq_is_enabled(irq), "IRQ %d is enabled", irq);

	unsigned int table_idx = z_get_sw_isr_table_idx(irq) + get_isr_table_offset(dev);

	_sw_isr_table[table_idx].arg = param;
	_sw_isr_table[table_idx].isr = routine;

	if (irq_get_level(irq) == 2) {
		irq = irq_from_level_2(irq);

		riscv_plic_intc_set_priority(irq, priority, dev);
	}
}

void z_riscv_plic_isr_install(unsigned int irq, unsigned int priority,
			      void (*routine)(const void *), const void *param)
{
	z_riscv_plic_isr_intc_install(irq, priority, routine, param, NULL);
}

/**
 * @brief Enable a riscv PLIC-specific interrupt line
 *
 * This routine enables a RISCV PLIC-specific interrupt line.
 * riscv_plic_irq_enable is called by SOC_FAMILY_RISCV_PRIVILEGED
 * arch_irq_enable function to enable external interrupts for
 * IRQS level == 2, whenever CONFIG_RISCV_HAS_PLIC variable is set.
 *
 * @param irq IRQ number to enable
 */
void riscv_plic_irq_enable(uint32_t irq, const struct device *dev)
{
	const struct plic_config *config = dev != NULL ? dev->config : DEVICE_DT_INST_GET(0);
	volatile uint32_t *en = (volatile uint32_t *) config->irq_en;
	uint32_t key;

	key = irq_lock();
	en += (irq >> 5);
	*en |= (1 << (irq & 31));
	irq_unlock(key);
}

/**
 * @brief Disable a riscv PLIC-specific interrupt line
 *
 * This routine disables a RISCV PLIC-specific interrupt line.
 * riscv_plic_irq_disable is called by SOC_FAMILY_RISCV_PRIVILEGED
 * arch_irq_disable function to disable external interrupts, for
 * IRQS level == 2, whenever CONFIG_RISCV_HAS_PLIC variable is set.
 *
 * @param irq IRQ number to disable
 */
void riscv_plic_irq_disable(uint32_t irq, const struct device *dev)
{
	const struct plic_config *config = dev != NULL ? dev->config : DEVICE_DT_INST_GET(0);
	volatile uint32_t *en = (volatile uint32_t *) config->irq_en;
	uint32_t key;

	key = irq_lock();
	en += (irq >> 5);
	*en &= ~(1 << (irq & 31));
	irq_unlock(key);
}

/**
 * @brief Check if a riscv PLIC-specific interrupt line is enabled
 *
 * This routine checks if a RISCV PLIC-specific interrupt line is enabled.
 * @param irq IRQ number to check
 *
 * @return 1 or 0
 */
int riscv_plic_irq_is_enabled(uint32_t irq, const struct device *dev)
{
	const struct plic_config *config = dev != NULL ? dev->config : DEVICE_DT_INST_GET(0);
	volatile uint32_t *en = (volatile uint32_t *) config->irq_en;

	en += (irq >> 5);
	return !!(*en & (1 << (irq & 31)));
}

void riscv_plic_intc_set_priority(uint32_t irq, uint32_t priority, const struct device *dev)
{
	const struct plic_config *config = dev != NULL ? dev->config : DEVICE_DT_INST_GET(0);
	volatile uint32_t *prio = (volatile uint32_t *) config->prio;

	if (priority > config->max_prio)
		priority = config->max_prio;

	prio += irq;
	*prio = priority;
}

/**
 * @brief Set priority of a riscv PLIC-specific interrupt line
 *
 * This routine set the priority of a RISCV PLIC-specific interrupt line.
 * riscv_plic_irq_set_prio is called by riscv arch_irq_priority_set to set
 * the priority of an interrupt whenever CONFIG_RISCV_HAS_PLIC variable is set.
 *
 * @param irq IRQ number for which to set priority
 * @param priority Priority of IRQ to set to
 */
void riscv_plic_set_priority(uint32_t irq, uint32_t prio)
{
	riscv_plic_intc_set_priority(irq, prio, NULL);
}

/**
 * @brief Get riscv PLIC-specific interrupt line causing an interrupt
 *
 * This routine returns the RISCV PLIC-specific interrupt line causing an
 * interrupt.
 *
 * @return PLIC-specific interrupt line causing an interrupt.
 */
int riscv_plic_get_irq(void)
{
	return save_irq;
}
const struct device *riscv_plic_get_dev(void)
{
	return save_dev;
}

static void plic_irq_handler(const struct device *dev)
{
	const struct plic_config *config = dev->config;
	volatile struct plic_regs_t *regs = (volatile struct plic_regs_t *) config->reg;
	uint32_t irq;
	struct _isr_table_entry *ite;
	int edge_irq;

	/* Get the IRQ number generating the interrupt */
	irq = regs->claim_complete;

	/*
	 * Save IRQ in save_irq. To be used, if need be, by
	 * subsequent handlers registered in the _sw_isr_table table,
	 * as IRQ number held by the claim_complete register is
	 * cleared upon read.
	 */
	save_irq = irq;
	save_dev = (struct device *)dev;

	/*
	 * If the IRQ is out of range, call z_irq_spurious.
	 * A call to z_irq_spurious will not return.
	 */
	if (irq == 0U || irq >= config->num_irqs)
		z_irq_spurious(NULL);

	edge_irq = riscv_plic_is_edge_irq(irq, dev);

	/*
	 * For edge triggered interrupts, write to the claim_complete register
	 * to indicate to the PLIC controller that the IRQ has been handled
	 * for edge triggered interrupts.
	 */
	if (edge_irq)
		regs->claim_complete = save_irq;

	irq += CONFIG_2ND_LVL_ISR_TBL_OFFSET;

	/* Call the corresponding IRQ handler in _sw_isr_table */
	ite = (struct _isr_table_entry *)&_sw_isr_table[irq];
	ite->isr(ite->arg);

	/*
	 * Write to claim_complete register to indicate to
	 * PLIC controller that the IRQ has been handled
	 * for level triggered interrupts.
	 */
	if (!edge_irq)
		regs->claim_complete = save_irq;
}

/**
 * @brief Initialize the Platform Level Interrupt Controller
 *
 * @param dev PLIC device struct
 *
 * @retval 0 on success.
 */
static int plic_init(const struct device *dev)
{
	const struct plic_config *config = dev->config;
	volatile uint32_t *en = (volatile uint32_t *) config->irq_en;
	volatile uint32_t *prio = (volatile uint32_t *) config->prio;
	volatile struct plic_regs_t *regs = (volatile struct plic_regs_t *) config->reg;

	/* Ensure that all interrupts are disabled initially */
	for (uint32_t i = 0; i < get_plic_enabled_size(dev); i++) {
		*en = 0U;
		en++;
	}

	/* Set priority of each interrupt line to 0 initially */
	for (uint32_t i = 0; i < config->num_irqs; i++) {
		*prio = 0U;
		prio++;
	}

	/* Set threshold priority to 0 */
	regs->threshold_prio = 0U;

	/* Configure IRQ for PLIC driver */
	config->irq_config_func();

	return 0;
}

#define PLIC_INTC_IRQ_FUNC_DECLARE(n) static void plic_irq_config_func_##n(void)

#define PLIC_INTC_IRQ_FUNC_DEFINE(n)                                                               \
	static void plic_irq_config_func_##n(void)                                                 \
	{                                                                                          \
		IRQ_CONNECT(DT_INST_IRQN(n), 0, plic_irq_handler, DEVICE_DT_INST_GET(n), 0);       \
		irq_enable(DT_INST_IRQN(n));                                                       \
	}

#define PLIC_INTC_CONFIG_INIT(n)                                                                   \
	PLIC_INTC_IRQ_FUNC_DECLARE(n);                                                             \
	static const struct plic_config plic_config_##n = {                                        \
		.prio = PLIC_BASE_ADDR(n) + PLIC_REG_PRIO_OFFSET,                                  \
		.irq_en = PLIC_BASE_ADDR(n) + PLIC_REG_IRQ_EN_OFFSET,                              \
		.reg = PLIC_BASE_ADDR(n) + PLIC_REG_REGS_OFFSET,                                   \
		.max_prio = DT_INST_PROP(n, riscv_max_priority),                                   \
		.trig = PLIC_BASE_ADDR(n) + PLIC_REG_TRIG_TYPE_OFFSET,                             \
		.num_irqs = DT_INST_PROP(n, riscv_ndev),                                           \
		.isr_offset = DT_INST_PROP(n, isr_offset),                                         \
		.irq_config_func = plic_irq_config_func_##n,                                       \
	};                                                                                         \
	PLIC_INTC_IRQ_FUNC_DEFINE(n)

#define PLIC_INTC_DEVICE_INIT(n)                                                                   \
	PLIC_INTC_CONFIG_INIT(n)                                                                   \
	DEVICE_DT_INST_DEFINE(n, &plic_init, NULL,                                                 \
			      NULL, &plic_config_##n,                                              \
			      PRE_KERNEL_1, CONFIG_INTC_INIT_PRIORITY,                             \
			      NULL);

DT_INST_FOREACH_STATUS_OKAY(PLIC_INTC_DEVICE_INIT)
