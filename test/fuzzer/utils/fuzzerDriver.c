#include "RyanJsonFuzzer.h"
#include <string.h>

#include "RyanJsonFuzzer.h"
#include <string.h>

/**
 * @brief 初始化 Fuzzer 状态
 *
 * 使用输入数据的前 4 个字节初始化 PRNG 种子，确保 Fuzzing 的确定性。
 *
 * @param state Fuzzer 状态指针
 * @param data 原始输入数据
 * @param size 输入数据大小
 */
void RyanJsonFuzzerInit(const uint8_t *data, size_t size)
{
	g_fuzzerState.data = data;
	g_fuzzerState.size = size;
	g_fuzzerState.pos = 0;
	g_fuzzerState.isEnableMemFail = true;

	if (0 == g_fuzzerState.seed) { g_fuzzerState.seed = time(NULL); }
	if (size >= 4)
	{
		uint32_t seed_input;
		memcpy(&seed_input, data, 4);
		g_fuzzerState.seed ^= seed_input;
	}
	else
	{
		for (size_t i = 0; i < size; i++)
		{
			g_fuzzerState.seed ^= ((uint32_t)data[i] << (i * 8));
		}
	}
}

/**
 * @brief 生成下一个伪随机数 (Xorshift32)
 *
 * 一个极其快速且轻量级的 PRNG，适合 Fuzzer 环境。
 *
 * @return uint32_t 随机数
 */
uint32_t RyanJsonFuzzerNextRand()
{
	uint32_t x = g_fuzzerState.seed;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	g_fuzzerState.seed = x;
	return x;
}

/**
 * @brief 决定是否应该触发失败
 *
 * 用于模拟内存分配失败等随机错误路径。
 *
 * @param probability 概率倒数 (例如 100 表示 1/100 的概率)
 * @return RyanJsonBool_e RyanJsonTrue 表示应该触发失败
 */
RyanJsonBool_e RyanJsonFuzzerShouldFail(uint32_t probability)
{
	if (false == g_fuzzerState.isEnableMemFail) { return RyanJsonFalse; }
	if (0 == probability) { return RyanJsonTrue; }
	return (0 == RyanJsonFuzzerNextRand() % probability) ? RyanJsonTrue : RyanJsonFalse;
}
