#include "testBase.h"

static void testAccessorContainerEntryAndSiblingTraversal(void)
{
	// 覆盖核心“入口取值 + 兄弟遍历”语义：
	// - Object 用 RyanJsonGetObjectValue 取首子节点；
	// - Array 用 RyanJsonGetArrayValue 取首元素；
	// - 使用 RyanJsonGetNext 遍历同层链表并校验计数与 GetSize 一致。
	const char *jsonText = "{\"obj\":{\"x\":1,\"y\":2},\"arr\":[10,20,30],\"str\":\"ok\",\"flag\":true}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "基础 Object 解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsObject(root));

	RyanJson_t firstChild = RyanJsonGetObjectValue(root);
	TEST_ASSERT_NOT_NULL_MESSAGE(firstChild, "Object 首子节点不应为空");

	uint32_t objectCount = 0;
	RyanJson_t child = firstChild;
	RyanJson_t objectLast = NULL;
	while (child)
	{
		objectCount++;
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsKey(child), "Object 子节点必须携带 key");
		objectLast = child;
		child = RyanJsonGetNext(child);
	}
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(RyanJsonGetSize(root), objectCount, "Object 链表遍历计数与 GetSize 不一致");
	TEST_ASSERT_NOT_NULL(objectLast);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(objectLast), "Object 尾节点的 GetNext 应返回 NULL");

	RyanJson_t arr = RyanJsonGetObjectToKey(root, "arr");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonIsArray(arr));
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));

	RyanJson_t arrHeadByValue = RyanJsonGetArrayValue(arr);
	RyanJson_t arrHeadByIndex = RyanJsonGetObjectByIndex(arr, 0);
	TEST_ASSERT_EQUAL_PTR_MESSAGE(arrHeadByIndex, arrHeadByValue, "GetArrayValue 与 GetObjectByIndex(0) 应返回同一节点");

	uint32_t arrayCount = 0;
	RyanJson_t elem = arrHeadByValue;
	RyanJson_t arrayLast = NULL;
	while (elem)
	{
		arrayCount++;
		TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsKey(elem), "Array 元素不应携带 key");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsNumber(elem), "Array 元素应为 number");
		arrayLast = elem;
		elem = RyanJsonGetNext(elem);
	}
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(RyanJsonGetArraySize(arr), arrayCount, "Array 链表遍历计数与 GetArraySize 不一致");
	TEST_ASSERT_NOT_NULL(arrayLast);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(arrayLast), "Array 尾元素的 GetNext 应返回 NULL");

	// 标量节点只做类型边界校验，不直接调用容器入口取值 API。
	RyanJson_t strNode = RyanJsonGetObjectToKey(root, "str");
	TEST_ASSERT_NOT_NULL(strNode);
	TEST_ASSERT_TRUE(RyanJsonIsString(strNode));
	TEST_ASSERT_FALSE(RyanJsonIsObject(strNode));
	TEST_ASSERT_FALSE(RyanJsonIsArray(strNode));

	RyanJsonDelete(root);
}

