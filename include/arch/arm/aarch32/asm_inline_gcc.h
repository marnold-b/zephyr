/* ARM AArch32 GCC specific public inline assembler functions and macros */

/*
 * Copyright (c) 2015, Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Either public functions or macros or invoked by public functions */

#ifndef ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_ASM_INLINE_GCC_H_
#define ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_ASM_INLINE_GCC_H_

/*
 * The file must not be included directly
 * Include arch/cpu.h instead
 */

#ifndef _ASMLANGUAGE

#include <zephyr/types.h>
#include <arch/arm/aarch32/exc.h>
#include <irq.h>

#if defined(CONFIG_CPU_CORTEX_R) || defined(CONFIG_CPU_AARCH32_CORTEX_A)
#include <arch/arm/aarch32/cortex_a_r/cpu.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* On ARMv7-M and ARMv8-M Mainline CPUs, this function prevents regular
 * exceptions (i.e. with interrupt priority lower than or equal to
 * _EXC_IRQ_DEFAULT_PRIO) from interrupting the CPU. NMI, Faults, SVC,
 * and Zero Latency IRQs (if supported) may still interrupt the CPU.
 *
 * On ARMv6-M with enabled Zero Latency IRQs feature, this function prevents
 * regular exceptions from interrupting the CPU. NMI, Faults, SVC, pendSV
 * and Zero Latency IRQs (if supported) may still interrupt the CPU.
 *
 * On ARMv6-M and ARMv8-M Baseline CPUs, this function reads the value of
 * PRIMASK which shows if interrupts are enabled, then disables all interrupts
 * except NMI.
 */

static ALWAYS_INLINE unsigned int arch_irq_lock(void)
{
	unsigned int key;

#if defined(CONFIG_ZERO_LATENCY_IRQS_ARMV6_M)

	/* write old shadow register into key*/
	key = zli_get_shadow_reg();
	zli_set_shadow_reg(key | zli_get_irq_status());

	/* update interrupt register */
	zli_set_irq_status((key | zli_get_irq_status()) & zli_get_mask());
	zli_set_lock_flag(true);
	__asm__ volatile(
		"dsb;"
		"isb;");

#elif defined(CONFIG_ARMV6_M_ARMV8_M_BASELINE)
	__asm__ volatile("mrs %0, PRIMASK;"
		"cpsid i"
		: "=r" (key)
		:
		: "memory");
#elif defined(CONFIG_ARMV7_M_ARMV8_M_MAINLINE)
	unsigned int tmp;

	__asm__ volatile(
		"mov %1, %2;"
		"mrs %0, BASEPRI;"
		"msr BASEPRI_MAX, %1;"
		"isb;"
		: "=r"(key), "=r"(tmp)
		: "i"(_EXC_IRQ_DEFAULT_PRIO)
		: "memory");
#elif defined(CONFIG_ARMV7_R) || defined(CONFIG_ARMV7_A)
	__asm__ volatile(
		"mrs %0, cpsr;"
		"and %0, #" TOSTR(I_BIT) ";"
		"cpsid i;"
		: "=r" (key)
		:
		: "memory", "cc");
#else
#error Unknown ARM architecture
#endif /* CONFIG_ARMV6_M_ARMV8_M_BASELINE */

	return key;
}


static ALWAYS_INLINE void arch_irq_unlock(unsigned int key)
{
/* unlocking with the key == 0, unlocks all locks */
#if defined(CONFIG_ZERO_LATENCY_IRQS_ARMV6_M)

	if (key == 0 && zli_locked()) {
		/* update the irq's status with the value of the shadow register*/
		zli_set_irq_status(zli_get_shadow_reg());
		zli_set_shadow_reg(0);
		zli_set_lock_flag(false);
		__asm__ volatile(
			"dsb;"
			"isb;");
	}

#elif defined(CONFIG_ARMV6_M_ARMV8_M_BASELINE)
	if (key != 0U) {
		return;
	}
	__asm__ volatile(
		"cpsie i;"
		"isb"
		: : : "memory");
#elif defined(CONFIG_ARMV7_M_ARMV8_M_MAINLINE)
	__asm__ volatile(
		"msr BASEPRI, %0;"
		"isb;"
		:  : "r"(key) : "memory");
#elif defined(CONFIG_ARMV7_R) || defined(CONFIG_ARMV7_A)
	if (key != 0U) {
		return;
	}
	__asm__ volatile(
		"cpsie i;"
		: : : "memory", "cc");
#else
#error Unknown ARM architecture
#endif /* CONFIG_ZERO_LATENCY_IRQS_ARMV6_M */
}

static ALWAYS_INLINE bool arch_irq_unlocked(unsigned int key)
{
	/* This convention works for both PRIMASK and BASEPRI */
	return key == 0U;
}

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_ARM_AARCH32_ASM_INLINE_GCC_H_ */
