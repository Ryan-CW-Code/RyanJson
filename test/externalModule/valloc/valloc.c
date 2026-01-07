#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "valloc.h"

#define HEADER_SIZE        sizeof(int)
#define MALLOC_HEADER_SIZE 0

static int count = 0;
static int use = 0;

void *v_malloc(size_t size)
{
	if (size == 0) { return NULL; }

	void *p = malloc(size + HEADER_SIZE);
	if (!p) { return NULL; }

	*(int *)p = (int)size;

	count++;
	use += (int)size + MALLOC_HEADER_SIZE;

	return (char *)p + HEADER_SIZE;
}

void *v_calloc(size_t num, size_t size)
{
	size_t total = num * size;
	void *p = v_malloc(total);
	if (p) { memset(p, 0, total); }
	return p;
}

void v_free(void *block)
{
	if (!block) { return; }

	void *p = (char *)block - HEADER_SIZE;
	int size = *(int *)p;

	count--;
	use -= size + MALLOC_HEADER_SIZE;

	free(p);
}

void *v_realloc(void *block, size_t size)
{
	if (size == 0)
	{
		v_free(block);
		return NULL;
	}

	int old_size = 0;
	void *raw = NULL;

	if (block)
	{
		raw = (char *)block - HEADER_SIZE;
		old_size = *(int *)raw;
	}

	void *p = realloc(raw, size + HEADER_SIZE);
	if (!p) { return NULL; }

	*(int *)p = (int)size;

	if (!block) { count++; }
	use += (int)(size - old_size);

	return (char *)p + HEADER_SIZE;
}

int v_mcheck(int *dstCount, int *dstUse)
{
	if (dstCount) { *dstCount = count; }
	if (dstUse) { *dstUse = use; }
	return 0;
}

int32_t vallocGetArea(void)
{
	int32_t area2 = 0, use2 = 0;
	v_mcheck(&area2, &use2);
	return area2;
}

int32_t vallocGetUse(void)
{
	int32_t area2 = 0, use2 = 0;
	v_mcheck(&area2, &use2);
	return use2;
}

void displayMem(void)
{
	int32_t area2 = 0, use2 = 0;
	v_mcheck(&area2, &use2);
	printf("|||----------->>> area = %d, size = %d\r\n", area2, use2);
}