static void testAccessorVarargsLookupSuccessAndFailure(void)
{
	// 覆盖变参路径查询 API 的成功/失败分支：
	// - RyanJsonGetObjectByKeys
	// - RyanJsonGetObjectByIndexs
	// 并验证与便捷宏 RyanJsonGetObjectToKey/ToIndex 返回一致。
	const char *jsonText = "{\"meta\":{\"list\":[{\"id\":\"n0\"},{\"id\":\"n1\",\"child\":[{\"v\":1},{\"v\":2}]}]}}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "变参路径测试样本解析失败");

	RyanJson_t listByKeys = RyanJsonGetObjectByKeys(root, "meta", "list", NULL);
	RyanJson_t listByMacro = RyanJsonGetObjectToKey(root, "meta", "list");
	TEST_ASSERT_NOT_NULL(listByKeys);
	TEST_ASSERT_EQUAL_PTR_MESSAGE(listByMacro, listByKeys, "GetObjectByKeys 与 GetObjectToKey 结果不一致");
	TEST_ASSERT_TRUE(RyanJsonIsArray(listByKeys));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(listByKeys));

	RyanJson_t secondByIndexs = RyanJsonGetObjectByIndexs(listByKeys, 1, UINT32_MAX);
	RyanJson_t secondByMacro = RyanJsonGetObjectToIndex(listByKeys, 1);
	TEST_ASSERT_NOT_NULL(secondByIndexs);
	TEST_ASSERT_EQUAL_PTR_MESSAGE(secondByMacro, secondByIndexs, "GetObjectByIndexs 与 GetObjectToIndex 结果不一致");
	TEST_ASSERT_EQUAL_STRING("n1", RyanJsonGetStringValue(RyanJsonGetObjectToKey(secondByIndexs, "id")));

	RyanJson_t childArr = RyanJsonGetObjectToKey(secondByIndexs, "child");
	TEST_ASSERT_NOT_NULL(childArr);
	TEST_ASSERT_TRUE(RyanJsonIsArray(childArr));

	RyanJson_t secondChildObj = RyanJsonGetObjectByIndexs(childArr, 1, UINT32_MAX);
	TEST_ASSERT_NOT_NULL(secondChildObj);
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(secondChildObj, "v")));

	// 失败路径：缺 key、越界 index、以及“中途深入失败”。
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "meta", "missing", NULL), "缺失 key 路径应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(listByKeys, 2, UINT32_MAX), "越界 index 路径应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(childArr, 1, 3, UINT32_MAX), "中途深入失败路径应返回 NULL");

	RyanJsonDelete(root);
}

static void testAccessorVarargsMacrosOnConstructedTree(void)
{
	// 覆盖手工构造树上的便捷宏路径查询：
	// - 不依赖 Parse，直接验证 Create/Add 后多级路径可达；
	// - 缺失 key 与越界 index 仍返回 NULL。
	RyanJson_t root = RyanJsonCreateObject();
	RyanJson_t objA = RyanJsonCreateObject();
	RyanJson_t objB = RyanJsonCreateObject();
	RyanJson_t arr = RyanJsonCreateArray();
	RyanJson_t sub = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_NOT_NULL(objA);
	TEST_ASSERT_NOT_NULL(objB);
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_NOT_NULL(sub);

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(objB, "c", 3));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(objA, "b", objB));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "a", objA));

	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(sub, 7));
	TEST_ASSERT_TRUE(RyanJsonAddItemToArray(arr, sub));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "arr", arr));

	RyanJson_t c = RyanJsonGetObjectToKey(root, "a", "b", "c");
	TEST_ASSERT_NOT_NULL_MESSAGE(c, "构造树上的 key 路径查询失败");
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, RyanJsonGetIntValue(c), "构造树上的 key 路径取值错误");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(root, "a", "missing"), "构造树上的缺失 key 应返回 NULL");

	RyanJson_t arrNode = RyanJsonGetObjectToKey(root, "arr");
	RyanJson_t v = RyanJsonGetObjectToIndex(arrNode, 0, 0);
	TEST_ASSERT_NOT_NULL_MESSAGE(v, "构造树上的 index 路径查询失败");
	TEST_ASSERT_EQUAL_INT_MESSAGE(7, RyanJsonGetIntValue(v), "构造树上的 index 路径取值错误");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToIndex(arrNode, 1), "构造树上的越界 index 应返回 NULL");

	RyanJsonDelete(root);
}

static void testAccessorVarargsStopOnScalar(void)
{
	// 覆盖路径查询在“经过标量节点”时应返回 NULL 而非崩溃：
	// - GetObjectByKeys 在标量处继续下钻应失败；
	// - GetObjectByIndexs 在标量处继续下钻应失败。
	const char *jsonText = "{\"obj\":{\"name\":\"str\",\"num\":1},\"arr\":[1,2]}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "标量路径样本解析失败");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "obj", "name", "x", NULL), "标量 String 下钻应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "obj", "num", "x", NULL), "标量 Number 下钻应返回 NULL");

	RyanJson_t arr = RyanJsonGetObjectByKey(root, "arr");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonIsArray(arr));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(arr, 0, 0, UINT32_MAX), "标量 Array 元素下钻应返回 NULL");

	RyanJsonDelete(root);
}

