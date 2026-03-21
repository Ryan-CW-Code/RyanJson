#include "testBase.h"

static void testEdgeParsePrintPrintFormatFlag(void)
{
	// 复杂链路：
	// Parse -> Print(format=true/false) -> 输出差异校验。
	// 目标：验证格式化开关影响输出形态。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":{\"c\":2,\"d\":3}}");
	TEST_ASSERT_NOT_NULL(root);

	char *pretty = RyanJsonPrint(root, 64, RyanJsonTrue, NULL);
	char *minified = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(pretty);
	TEST_ASSERT_NOT_NULL(minified);
	TEST_ASSERT_NOT_NULL(strchr(pretty, '\n'));
	TEST_ASSERT_NULL(strchr(minified, '\n'));

	RyanJsonFree(minified);
	RyanJsonFree(pretty);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintPresetSmall(void)
{
	// 复杂链路：
	// Parse -> Print(preset=1) -> Parse -> Compare。
	// 目标：验证小预设缓冲也可正常扩容。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":[1,2,3]}");
	TEST_ASSERT_NOT_NULL(root);

	char *printed = RyanJsonPrint(root, 1, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintPreallocatedExactFitFromPrint(void)
{
	// 复杂链路：
	// Print -> 获取长度 -> PrintPreallocated(足够容量) -> 输出一致性。
	// 目标：验证预分配打印在安全容量下输出一致。
	RyanJson_t root = RyanJsonParse("{\"k\":\"v\",\"arr\":[1,2]}");
	TEST_ASSERT_NOT_NULL(root);

	uint32_t len = 0;
	char *printed = RyanJsonPrint(root, 0, RyanJsonFalse, &len);
	TEST_ASSERT_NOT_NULL(printed);

	char buf[512];
	TEST_ASSERT_TRUE(len + 1U <= sizeof(buf));
	char *out = RyanJsonPrintPreallocated(root, buf, (uint32_t)sizeof(buf), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING(printed, out);

	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintPreallocatedTooSmallThenSuccess(void)
{
	// 复杂链路：
	// Print(获取长度) -> PrintPreallocated(失败) -> PrintPreallocated(成功)。
	// 目标：验证不足容量失败路径可恢复。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":{\"c\":2}}");
	TEST_ASSERT_NOT_NULL(root);

	uint32_t len = 0;
	char *printed = RyanJsonPrint(root, 0, RyanJsonFalse, &len);
	TEST_ASSERT_NOT_NULL(printed);

	char tiny[8];
	TEST_ASSERT_TRUE(len + 1U > sizeof(tiny));
	TEST_ASSERT_NULL(RyanJsonPrintPreallocated(root, tiny, (uint32_t)sizeof(tiny), RyanJsonFalse, NULL));

	char buf[512];
	TEST_ASSERT_TRUE(len + 1U <= sizeof(buf));
	TEST_ASSERT_NOT_NULL(RyanJsonPrintPreallocated(root, buf, (uint32_t)sizeof(buf), RyanJsonFalse, NULL));

	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintWithStyleCustomIndent(void)
{
	// 复杂链路：
	// Parse -> PrintWithStyle(自定义缩进) -> Parse -> Compare。
	// 目标：验证自定义缩进风格可往返。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":{\"c\":2}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJsonPrintStyle style = {
		.indent = " \t", .newline = "\n", .indentLen = 2, .newlineLen = 1, .spaceAfterColon = 1, .format = RyanJsonTrue};
	char *printed = RyanJsonPrintWithStyle(root, 256, &style, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintWithStyleCrlf(void)
{
	// 复杂链路：
	// Parse -> PrintWithStyle(\r\n) -> Parse -> Compare。
	// 目标：验证 CRLF 换行风格可往返。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJsonPrintStyle style = {
		.indent = "  ", .newline = "\r\n", .indentLen = 2, .newlineLen = 2, .spaceAfterColon = 1, .format = RyanJsonTrue};
	char *printed = RyanJsonPrintWithStyle(root, 256, &style, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintWithStyleLongIndent(void)
{
	// 复杂链路：
	// Parse -> PrintWithStyle(长缩进) -> Parse -> Compare。
	// 目标：验证长缩进 String 可往返。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":{\"c\":2}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJsonPrintStyle style = {
		.indent = "    ", .newline = "\n", .indentLen = 4, .newlineLen = 1, .spaceAfterColon = 1, .format = RyanJsonTrue};
	char *printed = RyanJsonPrintWithStyle(root, 64, &style, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintWithStyleFormatFalse(void)
{
	// 复杂链路：
	// Parse -> PrintWithStyle(format=false) -> Parse -> Compare。
	// 目标：验证 style.format=false 输出仍可解析。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJsonPrintStyle style = {
		.indent = "  ", .newline = "\n", .indentLen = 2, .newlineLen = 1, .spaceAfterColon = 1, .format = RyanJsonFalse};
	char *printed = RyanJsonPrintWithStyle(root, 64, &style, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintNullRootRoundtrip(void)
{
	// 复杂链路：
	// Parse(Null) -> Print -> Parse -> Compare。
	// 目标：验证根 Null 往返一致。
	RyanJson_t root = RyanJsonParse("null");
	TEST_ASSERT_NOT_NULL(root);
	char *printed = RyanJsonPrint(root, 8, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintBoolRootRoundtrip(void)
{
	// 复杂链路：
	// Parse(Bool) -> Print -> Parse -> Compare。
	// 目标：验证根 Bool 往返一致。
	RyanJson_t root = RyanJsonParse("false");
	TEST_ASSERT_NOT_NULL(root);
	char *printed = RyanJsonPrint(root, 8, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintNumberRootRoundtrip(void)
{
	// 复杂链路：
	// Parse(Number) -> Print -> Parse -> Compare。
	// 目标：验证根数值往返一致。
	RyanJson_t root = RyanJsonParse("123.5");
	TEST_ASSERT_NOT_NULL(root);
	char *printed = RyanJsonPrint(root, 8, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintPreallocatedWithStyleExact(void)
{
	// 复杂链路：
	// PrintWithStyle -> 获取长度 -> PrintPreallocatedWithStyle(精确) -> 输出一致性。
	// 目标：验证预分配风格打印边界。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJsonPrintStyle style = {
		.indent = "  ", .newline = "\n", .indentLen = 2, .newlineLen = 1, .spaceAfterColon = 1, .format = RyanJsonTrue};
	uint32_t len = 0;
	char *printed = RyanJsonPrintWithStyle(root, 32, &style, &len);
	TEST_ASSERT_NOT_NULL(printed);

	char buf[512];
	TEST_ASSERT_TRUE(len + 1U <= sizeof(buf));
	char *out = RyanJsonPrintPreallocatedWithStyle(root, buf, (uint32_t)sizeof(buf), &style, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING(printed, out);

	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintPreallocatedFormatTrue(void)
{
	// 复杂链路：
	// Parse -> PrintPreallocated(format=true) -> Parse -> Compare。
	// 目标：验证预分配格式化打印可往返。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":{\"c\":2}}");
	TEST_ASSERT_NOT_NULL(root);

	char buf[256];
	char *out = RyanJsonPrintPreallocated(root, buf, sizeof(buf), RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL(out);
	RyanJson_t reparsed = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintWithStyleNoSpaceAfterColon(void)
{
	// 复杂链路：
	// Parse -> PrintWithStyle(spaceAfterColon=0) -> Parse -> Compare。
	// 目标：验证冒号后无空格风格可往返。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJsonPrintStyle style = {
		.indent = "  ", .newline = "\n", .indentLen = 2, .newlineLen = 1, .spaceAfterColon = 0, .format = RyanJsonTrue};
	char *printed = RyanJsonPrintWithStyle(root, 32, &style, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintWithStyleIndentLenMismatch(void)
{
	// 复杂链路：
	// Parse -> PrintWithStyle(indentLen 不匹配) -> Parse -> Compare。
	// 目标：验证 indentLen 较短时仍可输出可解析结果。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":{\"c\":2}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJsonPrintStyle style = {
		.indent = "  ", .newline = "\n", .indentLen = 1, .newlineLen = 1, .spaceAfterColon = 1, .format = RyanJsonTrue};
	char *printed = RyanJsonPrintWithStyle(root, 64, &style, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintWithStyleNewlineLenMismatch(void)
{
	// 复杂链路：
	// Parse -> PrintWithStyle(newlineLen 不匹配) -> Parse -> Compare。
	// 目标：验证 newlineLen 较短时仍可输出可解析结果。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":{\"c\":2}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJsonPrintStyle style = {
		.indent = "  ", .newline = "\r\n", .indentLen = 2, .newlineLen = 1, .spaceAfterColon = 1, .format = RyanJsonTrue};
	char *printed = RyanJsonPrintWithStyle(root, 64, &style, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintPreallocatedFormatTrueExact(void)
{
	// 复杂链路：
	// Print(format=true) -> 获取长度 -> PrintPreallocated(format=true) -> Compare。
	// 目标：验证格式化预分配输出边界。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":{\"c\":2}}");
	TEST_ASSERT_NOT_NULL(root);

	uint32_t len = 0;
	char *printed = RyanJsonPrint(root, 16, RyanJsonTrue, &len);
	TEST_ASSERT_NOT_NULL(printed);

	char buf[512];
	TEST_ASSERT_TRUE(len + 1U <= sizeof(buf));
	char *out = RyanJsonPrintPreallocated(root, buf, (uint32_t)sizeof(buf), RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING(printed, out);

	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintPreallocatedFormatTrueTooSmall(void)
{
	// 复杂链路：
	// Print(format=true) -> PrintPreallocated(不足) -> PrintPreallocated(足够)。
	// 目标：验证格式化预分配失败可恢复。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":{\"c\":2}}");
	TEST_ASSERT_NOT_NULL(root);

	uint32_t len = 0;
	char *printed = RyanJsonPrint(root, 16, RyanJsonTrue, &len);
	TEST_ASSERT_NOT_NULL(printed);

	char tiny[8];
	TEST_ASSERT_TRUE(len + 1U > sizeof(tiny));
	TEST_ASSERT_NULL(RyanJsonPrintPreallocated(root, tiny, (uint32_t)sizeof(tiny), RyanJsonTrue, NULL));

	char buf[512];
	TEST_ASSERT_TRUE(len + 1U <= sizeof(buf));
	TEST_ASSERT_NOT_NULL(RyanJsonPrintPreallocated(root, buf, (uint32_t)sizeof(buf), RyanJsonTrue, NULL));

	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintPrintPreallocatedWithStyleFormatFalse(void)
{
	// 复杂链路：
	// PrintWithStyle(format=false) -> PrintPreallocatedWithStyle -> Parse -> Compare。
	// 目标：验证 style.format=false 的预分配输出可回环。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJsonPrintStyle style = {
		.indent = "  ", .newline = "\n", .indentLen = 2, .newlineLen = 1, .spaceAfterColon = 1, .format = RyanJsonFalse};
	uint32_t len = 0;
	char *printed = RyanJsonPrintWithStyle(root, 16, &style, &len);
	TEST_ASSERT_NOT_NULL(printed);

	char buf[512];
	TEST_ASSERT_TRUE(len + 1U <= sizeof(buf));
	char *out = RyanJsonPrintPreallocatedWithStyle(root, buf, (uint32_t)sizeof(buf), &style, NULL);
	TEST_ASSERT_NOT_NULL(out);
	RyanJson_t reparsed = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

void testEdgePrintOptionsRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEdgeParsePrintPrintFormatFlag);
	RUN_TEST(testEdgeParsePrintPrintPresetSmall);
	RUN_TEST(testEdgeParsePrintPrintPreallocatedExactFitFromPrint);
	RUN_TEST(testEdgeParsePrintPrintPreallocatedTooSmallThenSuccess);
	RUN_TEST(testEdgeParsePrintPrintWithStyleCustomIndent);
	RUN_TEST(testEdgeParsePrintPrintWithStyleCrlf);
	RUN_TEST(testEdgeParsePrintPrintWithStyleLongIndent);
	RUN_TEST(testEdgeParsePrintPrintWithStyleFormatFalse);
	RUN_TEST(testEdgeParsePrintPrintNullRootRoundtrip);
	RUN_TEST(testEdgeParsePrintPrintBoolRootRoundtrip);
	RUN_TEST(testEdgeParsePrintPrintNumberRootRoundtrip);
	RUN_TEST(testEdgeParsePrintPrintPreallocatedWithStyleExact);
	RUN_TEST(testEdgeParsePrintPrintPreallocatedFormatTrue);
	RUN_TEST(testEdgeParsePrintPrintWithStyleNoSpaceAfterColon);
	RUN_TEST(testEdgeParsePrintPrintWithStyleIndentLenMismatch);
	RUN_TEST(testEdgeParsePrintPrintWithStyleNewlineLenMismatch);
	RUN_TEST(testEdgeParsePrintPrintPreallocatedFormatTrueExact);
	RUN_TEST(testEdgeParsePrintPrintPreallocatedFormatTrueTooSmall);
	RUN_TEST(testEdgeParsePrintPrintPreallocatedWithStyleFormatFalse);
}
