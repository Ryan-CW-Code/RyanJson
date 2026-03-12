#include "RyanJsonFuzzer.h"

/**
 * @brief 从当前输入稳定生成 PRNG 种子
 *
 * 这里不追求密码学强度，只要求同一份输入总能得到同一条随机路径，
 * 方便 libFuzzer 复现问题并保持覆盖结果稳定。
 */
static uint32_t RyanJsonFuzzerSeedFromInput(const uint8_t *data, size_t size)
{
	uint32_t seed = 2166136261u;

	for (size_t i = 0; i < size; ++i)
	{
		seed ^= (uint32_t)data[i];
		seed *= 16777619u;
	}

	seed ^= (uint32_t)size;
	return 0 == seed ? 0x811C9DC5u : seed;
}

/**
 * @brief 初始化 Fuzzer 状态
 *
 * 使用当前输入数据混合 PRNG 种子，保证同一输入具备稳定路径。
 *
 * @param data 原始输入数据
 * @param size 输入数据大小
 */
void RyanJsonFuzzerInit(const uint8_t *data, size_t size)
{
	g_fuzzerState.isEnableMemFail = true;
	g_fuzzerState.seed = RyanJsonFuzzerSeedFromInput(data, size);
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