static void testAccessorTypePredicateMatrix(void)
{
	// 覆盖类型判定矩阵，重点验证 RyanJsonIsNumber 在 Int/Double 上为真，
	// 且不会误判到 Bool/String/Null/Object/Array。
	RyanJson_t root = RyanJsonParse("[null,true,1,1.5,\"1\",{},[]]");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "类型矩阵解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsArray(root));
	TEST_ASSERT_EQUAL_UINT32(7U, RyanJsonGetArraySize(root));

	RyanJson_t n0 = RyanJsonGetObjectByIndex(root, 0);
	RyanJson_t n1 = RyanJsonGetObjectByIndex(root, 1);
	RyanJson_t n2 = RyanJsonGetObjectByIndex(root, 2);
	RyanJson_t n3 = RyanJsonGetObjectByIndex(root, 3);
	RyanJson_t n4 = RyanJsonGetObjectByIndex(root, 4);
	RyanJson_t n5 = RyanJsonGetObjectByIndex(root, 5);
	RyanJson_t n6 = RyanJsonGetObjectByIndex(root, 6);

	TEST_ASSERT_TRUE(RyanJsonIsNull(n0));
	TEST_ASSERT_TRUE(RyanJsonIsBool(n1));
	TEST_ASSERT_TRUE(RyanJsonIsInt(n2));
	TEST_ASSERT_TRUE(RyanJsonIsDouble(n3));
	TEST_ASSERT_TRUE(RyanJsonIsString(n4));
	TEST_ASSERT_TRUE(RyanJsonIsObject(n5));
	TEST_ASSERT_TRUE(RyanJsonIsArray(n6));

	TEST_ASSERT_FALSE(RyanJsonIsNumber(n0));
	TEST_ASSERT_FALSE(RyanJsonIsNumber(n1));
	TEST_ASSERT_TRUE(RyanJsonIsNumber(n2));
	TEST_ASSERT_TRUE(RyanJsonIsNumber(n3));
	TEST_ASSERT_FALSE(RyanJsonIsNumber(n4));
	TEST_ASSERT_FALSE(RyanJsonIsNumber(n5));
	TEST_ASSERT_FALSE(RyanJsonIsNumber(n6));
	TEST_ASSERT_FALSE(RyanJsonIsNull(NULL));

	RyanJsonDelete(root);
}

