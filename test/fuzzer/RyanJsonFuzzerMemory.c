#include "RyanJsonFuzzer.h"

static int32_t mallocCount = 0;
static int32_t reallocCount = 0;

void *RyanJsonFuzzerMalloc(size_t size)
{
	mallocCount++;
	if (RyanJsonTrue == isEnableRandomMemFail)
	{
		if (0 == mallocCount % 598) { return NULL; }
	}
	return (char *)v_malloc(size);
}

void RyanJsonFuzzerFree(void *block) { v_free(block); }

void *RyanJsonFuzzerRealloc(void *block, size_t size)
{
	reallocCount++;
	if (RyanJsonTrue == isEnableRandomMemFail)
	{
		if (0 == reallocCount % 508) { return NULL; }
	}
	return (char *)v_realloc(block, size);
}
