#include "RyanJsonFuzzer.h"

// Defined in entry.c
RyanJsonFuzzerState g_fuzzerState = {0};

void *RyanJsonFuzzerMalloc(size_t size)
{
	if (g_fuzzerState.isEnableMemFail && RyanJsonFuzzerShouldFail(598)) { return NULL; }
	return (char *)malloc(size);
}

void RyanJsonFuzzerFree(void *block)
{
	free(block);
}

void *RyanJsonFuzzerRealloc(void *block, size_t size)
{
	if (g_fuzzerState.isEnableMemFail && RyanJsonFuzzerShouldFail(508)) { return NULL; }
	return (char *)realloc(block, size);
}