static void testAccessorLookupArgumentGuards(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(NULL, "a"), "NULL 根节点按 key 查询应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToIndex(NULL, 0), "NULL 根节点按 index 查询应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKey(NULL, "a"), "GetObjectByKey(NULL, key) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKey(obj, NULL), "GetObjectByKey(obj, NULL) 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testAccessorTypedArrayCreateAndRoundtrip(void)
{
	// 覆盖 CreateIntArray/CreateDoubleArray/CreateStringArray 与
	// Parse/Print/Compare 的联动路径，验证“构造型 API”生成结果可稳定往返。
	const int32_t intValues[] = {3, 2, 1};
	const double doubleValues[] = {1.25, -2.5};
	const char *stringValues[] = {"alpha", "beta"};

	RyanJson_t root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t intArr = RyanJsonCreateIntArray(intValues, (uint32_t)(sizeof(intValues) / sizeof(intValues[0])));
	RyanJson_t doubleArr = RyanJsonCreateDoubleArray(doubleValues, (uint32_t)(sizeof(doubleValues) / sizeof(doubleValues[0])));
	RyanJson_t stringArr = RyanJsonCreateStringArray(stringValues, (uint32_t)(sizeof(stringValues) / sizeof(stringValues[0])));

	TEST_ASSERT_NOT_NULL(intArr);
	TEST_ASSERT_NOT_NULL(doubleArr);
	TEST_ASSERT_NOT_NULL(stringArr);

	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "ints", intArr));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "doubles", doubleArr));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "strings", stringArr));

	TEST_ASSERT_TRUE(RyanJsonIsArray(RyanJsonGetObjectToKey(root, "ints")));
	TEST_ASSERT_TRUE(RyanJsonIsArray(RyanJsonGetObjectToKey(root, "doubles")));
	TEST_ASSERT_TRUE(RyanJsonIsArray(RyanJsonGetObjectToKey(root, "strings")));
	TEST_ASSERT_TRUE(RyanJsonIsNumber(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(root, "ints"), 0)));
	TEST_ASSERT_TRUE(RyanJsonIsNumber(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(root, "doubles"), 0)));
	TEST_ASSERT_TRUE(RyanJsonIsString(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(root, "strings"), 0)));

	char *printed = RyanJsonPrint(root, 256, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "构造型 Array 样本打印失败");

	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "构造型 Array 样本往返解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, roundtrip), "构造型 Array 样本往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testAccessorHasMacrosCoverage(void)
{
	// 覆盖 HasObjectByKey/ByIndex/ToKey/ToIndex 宏的成功与失败路径，
	// 重点验证“路径存在性判断”与“实际节点可达性”保持一致。
	const char *jsonText = "{\"meta\":{\"arr\":[{\"name\":\"n0\"},{\"name\":\"n1\"}],\"obj\":{\"k\":1}},\"flag\":true}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "Has 宏覆盖样本解析失败");

	RyanJson_t arr = RyanJsonGetObjectToKey(root, "meta", "arr");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonIsArray(arr));

	RyanJson_t secondObj = RyanJsonGetObjectToIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(secondObj);
	TEST_ASSERT_TRUE(RyanJsonIsObject(secondObj));

	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, "meta"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "meta", "arr"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "meta", "obj", "k"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByIndex(arr, 0));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByIndex(arr, 1));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(secondObj, "name"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToIndex(arr, 1, 0));

	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "missing"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "meta", "missing"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByIndex(arr, 2));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToIndex(arr, 1, 1));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToIndex(arr, 5));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(NULL, "meta"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByIndex(NULL, 0));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(NULL, "meta"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToIndex(NULL, 0));

	RyanJsonDelete(root);
}

static void testAccessorNullVsMissingSemantics(void)
{
	// 覆盖 Null 字段与缺失字段的语义区别：
	// - HasObjectByKey 对显式 Null 应返回 true；
	// - 缺失 key 应返回 false；
	// - GetObjectByKey 对 Null 节点返回非 NULL 指针。
	RyanJson_t root = RyanJsonParse("{\"k\":null,\"v\":1}");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "null 语义样本解析失败");

	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, "k"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "missing"));

	RyanJson_t kNode = RyanJsonGetObjectByKey(root, "k");
	TEST_ASSERT_NOT_NULL_MESSAGE(kNode, "null 字段应返回节点指针");
	TEST_ASSERT_TRUE(RyanJsonIsNull(kNode));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKey(root, "missing"), "缺失字段应返回 NULL");

	RyanJsonDelete(root);
}

static void testAccessorNullLeafInNestedObject(void)
{
	// 覆盖嵌套路径上的 Null 叶子语义：
	// - HasObjectToKey 对 Null 叶子应返回 true；
	// - GetObjectByKeys 返回的节点非 NULL 且类型为 Null。
	RyanJson_t root = RyanJsonParse("{\"obj\":{\"n\":null,\"v\":1}}");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "嵌套 null 样本解析失败");

	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "obj", "n"));
	RyanJson_t nNode = RyanJsonGetObjectByKeys(root, "obj", "n", NULL);
	TEST_ASSERT_NOT_NULL(nNode);
	TEST_ASSERT_TRUE(RyanJsonIsNull(nNode));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKeys(root, "obj", "v", NULL)));

	RyanJsonDelete(root);
}

static void testAccessorHasObjectByIndexForNullElement(void)
{
	// 覆盖 Array 元素为 Null 时 HasObjectByIndex 的语义。
	RyanJson_t root = RyanJsonParse("[null,1]");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "null Array 样本解析失败");

	TEST_ASSERT_TRUE(RyanJsonHasObjectByIndex(root, 0));
	TEST_ASSERT_TRUE(RyanJsonIsNull(RyanJsonGetObjectByIndex(root, 0)));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByIndex(root, 1));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(root, 1)));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByIndex(root, 2));

	RyanJsonDelete(root);
}

