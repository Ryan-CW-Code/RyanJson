#include <stdint.h>

#include "qemuPlatform.h"

#define qemuScbCfsrReg      (*(volatile uint32_t *)0xE000ED28UL)
#define qemuScbHfsrReg      (*(volatile uint32_t *)0xE000ED2CUL)
#define qemuScbMmfarReg     (*(volatile uint32_t *)0xE000ED34UL)
#define qemuScbBfarReg      (*(volatile uint32_t *)0xE000ED38UL)
#define qemuSsramStart      (0x20000000UL)
#define qemuSsramEnd        (0x20400000UL)
#define qemuPsramStart      (0x21000000UL)
#define qemuPsramEnd        (0x21200000UL)
#define qemuStackFrameWords (8UL)

extern uint32_t _estack;

static bool qemuIsAddressInRange(uintptr_t address, uintptr_t rangeStart, uintptr_t rangeEnd)
{
	return (address >= rangeStart) && (address < rangeEnd);
}

static bool qemuIsValidStackedFrame(const uint32_t *stackPtr)
{
	const uintptr_t address = (uintptr_t)stackPtr;
	const uintptr_t frameEnd = address + (qemuStackFrameWords * sizeof(uint32_t));
	bool inSsram = false;
	bool inPsram = false;

	if (frameEnd < address) { return false; }

	inSsram = qemuIsAddressInRange(address, qemuSsramStart, qemuSsramEnd) && (frameEnd <= qemuSsramEnd);
	inPsram = qemuIsAddressInRange(address, qemuPsramStart, qemuPsramEnd) && (frameEnd <= qemuPsramEnd);

	return (NULL != stackPtr) && (0U == (address & 0x7UL)) && (inSsram || inPsram);
}

static void qemuDumpFaultContext(const char *tag, uint32_t *stackPtr, uint32_t excReturn)
{
	const uint32_t cfsr = qemuScbCfsrReg;
	const uint32_t hfsr = qemuScbHfsrReg;
	const uint32_t mmfar = qemuScbMmfarReg;
	const uint32_t bfar = qemuScbBfarReg;

	qemuLogf("[QEMU][%s] EXC_RETURN=0x%08lx\n", tag, (unsigned long)excReturn);
	qemuLogf("[QEMU][%s] CFSR=0x%08lx HFSR=0x%08lx BFAR=0x%08lx MMFAR=0x%08lx\n", tag, (unsigned long)cfsr, (unsigned long)hfsr,
		 (unsigned long)bfar, (unsigned long)mmfar);

	if (qemuIsValidStackedFrame(stackPtr))
	{
		qemuLogf("[QEMU][%s] stacked_r0=0x%08lx stacked_r1=0x%08lx stacked_r2=0x%08lx stacked_r3=0x%08lx\n", tag,
			 (unsigned long)stackPtr[0], (unsigned long)stackPtr[1], (unsigned long)stackPtr[2], (unsigned long)stackPtr[3]);
		qemuLogf("[QEMU][%s] stacked_r12=0x%08lx stacked_lr=0x%08lx stacked_pc=0x%08lx stacked_xpsr=0x%08lx\n", tag,
			 (unsigned long)stackPtr[4], (unsigned long)stackPtr[5], (unsigned long)stackPtr[6], (unsigned long)stackPtr[7]);
	}
	else
	{
		qemuLogf("[QEMU][%s] stacked_frame invalid: 0x%08lx\n", tag, (unsigned long)(uintptr_t)stackPtr);
	}

	if ((cfsr & (1UL << 24)) != 0U) { qemuLogf("[QEMU][%s] CFSR.UNALIGNED=1\n", tag); }

	if (qemuHardfaultWasExpected()) { qemuLogf("[QEMU][RESULT] EXPECTED_UNALIGNED_FAULT cfsr=0x%08lx\n", (unsigned long)cfsr); }
}

void qemuHardfaultHandlerC(uint32_t *stackPtr, uint32_t excReturn)
{
	bool expectedFault = qemuHardfaultWasExpected();
	qemuDumpFaultContext("HARDFAULT", stackPtr, excReturn);
	qemuSetExpectUnalignedFault(false);

	if (expectedFault) { qemuRequestExit(0); }

	qemuRequestExit(1);
}

__attribute__((naked)) void HardFault_Handler(void)
{
	__asm volatile("tst lr, #4                        \n"
		       "ite eq                            \n"
		       "mrseq r0, msp                     \n"
		       "mrsne r0, psp                     \n"
		       "mov r1, lr                        \n"
		       "ldr r2, =_estack                  \n"
		       "msr msp, r2                       \n"
		       "b qemuHardfaultHandlerC           \n");
}

void UsageFault_Handler(void)
{
	qemuDumpFaultContext("USAGEFAULT", NULL, 0U);
	qemuRequestExit(1);
}

void BusFault_Handler(void)
{
	qemuDumpFaultContext("BUSFAULT", NULL, 0U);
	qemuRequestExit(1);
}

void MemManage_Handler(void)
{
	qemuDumpFaultContext("MEMFAULT", NULL, 0U);
	qemuRequestExit(1);
}
