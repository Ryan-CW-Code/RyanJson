#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int count = 0;
static int use = 0;

void *v_malloc(size_t size)
{
    void *p;
    p = malloc(size ? size + sizeof(int) : 0);
    if (!p)
        return NULL;
    count++;
    *(int *)p = size;
    use += size;
    return (void *)((char *)p + sizeof(int));
}

void *v_calloc(size_t num, size_t size)
{
    void *p;
    p = v_malloc(num * size);
    if (!p)
        return NULL;
    memset(p, 0, num * size);
    return p;
}

void v_free(void *block)
{
    void *p;
    if (!block)
        return;
    p = (void *)((char *)block - sizeof(int));
    use -= *(int *)p;
    count--;
    free(p);
}

void *v_realloc(void *block, size_t size)
{
    void *p;
    int s = 0;
    if (block)
    {
        block = (void *)((char *)block - sizeof(int));
        s = *(int *)block;
    }
    p = realloc(block, size ? size + sizeof(int) : 0);
    if (!p)
        return NULL;
    if (!block)
        count++;
    *(int *)p = size;
    use += (size - s);
    return (void *)((char *)p + sizeof(int));
}

int v_mcheck(int *_count, int *_use)
{
    if (_count)
        *_count = count;
    if (_use)
        *_use = use;

    // if (count || use)
    // {
    // 	printf("|||----------->>> area = %d, size = %d\r\n", count, use);
    // 	return 1;
    // }

    return 0;
}