static void testAccessorVarargsRootTypeMismatch(void)
{
	// 覆盖变参路径在根类型不匹配时返回 NULL：
	// - GetObjectByKeys 应拒绝 Array 根/标量根；
	// - GetObjectByIndexs 应拒绝标量根。
	RyanJson_t arr = RyanJsonParse("[{\"a\":1}]");
	RyanJson_t scalar = RyanJsonParse("1");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_NOT_NULL(scalar);

	TEST_ASSERT_NULL(RyanJsonGetObjectByKeys(arr, "a", NULL));
	TEST_ASSERT_NULL(RyanJsonGetObjectByKeys(scalar, "a", NULL));
	TEST_ASSERT_NULL(RyanJsonGetObjectByIndexs(scalar, 0, UINT32_MAX));

	RyanJsonDelete(scalar);
	RyanJsonDelete(arr);
}

static void testAccessorObjectIndexTraversalCoverage(void)
{
	// 覆盖 Object 按索引获取的边界与一致性：
	// - [0, size-1] 必须可达；
	// - size 与超大索引必须返回 NULL；
	// - 索引遍历计数应与链表遍历计数一致。
	const char *jsonText = "{\"a\":1,\"b\":2,\"c\":3,\"d\":4}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "Object 索引遍历样本解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsObject(root));
	TEST_ASSERT_EQUAL_UINT32(4U, RyanJsonGetSize(root));

	uint32_t indexCount = 0;
	for (uint32_t i = 0; i < RyanJsonGetSize(root); i++)
	{
		RyanJson_t node = RyanJsonGetObjectByIndex(root, i);
		TEST_ASSERT_NOT_NULL_MESSAGE(node, "Object 索引范围内节点不应为空");
		TEST_ASSERT_TRUE(RyanJsonIsKey(node));
		TEST_ASSERT_TRUE(RyanJsonIsNumber(node));
		indexCount++;
	}
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndex(root, RyanJsonGetSize(root)), "Object index==size 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndex(root, UINT32_MAX), "Object 超大索引应返回 NULL");

	uint32_t siblingCount = 0;
	RyanJson_t node = RyanJsonGetObjectValue(root);
	while (node)
	{
		siblingCount++;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(indexCount, siblingCount, "Object index 遍历与兄弟链表遍历计数不一致");

	RyanJsonDelete(root);
}

static void testAccessorEmptyContainerEntryValue(void)
{
	// 覆盖空容器在入口访问 API 上的语义：
	// 空 Object/Array 的首元素入口都应为 NULL，且挂载后语义不变。
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_NOT_NULL(arr);

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectValue(obj), "空 Object GetObjectValue 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetArrayValue(arr), "空 Array GetArrayValue 应返回 NULL");

	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(obj, "arr", arr));
	TEST_ASSERT_TRUE(RyanJsonIsArray(RyanJsonGetObjectToKey(obj, "arr")));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetArrayValue(RyanJsonGetObjectToKey(obj, "arr")), "挂载后的空 Array 入口仍应为 NULL");

	RyanJsonDelete(obj);
}

static void testAccessorGetSizeNullAndScalarRoot(void)
{
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, RyanJsonGetSize(NULL), "GetSize(NULL) 应返回 0");

	RyanJson_t scalar = RyanJsonParse("1");
	TEST_ASSERT_NOT_NULL(scalar);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, RyanJsonGetSize(scalar), "标量根节点 GetSize 应返回 1");

	RyanJsonDelete(scalar);
}

static void testAccessorNullMidPathStopsTraversal(void)
{
	// 覆盖路径查询在中途遇到 Null 时的行为：
	// - GetObjectByKeys 不应继续下钻；
	// - GetObjectByIndexs 不应继续下钻。
	RyanJson_t root = RyanJsonParse("{\"obj\":{\"n\":null},\"arr\":[null,{\"k\":1}]}");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "null 中途样本解析失败");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "obj", "n", "x", NULL), "null 中途下钻应返回 NULL");

	RyanJson_t arr = RyanJsonGetObjectByKey(root, "arr");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonIsArray(arr));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(arr, 0, 0, UINT32_MAX), "null Array 元素下钻应返回 NULL");

	RyanJsonDelete(root);
}

