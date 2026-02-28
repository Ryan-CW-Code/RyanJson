#include <stdint.h>

#include "qemuPlatform.h"

extern int main(void);
extern void __libc_init_array(void);

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

void _init(void)
{
}

void _fini(void)
{
}

void Reset_Handler(void);
void Default_Handler(void);

void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

extern void HardFault_Handler(void);

__attribute__((section(".isr_vector"))) const void *const gQemuVectors[] = {
	&_estack,
	Reset_Handler,
	NMI_Handler,
	HardFault_Handler,
	MemManage_Handler,
	BusFault_Handler,
	UsageFault_Handler,
	0,
	0,
	0,
	0,
	SVC_Handler,
	DebugMon_Handler,
	0,
	PendSV_Handler,
	SysTick_Handler //
};

static void qemuCopyDataAndBss(void)
{
	uint32_t *src = &_sidata;
	uint32_t *dst = &_sdata;

	while (dst < &_edata)
	{
		*dst++ = *src++;
	}

	dst = &_sbss;
	while (dst < &_ebss)
	{
		*dst++ = 0U;
	}
}

void Reset_Handler(void)
{
	volatile uint32_t *const scbVtor = (volatile uint32_t *)0xE000ED08UL;
	volatile uint32_t *const scbCcr = (volatile uint32_t *)0xE000ED14UL;
	volatile uint32_t *const scbShcsr = (volatile uint32_t *)0xE000ED24UL;

	qemuCopyDataAndBss();
	*scbVtor = (uint32_t)(uintptr_t)gQemuVectors;

	qemuPlatformInit();

	/* Route configurable faults to dedicated handlers for clearer diagnostics. */
	*scbShcsr |= ((1UL << 16) | (1UL << 17) | (1UL << 18));

	/* Force unaligned accesses to trap so QEMU can validate hardware semantics. */
	*scbCcr |= (1UL << 3);

	qemuLogf("[QEMU][BOOT] reset entered\n");
	qemuLogf("[QEMU][BOOT] SCB->CCR.UNALIGN_TRP=1\n");

	__libc_init_array();

	(void)main();

	qemuLogf("[QEMU][BOOT] main returned unexpectedly\n");
	qemuPanicLoop();
}

void Default_Handler(void)
{
	qemuLogf("[QEMU][FAULT] unexpected exception\n");
	qemuPanicLoop();
}
