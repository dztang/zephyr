/*
 * Copyright (c) 2023 Advanced Micro Devices, Inc. (AMD)
 * Copyright (c) 2023 Alp Sayin <alpsayin@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */


/* Memory bits manipulation functions in non-arch-specific C code */

#ifndef ZEPHYR_INCLUDE_ARCH_MICROBLAZE_SYS_BITOPS_H_
#define ZEPHYR_INCLUDE_ARCH_MICROBLAZE_SYS_BITOPS_H_

#ifndef _ASMLANGUAGE

#include <zephyr/sys/sys_io.h>
#include <zephyr/toolchain.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

static ALWAYS_INLINE void sys_set_bit(mem_addr_t addr, unsigned int bit)
{
	compiler_barrier();
	uint32_t temp = *(volatile uint32_t *)addr;
	*(volatile uint32_t *)addr = temp | (1 << bit);
	compiler_barrier();
}

static ALWAYS_INLINE void sys_clear_bit(mem_addr_t addr, unsigned int bit)
{
	compiler_barrier();
	uint32_t temp = *(volatile uint32_t *)addr;
	*(volatile uint32_t *)addr = temp & ~(1 << bit);
	compiler_barrier();
}

static ALWAYS_INLINE int sys_test_bit(mem_addr_t addr, unsigned int bit)
{
	uint32_t temp;

	compiler_barrier();
	temp = *(volatile uint32_t *)addr;
	compiler_barrier();

	return temp & (1 << bit);
}

static ALWAYS_INLINE void sys_set_bits(mem_addr_t addr, unsigned int mask)
{
	compiler_barrier();
	uint32_t temp = *(volatile uint32_t *)addr;
	*(volatile uint32_t *)addr = temp | mask;
	compiler_barrier();
}

static ALWAYS_INLINE void sys_clear_bits(mem_addr_t addr, unsigned int mask)
{
	compiler_barrier();
	uint32_t temp = *(volatile uint32_t *)addr;
	*(volatile uint32_t *)addr = temp & ~mask;
	compiler_barrier();
}

static ALWAYS_INLINE void sys_bitfield_set_bit(mem_addr_t addr, unsigned int bit)
{
	/* Doing memory offsets in terms of 32-bit values to prevent
	 * alignment issues
	 */
	sys_set_bit(addr + ((bit >> 5) << 2), bit & 0x1F);
}

static ALWAYS_INLINE void sys_bitfield_clear_bit(mem_addr_t addr, unsigned int bit)
{
	sys_clear_bit(addr + ((bit >> 5) << 2), bit & 0x1F);
}

static ALWAYS_INLINE int sys_bitfield_test_bit(mem_addr_t addr, unsigned int bit)
{
	return sys_test_bit(addr + ((bit >> 5) << 2), bit & 0x1F);
}

static ALWAYS_INLINE int sys_test_and_set_bit(mem_addr_t addr, unsigned int bit)
{
	int ret;

	ret = sys_test_bit(addr, bit);
	sys_set_bit(addr, bit);

	return ret;
}

static ALWAYS_INLINE int sys_test_and_clear_bit(mem_addr_t addr, unsigned int bit)
{
	int ret;

	ret = sys_test_bit(addr, bit);
	sys_clear_bit(addr, bit);

	return ret;
}

static ALWAYS_INLINE int sys_bitfield_test_and_set_bit(mem_addr_t addr, unsigned int bit)
{
	int ret;

	ret = sys_bitfield_test_bit(addr, bit);
	sys_bitfield_set_bit(addr, bit);

	return ret;
}

static ALWAYS_INLINE int sys_bitfield_test_and_clear_bit(mem_addr_t addr, unsigned int bit)
{
	int ret;

	ret = sys_bitfield_test_bit(addr, bit);
	sys_bitfield_clear_bit(addr, bit);

	return ret;
}

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_MICROBLAZE_SYS_BITOPS_H_ */
