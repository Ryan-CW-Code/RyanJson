#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief minify 模块的一次性确定性自检
 *
 * 这里只保留运行期 harness 不会主动构造的非法入参。
 */
void RyanJsonFuzzerSelfTestMinifyCases(void)
{
	// 运行期总是传入非负 size，这里只补齐负长度保护。
	{
		uint8_t rawBuf[5] = {'a', 'b', 'c', '#', '#'};
		assert(0 == RyanJsonMinify((char *)rawBuf, -10));
		assert('a' == rawBuf[0]);
		assert('b' == rawBuf[1]);
		assert('c' == rawBuf[2]);
		assert('#' == rawBuf[3]);
		assert('#' == rawBuf[4]);
	}

	assert(0 == RyanJsonMinify(NULL, 0));
	assert(0 == RyanJsonMinify(NULL, 10));
	assert(0 == RyanJsonMinify(NULL, -10));
}

/**
 * @brief 压缩测试
 *
 * 测试 RyanJson 的 Minify 功能（去除空白字符）。
 * 覆盖场景：
 * Minify 功能：将原始 Json 字符串压缩，验证是否能成功压缩。
 * 再次解析：解析压缩后的字符串，验证压缩未破坏 Json 结构。
 *
 * @param state Fuzzer 状态上下文
 * @param data 原始输入数据字符串
 * @param size 输入数据长度
 */
RyanJsonBool_e RyanJsonFuzzerTestMinify(const char *data, uint32_t size)
{
	// 准备缓冲区并拷贝数据
	// 分配比原始数据稍大的缓冲区，防止边界溢出
	char *buf = (char *)malloc((size_t)size + 1U);
	if (!buf) { return RyanJsonFalse; }

	memcpy(buf, data, size);
	buf[size] = '\0';

	// 执行 Minify
	// RyanJsonMinify 会原地修改缓冲区并移除空白字符
	uint32_t len = RyanJsonMinify(buf, (int32_t)size);
	assert(len > 0);
	assert(len <= size);

	// 验证 Minify 后数据有效性
	// 尝试解析压缩后的字符串，确认结构未被破坏
	// 注意：Minify 只是去空格，如果原串合法，Minify 后也应合法。
	// 这里使用可选尾部模式（不强制 Null Terminator，虽然已补终止符）
	RyanJson_t pJson = RyanJsonParseOptions(buf, len, size % 2 ? RyanJsonTrue : RyanJsonFalse, NULL);
	free(buf);
	if (NULL != pJson)
	{
		// 如果解析成功，尝试打印回来，确保对象结构完整
		uint32_t lenPrint = 0;
		char *jsonStr = RyanJsonPrint(pJson, 100, RyanJsonFalse, &lenPrint);

		RyanJsonCheckCode(NULL != jsonStr && lenPrint > 0, {
			RyanJsonDelete(pJson);
			return RyanJsonFalse;
		});

		RyanJsonFree(jsonStr);
		RyanJsonDelete(pJson);
	}
	else
	{
		// 如果输入本身非法，解析失败是预期行为
		return RyanJsonFalse;
	}

	return RyanJsonTrue;
}