static void testAccessorGetNextAfterDetachAndInsert(void)
{
	// 覆盖 Array 在 Detach/Insert 后的兄弟链表一致性：
	// - 分离中间元素后，前后元素应直接相邻；
	// - 重新按索引插入后，顺序与链表遍历应恢复预期。
	RyanJson_t arr = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL_MESSAGE(arr, "Array 样本解析失败");
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));

	RyanJson_t first = RyanJsonGetObjectByIndex(arr, 0);
	RyanJson_t second = RyanJsonGetObjectByIndex(arr, 1);
	RyanJson_t third = RyanJsonGetObjectByIndex(arr, 2);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_NOT_NULL(third);
	TEST_ASSERT_EQUAL_PTR(second, RyanJsonGetNext(first));
	TEST_ASSERT_EQUAL_PTR(third, RyanJsonGetNext(second));
	TEST_ASSERT_NULL(RyanJsonGetNext(third));

	RyanJson_t detached = RyanJsonDetachByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离中间元素失败");
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_PTR(third, RyanJsonGetNext(first));
	TEST_ASSERT_NULL(RyanJsonGetNext(third));

	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 1, detached));
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 1)));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 2)));

	first = RyanJsonGetObjectByIndex(arr, 0);
	second = RyanJsonGetObjectByIndex(arr, 1);
	third = RyanJsonGetObjectByIndex(arr, 2);
	TEST_ASSERT_EQUAL_PTR(second, RyanJsonGetNext(first));
	TEST_ASSERT_EQUAL_PTR(third, RyanJsonGetNext(second));
	TEST_ASSERT_NULL(RyanJsonGetNext(third));

	RyanJsonDelete(arr);
}

static void testAccessorObjectHeadUpdateAfterInsert(void)
{
	// 覆盖 Object 头插后的入口更新：
	// Insert(0) 后，GetObjectValue 应始终指向新的首子节点。
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL_MESSAGE(obj, "Object 头插样本创建失败");
	TEST_ASSERT_TRUE(RyanJsonIsObject(obj));

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("a", 1)), "首次 Object 头插失败");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(RyanJsonGetObjectByKey(obj, "a"), RyanJsonGetObjectValue(obj), "首个 Object 子节点入口不正确");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("b", 2)), "第二次 Object 头插失败");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(RyanJsonGetObjectByKey(obj, "b"), RyanJsonGetObjectValue(obj), "Object 头插后入口未更新到新头节点");
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 1)));

	RyanJsonDelete(obj);
}

static void testAccessorArrayHeadUpdateAfterInsert(void)
{
	// 覆盖 Array 头插后的入口更新：
	// Insert(0) 后，GetArrayValue 应始终指向新的首元素。
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL_MESSAGE(arr, "Array 头插样本创建失败");
	TEST_ASSERT_TRUE(RyanJsonIsArray(arr));

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(arr, 0, RyanJsonCreateInt(NULL, 1)), "首次 Array 头插失败");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(RyanJsonGetObjectByIndex(arr, 0), RyanJsonGetArrayValue(arr), "首个 Array 元素入口不正确");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(arr, 0, RyanJsonCreateInt(NULL, 2)), "第二次 Array 头插失败");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(RyanJsonGetObjectByIndex(arr, 0), RyanJsonGetArrayValue(arr), "Array 头插后入口未更新到新头元素");
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 1)));

	RyanJsonDelete(arr);
}

static void testAccessorObjectHeadUpdateAfterDetach(void)
{
	// 覆盖 Object 头节点分离后的入口更新：
	// DetachByKey(头节点) 后，GetObjectValue 应指向新的首子节点。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL_MESSAGE(obj, "Object 头更新样本解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsObject(obj));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetSize(obj));

	RyanJson_t detached = RyanJsonDetachByKey(obj, "a");
	TEST_ASSERT_NOT_NULL_MESSAGE(detached, "Object 头节点分离失败");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(detached));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(obj));

	RyanJson_t newHead = RyanJsonGetObjectValue(obj);
	TEST_ASSERT_NOT_NULL_MESSAGE(newHead, "Object 头节点分离后入口不应为空");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(RyanJsonGetObjectByKey(obj, "b"), newHead, "Detach 后 Object 头入口应与 key=b 节点一致");
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetKey(newHead));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(newHead), "仅剩单个 Object 子节点时 GetNext 应为 NULL");

	RyanJsonDelete(detached);
	RyanJsonDelete(obj);
}

