#ifndef ryanJsonQemuPlatformH
#define ryanJsonQemuPlatformH

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void qemuPlatformInit(void);
void qemuUartPutc(char ch);
void qemuUartWrite(const char *str);
void qemuWaitUartDrain(void);
void qemuLogf(const char *fmt, ...);
void qemuVlogf(const char *fmt, va_list args);
void qemuAssertFailed(const char *file, int line);
void qemuRequestExit(int32_t status) __attribute__((noreturn));
void qemuPanicLoop(void) __attribute__((noreturn));

bool qemuHardfaultWasExpected(void);
void qemuSetExpectUnalignedFault(bool expect);

#define qemuAssert(expr)                                                                                                                   \
	do                                                                                                                                 \
	{                                                                                                                                  \
		if (!(expr)) { qemuAssertFailed(__FILE__, __LINE__); }                                                                     \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif
