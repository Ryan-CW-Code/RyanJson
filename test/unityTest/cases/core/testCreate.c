#include "testBase.h"

static int32_t gCreateFailAfter = -1;
static int32_t gCreateAllocCount = 0;

static void *createFailMalloc(size_t size)
{
	if (gCreateFailAfter >= 0 && gCreateAllocCount++ >= gCreateFailAfter) { return NULL; }
	return unityTestMalloc(size);
}

static void *createFailRealloc(void *block, size_t size)
{
	if (gCreateFailAfter >= 0 && gCreateAllocCount++ >= gCreateFailAfter) { return NULL; }
	return unityTestRealloc(block, size);
}

static void createSetFailAfter(int32_t n)
{
	gCreateFailAfter = n;
	gCreateAllocCount = 0;
	RyanJsonInitHooks(createFailMalloc, unityTestFree, createFailRealloc);
}

static void createRestoreHooks(void)
{
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc);
	gCreateFailAfter = -1;
	gCreateAllocCount = 0;
}

static void testCreateEdgeCases(void)
{
	// 测试创建空列表
	RyanJson_t emptyArr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(emptyArr);
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetArraySize(emptyArr));
	RyanJsonDelete(emptyArr);

	// 测试通过辅助函数创建空列表
	// RyanJsonCreateIntArray 传入 NULL 指针和 0 长度
	RyanJson_t nullIntArr = RyanJsonCreateIntArray(NULL, 0);
	// 该场景属于边界输入：只要不崩溃且行为可预测即可。
	// 若实现返回空数组，则继续校验 size；
	// 若实现返回 NULL，也视为可接受行为。
	if (nullIntArr)
	{
		TEST_ASSERT_EQUAL_INT(0, RyanJsonGetArraySize(nullIntArr));
		RyanJsonDelete(nullIntArr);
	}
}

static void testCreatePropertyCheck(void)
{
	// 验证创建后的节点属性
	RyanJson_t node = RyanJsonCreateInt("age", 25);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_STRING("age", RyanJsonGetKey(node));
	TEST_ASSERT_TRUE(RyanJsonIsInt(node));
	TEST_ASSERT_EQUAL_INT(25, RyanJsonGetIntValue(node));
	RyanJsonDelete(node);

	node = RyanJsonCreateDouble("pi", 3.14159);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(node));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(3.14159, RyanJsonGetDoubleValue(node)));
	RyanJsonDelete(node);

	node = RyanJsonCreateBool("active", RyanJsonTrue);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonIsBool(node));
	TEST_ASSERT_EQUAL_INT(RyanJsonTrue, RyanJsonGetBoolValue(node));
	RyanJsonDelete(node);
}

static void testAddBoundary(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);

	// INT32 边界
	RyanJsonAddIntToObject(obj, "max", INT32_MAX);
	RyanJsonAddIntToObject(obj, "min", INT32_MIN);
	TEST_ASSERT_EQUAL_INT(INT32_MAX, RyanJsonGetIntValue(RyanJsonGetObjectToKey(obj, "max")));
	TEST_ASSERT_EQUAL_INT(INT32_MIN, RyanJsonGetIntValue(RyanJsonGetObjectToKey(obj, "min")));

	// Double 边界（仅做数值路径校验，不做位级比较）
	RyanJsonAddDoubleToObject(obj, "very_small", 1e-100);
	RyanJsonAddDoubleToObject(obj, "very_big", 1e+100);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(RyanJsonGetObjectToKey(obj, "very_small")));
	TEST_ASSERT_TRUE(RyanJsonIsDouble(RyanJsonGetObjectToKey(obj, "very_big")));

	// 空 key 添加测试
	RyanJsonAddBoolToObject(obj, "", RyanJsonTrue);
	TEST_ASSERT_TRUE(RyanJsonGetBoolValue(RyanJsonGetObjectToKey(obj, "")));

	RyanJsonDelete(obj);
}