static void testAccessorVarargsSentinelBehavior(void)
{
	// 覆盖变参查询函数在“立即终止哨兵”场景下的行为：
	// - Keys 路径传入 NULL 哨兵时应返回首层命中节点；
	// - Indexs 路径传入 UINT32_MAX 哨兵时应返回首层命中节点。
	const char *jsonText = "{\"meta\":{\"arr\":[10,20],\"obj\":{\"k\":\"v\"}}}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "哨兵行为样本解析失败");

	RyanJson_t metaByKeys = RyanJsonGetObjectByKeys(root, "meta", NULL);
	RyanJson_t metaByMacro = RyanJsonGetObjectToKey(root, "meta");
	TEST_ASSERT_NOT_NULL(metaByKeys);
	TEST_ASSERT_EQUAL_PTR_MESSAGE(metaByMacro, metaByKeys, "GetObjectByKeys 哨兵行为不符合预期");

	RyanJson_t arr = RyanJsonGetObjectByKeys(root, "meta", "arr", NULL);
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonIsArray(arr));

	RyanJson_t firstByIndexs = RyanJsonGetObjectByIndexs(arr, 0, UINT32_MAX);
	RyanJson_t firstByMacro = RyanJsonGetObjectToIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(firstByIndexs);
	TEST_ASSERT_EQUAL_PTR_MESSAGE(firstByMacro, firstByIndexs, "GetObjectByIndexs 哨兵行为不符合预期");
	TEST_ASSERT_EQUAL_INT(10, RyanJsonGetIntValue(firstByIndexs));

	RyanJsonDelete(root);
}

static void testAccessorArrayHeadUpdateAfterDetach(void)
{
	// 覆盖 Array 头元素分离后的入口更新：
	// DetachByIndex(0) 后，GetArrayValue 应指向新的头元素。
	RyanJson_t arr = RyanJsonParse("[5,6,7]");
	TEST_ASSERT_NOT_NULL_MESSAGE(arr, "Array 头更新样本解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsArray(arr));
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));

	RyanJson_t oldHead = RyanJsonGetArrayValue(arr);
	TEST_ASSERT_NOT_NULL(oldHead);
	TEST_ASSERT_EQUAL_INT(5, RyanJsonGetIntValue(oldHead));

	RyanJson_t detached = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(detached);
	TEST_ASSERT_EQUAL_INT(5, RyanJsonGetIntValue(detached));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(arr));

	RyanJson_t newHead = RyanJsonGetArrayValue(arr);
	TEST_ASSERT_NOT_NULL(newHead);
	TEST_ASSERT_EQUAL_INT(6, RyanJsonGetIntValue(newHead));
	TEST_ASSERT_EQUAL_PTR_MESSAGE(RyanJsonGetObjectByIndex(arr, 0), newHead, "Detach 后 Array 头入口应与 index=0 一致");

	RyanJsonDelete(detached);
	RyanJsonDelete(arr);
}

