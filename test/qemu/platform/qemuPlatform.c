#include "qemuPlatform.h"

#include <stdio.h>

#define qemuUart0Base          (0x40004000UL)
#define qemuUartDataReg        (*(volatile uint32_t *)(qemuUart0Base + 0x000U))
#define qemuUartStateReg       (*(volatile uint32_t *)(qemuUart0Base + 0x004U))
#define qemuUartCtrlReg        (*(volatile uint32_t *)(qemuUart0Base + 0x008U))
#define qemuUartBauddivReg     (*(volatile uint32_t *)(qemuUart0Base + 0x010U))
#define qemuUartStateTxFull    (1UL << 0)
#define qemuUartCtrlTxEnable   (1UL << 0)
#define qemuUartDefaultBauddiv (16UL)
#define qemuUartDrainSpinCount (200000UL)

static volatile uint32_t gExpectUnalignedFault = 0U;
extern void _exit(int status);

void qemuPlatformInit(void)
{
	/* CMSDK UART: enable TX and keep a conservative divider for deterministic output. */
	qemuUartBauddivReg = qemuUartDefaultBauddiv;
	qemuUartCtrlReg = qemuUartCtrlTxEnable;
}

void qemuUartPutc(char ch)
{
	for (uint32_t spin = 0U; spin < 1000000U; ++spin)
	{
		if ((qemuUartStateReg & qemuUartStateTxFull) == 0U) { break; }
	}

	qemuUartDataReg = (uint32_t)(uint8_t)ch;
}

void qemuUartWrite(const char *str)
{
	if (NULL == str) { return; }

	while ('\0' != *str)
	{
		if ('\n' == *str) { qemuUartPutc('\r'); }
		qemuUartPutc(*str);
		++str;
	}
}

void qemuWaitUartDrain(void)
{
	for (volatile uint32_t spin = 0U; spin < qemuUartDrainSpinCount; ++spin)
	{
		__asm volatile("nop");
	}
}

void qemuVlogf(const char *fmt, va_list args)
{
	char buffer[256];

	if (NULL == fmt) { return; }

	(void)vsnprintf(buffer, sizeof(buffer), fmt, args);
	qemuUartWrite(buffer);
}

void qemuLogf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	qemuVlogf(fmt, args);
	va_end(args);
}

void qemuAssertFailed(const char *file, int line)
{
	qemuLogf("[QEMU][ASSERT] %s:%d\n", (NULL != file) ? file : "<null>", line);
	qemuRequestExit(1);
}

void qemuRequestExit(int32_t status)
{
	qemuWaitUartDrain();
	_exit((int)status);
	qemuPanicLoop();
}

void qemuPanicLoop(void)
{
	for (;;)
	{
		__asm volatile("wfi");
	}
}

bool qemuHardfaultWasExpected(void)
{
	return (0U != gExpectUnalignedFault);
}

void qemuSetExpectUnalignedFault(bool expect)
{
	gExpectUnalignedFault = expect ? 1U : 0U;
}
