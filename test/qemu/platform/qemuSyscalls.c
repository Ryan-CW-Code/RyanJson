#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "qemuPlatform.h"

extern uint8_t _end;
extern uint8_t _estack;

#define qemuSemihostingOpSysExitExtended (0x20U)
#define qemuSemihostingReasonAppExit     (0x20026U)

typedef struct
{
	uint32_t reason;
	uint32_t subCode;
} qemuSemihostingExitArgs_t;

static void qemuSemihostingExit(int status)
{
	qemuSemihostingExitArgs_t exitArgs = {qemuSemihostingReasonAppExit, (uint32_t)status};
#if defined(__arm__) || defined(__thumb__)
	register uint32_t opReg __asm("r0") = qemuSemihostingOpSysExitExtended;
	register qemuSemihostingExitArgs_t *argReg __asm("r1") = &exitArgs;
	__asm volatile("bkpt 0xAB" : : "r"(opReg), "r"(argReg) : "memory");
#else
	(void)exitArgs;
#endif
}

caddr_t _sbrk(int incr)
{
	static uint8_t *heap = &_end;
	uint8_t *const prevHeap = heap;
	uint8_t *const nextHeap = heap + incr;

	if (nextHeap >= &_estack)
	{
		errno = ENOMEM;
		return (caddr_t)-1;
	}

	heap = nextHeap;
	return (caddr_t)prevHeap;
}

int _write(int fd, const void *buf, size_t count)
{
	const char *ptr = (const char *)buf;

	if ((1 != fd && 2 != fd) || NULL == ptr)
	{
		errno = EBADF;
		return -1;
	}

	for (size_t i = 0U; i < count; ++i)
	{
		qemuUartPutc(ptr[i]);
	}

	return (int)count;
}

int _read(int fd, void *buf, size_t count)
{
	(void)fd;
	(void)buf;
	(void)count;
	errno = ENOSYS;
	return -1;
}

int _close(int fd)
{
	(void)fd;
	return 0;
}

int _fstat(int fd, struct stat *st)
{
	(void)fd;

	if (NULL == st)
	{
		errno = EFAULT;
		return -1;
	}

	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int fd)
{
	return (1 == fd || 2 == fd) ? 1 : 0;
}

off_t _lseek(int fd, off_t offset, int whence)
{
	(void)fd;
	(void)offset;
	(void)whence;
	return 0;
}

int _kill(int pid, int sig)
{
	(void)pid;
	(void)sig;
	errno = EINVAL;
	return -1;
}

int _getpid(void)
{
	return 1;
}

void _exit(int status)
{
	qemuLogf("[QEMU][EXIT] status=%d\n", status);
	qemuWaitUartDrain();
	qemuSemihostingExit(status);
	qemuPanicLoop();
}