static void testAccessorPathConsistencyAfterSequentialMutations(void)
{
	// 复杂链路：
	// Parse -> GetObjectByKeys/GetObjectByIndexs 读取 -> DetachByIndex -> AddItemToObject
	// -> ReplaceByIndex -> HasObjectToKey/ToIndex 二次校验 -> Compare(期望文档)。
	// 目标：
	// - 验证路径访问 API 在“跨容器迁移 + 局部替换”后仍返回正确节点；
	// - 验证 Has 宏与实际可达路径保持一致；
	// - 验证复杂变更后语义可与期望文档稳定对齐。
	const char *source = "{\"src\":[{\"id\":\"a\",\"n\":{\"v\":1}},{\"id\":\"b\",\"n\":{\"v\":2}}],\"map\":{\"hold\":{\"id\":\"x\"}}}";
	const char *expectText =
		"{\"src\":[{\"id\":\"a2\",\"n\":{\"v\":10}}],\"map\":{\"hold\":{\"id\":\"x\"},\"moved\":{\"id\":\"b\",\"n\":{\"v\":2}}}}";
	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "访问链路样本解析失败");

	RyanJson_t srcArr = RyanJsonGetObjectByKeys(root, "src", NULL);
	RyanJson_t mapObj = RyanJsonGetObjectByKeys(root, "map", NULL);
	TEST_ASSERT_NOT_NULL(srcArr);
	TEST_ASSERT_NOT_NULL(mapObj);
	TEST_ASSERT_TRUE(RyanJsonIsArray(srcArr));
	TEST_ASSERT_TRUE(RyanJsonIsObject(mapObj));
	{
		RyanJson_t secondSrc = RyanJsonGetObjectByIndexs(srcArr, 1, UINT32_MAX);
		TEST_ASSERT_NOT_NULL(secondSrc);
		TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(secondSrc, "n", "v")));
	}

	RyanJson_t moved = RyanJsonDetachByIndex(srcArr, 1);
	TEST_ASSERT_NOT_NULL_MESSAGE(moved, "分离 src[1] 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(mapObj, "moved", moved), "挂载 map.moved 失败");

	RyanJson_t newHead = RyanJsonParse("{\"id\":\"a2\",\"n\":{\"v\":10}}");
	TEST_ASSERT_NOT_NULL(newHead);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByIndex(srcArr, 0, newHead), "替换 src[0] 失败");

	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "map", "moved", "id"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByIndex(srcArr, 0));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByIndex(srcArr, 1));
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetStringValue(RyanJsonGetObjectByKeys(root, "map", "moved", "id", NULL)));
	{
		RyanJson_t firstSrc = RyanJsonGetObjectByIndexs(srcArr, 0, UINT32_MAX);
		TEST_ASSERT_NOT_NULL(firstSrc);
		TEST_ASSERT_EQUAL_INT(10, RyanJsonGetIntValue(RyanJsonGetObjectToKey(firstSrc, "n", "v")));
	}

	RyanJson_t expect = RyanJsonParse(expectText);
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, expect), "复杂访问链路结果与期望文档不一致");

	char *printed = RyanJsonPrint(root, 192, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

void testAccessorRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testAccessorContainerEntryAndSiblingTraversal);
	RUN_TEST(testAccessorVarargsLookupSuccessAndFailure);
	RUN_TEST(testAccessorVarargsMacrosOnConstructedTree);
	RUN_TEST(testAccessorVarargsStopOnScalar);
	RUN_TEST(testAccessorTypePredicateMatrix);
	RUN_TEST(testAccessorLookupArgumentGuards);
	RUN_TEST(testAccessorTypedArrayCreateAndRoundtrip);
	RUN_TEST(testAccessorHasMacrosCoverage);
	RUN_TEST(testAccessorNullVsMissingSemantics);
	RUN_TEST(testAccessorNullLeafInNestedObject);
	RUN_TEST(testAccessorHasObjectByIndexForNullElement);
	RUN_TEST(testAccessorVarargsRootTypeMismatch);
	RUN_TEST(testAccessorObjectIndexTraversalCoverage);
	RUN_TEST(testAccessorEmptyContainerEntryValue);
	RUN_TEST(testAccessorGetSizeNullAndScalarRoot);
	RUN_TEST(testAccessorNullMidPathStopsTraversal);
	RUN_TEST(testAccessorGetNextAfterDetachAndInsert);
	RUN_TEST(testAccessorObjectHeadUpdateAfterInsert);
	RUN_TEST(testAccessorArrayHeadUpdateAfterInsert);
	RUN_TEST(testAccessorObjectHeadUpdateAfterDetach);
	RUN_TEST(testAccessorVarargsSentinelBehavior);
	RUN_TEST(testAccessorArrayHeadUpdateAfterDetach);
	RUN_TEST(testAccessorPathConsistencyAfterSequentialMutations);
}
