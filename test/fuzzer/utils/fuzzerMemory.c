#include "RyanJsonFuzzer.h"

// Fuzzer 全局状态
RyanJsonFuzzerState g_fuzzerState = {0};

/**
 * @brief Fuzzer 专用 malloc Hook
 *
 * 正常情况下直接透传系统分配；当故障注入开启时，按概率返回 NULL，
 * 用于命中库内部的 OOM 清理与回滚路径。
 */
void *RyanJsonFuzzerMalloc(size_t size)
{
	if (g_fuzzerState.isEnableMemFail && RyanJsonFuzzerShouldFail(598)) { return NULL; }
	return (char *)malloc(size);
}

void RyanJsonFuzzerFree(void *block)
{
	free(block);
}

/**
 * @brief Fuzzer 专用 realloc Hook
 *
 * 与 malloc Hook 保持同样的注入语义，重点覆盖扩容失败后的资源释放逻辑。
 */
void *RyanJsonFuzzerRealloc(void *block, size_t size)
{
	if (g_fuzzerState.isEnableMemFail && RyanJsonFuzzerShouldFail(508)) { return NULL; }
	return (char *)realloc(block, size);
}