static void testCreateStandardComplexHierarchy(void)
{
	RyanJson_t jsonRoot, item;

	// 对象生成测试
	jsonRoot = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, "创建对象失败");

	RyanJsonAddIntToObject(jsonRoot, "inter", 16);
	RyanJsonAddDoubleToObject(jsonRoot, "double", 16.89);
	RyanJsonAddStringToObject(jsonRoot, "string", "hello");
	RyanJsonAddBoolToObject(jsonRoot, "boolTrue", RyanJsonTrue);
	RyanJsonAddBoolToObject(jsonRoot, "boolFalse", RyanJsonFalse);
	RyanJsonAddNullToObject(jsonRoot, "null");

	/**
	 * @brief 对象添加测试
	 *
	 */
	item = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL_MESSAGE(item, "创建子对象失败");
	RyanJsonAddIntToObject(item, "inter", 16);
	RyanJsonAddDoubleToObject(item, "double", 16.89);
	RyanJsonAddStringToObject(item, "string", "hello");
	RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
	RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
	RyanJsonAddNullToObject(item, "null");
	RyanJsonAddItemToObject(jsonRoot, "item", item);

	/**
	 * @brief 数组添加测试
	 *
	 */
	int32_t arrayInt[] = {16, 16, 16, 16, 16};
	RyanJsonAddItemToObject(jsonRoot, "arrayInt", RyanJsonCreateIntArray(arrayInt, sizeof(arrayInt) / sizeof(arrayInt[0])));

	double arrayDouble[] = {16.89, 16.89, 16.89, 16.89, 16.89};
	RyanJsonAddItemToObject(jsonRoot, "arrayDouble",
				RyanJsonCreateDoubleArray(arrayDouble, sizeof(arrayDouble) / sizeof(arrayDouble[0])));

	const char *arrayString[] = {"hello", "hello", "hello", "hello", "hello"};
	RyanJsonAddItemToObject(jsonRoot, "arrayString",
				RyanJsonCreateStringArray(arrayString, sizeof(arrayString) / sizeof(arrayString[0])));

	RyanJson_t array = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL_MESSAGE(array, "创建数组失败");
	RyanJsonAddIntToArray(array, 16);
	RyanJsonAddDoubleToArray(array, 16.89);
	RyanJsonAddStringToArray(array, "hello");
	RyanJsonAddBoolToArray(array, RyanJsonTrue);
	RyanJsonAddBoolToArray(array, RyanJsonFalse);
	RyanJsonAddNullToArray(array);
	RyanJsonAddItemToObject(jsonRoot, "array", array);

	/**
	 * @brief 对象数组测试
	 *
	 */
	RyanJson_t arrayItem = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL_MESSAGE(arrayItem, "创建对象数组失败");

	item = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL_MESSAGE(item, "创建对象数组项 1 失败");
	RyanJsonAddIntToObject(item, "inter", 16);
	RyanJsonAddDoubleToObject(item, "double", 16.89);
	RyanJsonAddStringToObject(item, "string", "hello");
	RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
	RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
	RyanJsonAddNullToObject(item, "null");
	RyanJsonAddItemToArray(arrayItem, item);

	item = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL_MESSAGE(item, "创建对象数组项 2 失败");
	RyanJsonAddIntToObject(item, "inter", 16);
	RyanJsonAddDoubleToObject(item, "double", 16.89);
	RyanJsonAddStringToObject(item, "string", "hello");
	RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
	RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
	RyanJsonAddNullToObject(item, "null");
	RyanJsonAddItemToArray(arrayItem, item);

	RyanJsonAddItemToObject(jsonRoot, "arrayItem", arrayItem);

	// 最终验证根对象结构
#if true == RyanJsonDefaultAddAtHead
	testCheckRootEx(jsonRoot, RyanJsonTrue);
#else
	testCheckRootEx(jsonRoot, RyanJsonFalse);
#endif

	RyanJsonDelete(jsonRoot);
}

static void testCreateHugeString(void)
{
	// 极限大字符串创建（模拟大对象；为控制测试耗时，先用 10KB 堆内存）
	uint32_t len = 1024 * 10;
	char *hugeStr = (char *)malloc(len + 1);
	TEST_ASSERT_NOT_NULL(hugeStr);
	memset(hugeStr, 'A', len);
	hugeStr[len] = '\0';

	RyanJson_t strJson = RyanJsonCreateString("huge", hugeStr);
	TEST_ASSERT_NOT_NULL(strJson);
	TEST_ASSERT_EQUAL_STRING(hugeStr, RyanJsonGetStringValue(strJson));

	RyanJsonDelete(strJson);
	free(hugeStr);

	// 特殊 Key 创建
	RyanJson_t nullKeyJson = RyanJsonCreateInt(NULL, 123);
	TEST_ASSERT_NOT_NULL(nullKeyJson); // 允许 NULL key（常见于 Array item）
	// 无 key 节点下，RyanJsonGetKey 可能返回值区指针。
	// 因此这里用 RyanJsonIsKey 作为权威判定。
	TEST_ASSERT_FALSE(RyanJsonIsKey(nullKeyJson));
	RyanJsonDelete(nullKeyJson);
}

static void testCreateOom(void)
{
	createSetFailAfter(0);
	RyanJson_t obj = RyanJsonCreateObject();
	createRestoreHooks();
	if (obj) { RyanJsonDelete(obj); }
	TEST_ASSERT_NULL_MESSAGE(obj, "CreateObject OOM 应返回 NULL");
}

static void testInsertOutOfRangeAndKeyValidation(void)
{
	// Array：index 超出范围应追加到尾部
	RyanJson_t arr = RyanJsonCreateArray();
	RyanJsonAddIntToArray(arr, 1);
	RyanJsonAddIntToArray(arr, 2);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 100, RyanJsonCreateInt(NULL, 3)));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetSize(arr));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 2)));
	RyanJsonDelete(arr);

	// Object：item 无 key 应失败
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJsonAddIntToObject(obj, "a", 1);
	RyanJson_t noKey = RyanJsonCreateInt(NULL, 2);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonInsert(obj, 0, noKey), "Object 插入无 key item 应失败");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(obj));

	// Object：重复 key 应失败
#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("a", 2)), "严格模式下 Object 插入重复 key 应失败");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonAddIntToObject(obj, "a", 3), "严格模式下 AddIntToObject 重复 key 应失败");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(obj));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));
#else
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("a", 2)), "非严格模式下 Object 插入重复 key 应成功");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToObject(obj, "a", 3), "非严格模式下 AddIntToObject 重复 key 应成功");
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetSize(obj));
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));
#else
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));
#endif
#endif
	RyanJsonDelete(obj);
}

static void testInsertRejectAttachedItem(void)
{
	RyanJson_t arr1 = RyanJsonCreateArray();
	RyanJson_t arr2 = RyanJsonCreateArray();

	RyanJson_t item = RyanJsonCreateInt(NULL, 1);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr1, 0, item));

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonInsert(arr2, 0, item), "已挂树的 item 不应再次插入");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(arr1));
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(arr2));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr1, 0)));

	RyanJsonDelete(arr1);
	RyanJsonDelete(arr2);
}

static void testDetachedItemApi(void)
{
	TEST_ASSERT_FALSE(RyanJsonIsDetachedItem(NULL));

	RyanJson_t arr = RyanJsonCreateArray();
	RyanJson_t item = RyanJsonCreateInt(NULL, 123);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(item));

	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, item));
	TEST_ASSERT_FALSE(RyanJsonIsDetachedItem(item));

	RyanJson_t detached = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_EQUAL_PTR(item, detached);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(detached));

	// 人工篡改到“非法游离态”：next==NULL 但 IsLast==1，应判定为非游离节点
	RyanJsonSetPayloadIsLastByFlag(detached, RyanJsonTrue);
	TEST_ASSERT_FALSE(RyanJsonIsDetachedItem(detached));
	// 恢复状态，避免影响后续释放
	RyanJsonSetPayloadIsLastByFlag(detached, RyanJsonFalse);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(detached));

	RyanJsonDelete(detached);
	RyanJsonDelete(arr);
}

static void testAddLargeArrayStress(void)
{
	// 压力测试：添加大量元素
	RyanJson_t arr = RyanJsonCreateArray();
	int32_t count = 2000; // 2000 个元素
	for (int32_t i = 0; i < count; i++)
	{
		RyanJsonAddIntToArray(arr, i);
	}
	TEST_ASSERT_EQUAL_INT(count, RyanJsonGetArraySize(arr));

	// 验证最后一个
	RyanJson_t last = RyanJsonGetObjectByIndex(arr, count - 1);
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetIntValue(last));
#else
	TEST_ASSERT_EQUAL_INT(count - 1, RyanJsonGetIntValue(last));
#endif

	RyanJsonDelete(arr);

	// 防御性测试：向非容器添加
	RyanJson_t strNode = RyanJsonCreateString("k", "v");
	// 尝试向 String 节点添加子项，预期失败或不崩溃
	RyanJson_t item = RyanJsonCreateInt("sub", 1);
	RyanJsonBool_e res = RyanJsonInsert(strNode, 0, item);
	TEST_ASSERT_FALSE(res);

	// 该场景 item 是游离节点，Insert 失败会负责释放 item
	RyanJsonDelete(strNode);
}

static void testAddItemObjectSemantics(void)
{
	// AddItemToObject 正常添加容器
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJson_t childArr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_NOT_NULL(childArr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(childArr, 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(childArr, 2));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(obj, "arr", childArr), "AddItemToObject(容器) 应成功");

	RyanJson_t arrNode = RyanJsonGetObjectByKey(obj, "arr");
	TEST_ASSERT_NOT_NULL(arrNode);
	TEST_ASSERT_TRUE(RyanJsonIsArray(arrNode));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetArraySize(arrNode));
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arrNode, 0)));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arrNode, 1)));
#else
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arrNode, 0)));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arrNode, 1)));
#endif

	// AddItemToObject 重复 key（容器）行为由严格模式控制
	RyanJson_t dupObj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(dupObj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(dupObj, "k", 99));
#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonAddItemToObject(obj, "arr", dupObj), "严格模式下 AddItemToObject(重复 key) 应失败");
#else
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(obj, "arr", dupObj), "非严格模式下 AddItemToObject(重复 key) 应成功");
#endif

#if true == RyanJsonStrictObjectKeyCheck
	arrNode = RyanJsonGetObjectByKey(obj, "arr");
	TEST_ASSERT_NOT_NULL(arrNode);
	TEST_ASSERT_TRUE(RyanJsonIsArray(arrNode));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetArraySize(arrNode));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(obj));
#else
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetSize(obj));

	RyanJson_t firstArrKeyNode = RyanJsonGetObjectByIndex(obj, 0);
	RyanJson_t secondArrKeyNode = RyanJsonGetObjectByIndex(obj, 1);
	TEST_ASSERT_NOT_NULL(firstArrKeyNode);
	TEST_ASSERT_NOT_NULL(secondArrKeyNode);
	TEST_ASSERT_TRUE(RyanJsonIsKey(firstArrKeyNode));
	TEST_ASSERT_TRUE(RyanJsonIsKey(secondArrKeyNode));
	TEST_ASSERT_EQUAL_STRING("arr", RyanJsonGetKey(firstArrKeyNode));
	TEST_ASSERT_EQUAL_STRING("arr", RyanJsonGetKey(secondArrKeyNode));

#if true == RyanJsonDefaultAddAtHead
	// 头插模式：新插入的 dupObj 在前，旧数组在后
	TEST_ASSERT_TRUE(RyanJsonIsObject(firstArrKeyNode));
	TEST_ASSERT_TRUE(RyanJsonIsArray(secondArrKeyNode));
	TEST_ASSERT_EQUAL_INT(99, RyanJsonGetIntValue(RyanJsonGetObjectByKey(firstArrKeyNode, "k")));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetArraySize(secondArrKeyNode));

	arrNode = RyanJsonGetObjectByKey(obj, "arr");
	TEST_ASSERT_NOT_NULL(arrNode);
	TEST_ASSERT_TRUE(RyanJsonIsObject(arrNode));
#else
	// 尾插模式：旧数组在前，新插入的 dupObj 在后
	TEST_ASSERT_TRUE(RyanJsonIsArray(firstArrKeyNode));
	TEST_ASSERT_TRUE(RyanJsonIsObject(secondArrKeyNode));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetArraySize(firstArrKeyNode));
	TEST_ASSERT_EQUAL_INT(99, RyanJsonGetIntValue(RyanJsonGetObjectByKey(secondArrKeyNode, "k")));

	arrNode = RyanJsonGetObjectByKey(obj, "arr");
	TEST_ASSERT_NOT_NULL(arrNode);
	TEST_ASSERT_TRUE(RyanJsonIsArray(arrNode));
#endif
#endif

	// AddItemToObject 传入标量应失败
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonAddItemToObject(obj, "scalar", RyanJsonCreateInt(NULL, 7)), "AddItemToObject(标量) 应失败");
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(obj, "scalar"));

	// 非对象调用 AddItemToObject 应失败
	RyanJson_t notObj = RyanJsonCreateInt("num", 1);
	RyanJson_t childObj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(notObj);
	TEST_ASSERT_NOT_NULL(childObj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(childObj, "x", 1));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonAddItemToObject(notObj, "child", childObj), "非对象调用 AddItemToObject 应失败");
	TEST_ASSERT_TRUE(RyanJsonIsInt(notObj));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(notObj));

	RyanJsonDelete(notObj);
	RyanJsonDelete(obj);
}

void testCreateRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testCreateEdgeCases);
	RUN_TEST(testCreatePropertyCheck);
	RUN_TEST(testAddBoundary);
	RUN_TEST(testCreateStandardComplexHierarchy);
	RUN_TEST(testCreateHugeString);
	RUN_TEST(testCreateOom);
	RUN_TEST(testInsertOutOfRangeAndKeyValidation);
	RUN_TEST(testInsertRejectAttachedItem);
	RUN_TEST(testDetachedItemApi);
	RUN_TEST(testAddLargeArrayStress);
	RUN_TEST(testAddItemObjectSemantics);
}
