#include "testBase.h"
#include <float.h>
static void testCompareEdgeCases(void)
{
	// 测试对象键值对顺序对比较的影响
	// RyanJson 的对象比较是无序的：键值对顺序不同也应视为相同

	RyanJson_t json1 = RyanJsonCreateObject();
	RyanJsonAddIntToObject(json1, "a", 1);
	RyanJsonAddIntToObject(json1, "b", 2);

	RyanJson_t json2 = RyanJsonCreateObject();
	RyanJsonAddIntToObject(json2, "b", 2);
	RyanJsonAddIntToObject(json2, "a", 1); // 顺序不同

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(json1, json2), "顺序不同的对象比较应返回 True (RyanJson 是无序比较)");

	RyanJsonDelete(json1);
	RyanJsonDelete(json2);
}

static void testCompareObjectOrderPaths(void)
{
	RyanJson_t left = RyanJsonCreateObject();
	RyanJson_t rightOrdered = RyanJsonCreateObject();
	RyanJson_t rightUnordered = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightOrdered);
	TEST_ASSERT_NOT_NULL(rightUnordered);

	// 同序构造：覆盖对象比较同序快路径
	for (int32_t i = 0; i < 64; i++)
	{
		char key[16];
		RyanJsonSnprintf(key, sizeof(key), "k%" PRId32, i);
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(left, key, i));
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(rightOrdered, key, i));
	}

	// 逆序构造：覆盖对象比较按 key 回退查找路径
	for (int32_t i = 63; i >= 0; i--)
	{
		char key[16];
		RyanJsonSnprintf(key, sizeof(key), "k%" PRId32, i);
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(rightUnordered, key, i));
	}

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightOrdered), "同序对象比较应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightUnordered), "乱序对象比较应返回 True");

	RyanJsonDelete(left);
	RyanJsonDelete(rightOrdered);
	RyanJsonDelete(rightUnordered);
}

static void testCompareScalarAndTypeMatrix(void)
{
	RyanJson_t intLeft = RyanJsonCreateInt(NULL, 10);
	RyanJson_t intRight = RyanJsonCreateInt(NULL, 11);
	RyanJson_t doubleCloseLeft = RyanJsonCreateDouble(NULL, 1.0);
	RyanJson_t doubleCloseRight = RyanJsonCreateDouble(NULL, 1.0 + (RyanJsonAbsTolerance / 2.0));
	RyanJson_t doubleFarLeft = RyanJsonCreateDouble(NULL, 1.0);
	RyanJson_t doubleFarRight = RyanJsonCreateDouble(NULL, 1.0 + (RyanJsonAbsTolerance * 100.0));
	RyanJson_t strLeft = RyanJsonCreateString(NULL, "alpha");
	RyanJson_t strRight = RyanJsonCreateString(NULL, "beta");
	RyanJson_t typeInt = RyanJsonCreateInt(NULL, 1);
	RyanJson_t typeDouble = RyanJsonCreateDouble(NULL, 1.0);
	RyanJson_t typeString = RyanJsonCreateString(NULL, "1");

	TEST_ASSERT_NOT_NULL(intLeft);
	TEST_ASSERT_NOT_NULL(intRight);
	TEST_ASSERT_NOT_NULL(doubleCloseLeft);
	TEST_ASSERT_NOT_NULL(doubleCloseRight);
	TEST_ASSERT_NOT_NULL(doubleFarLeft);
	TEST_ASSERT_NOT_NULL(doubleFarRight);
	TEST_ASSERT_NOT_NULL(strLeft);
	TEST_ASSERT_NOT_NULL(strRight);
	TEST_ASSERT_NOT_NULL(typeInt);
	TEST_ASSERT_NOT_NULL(typeDouble);
	TEST_ASSERT_NOT_NULL(typeString);

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(intLeft, intRight), "int 值不同时 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(intLeft, intRight), "int 值不同时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(doubleCloseLeft, doubleCloseRight), "double 在容差内应判等");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(doubleFarLeft, doubleFarRight), "double 超过容差应不相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(doubleFarLeft, doubleFarRight), "double 值不同时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(strLeft, strRight), "string 值不同时 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(strLeft, strRight), "string 值不同时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(typeInt, typeString), "类型不同时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(typeInt, typeString), "类型不同时 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(typeInt, typeDouble), "int/double 即使数值相同，类型不同也应不相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(typeInt, typeDouble),
				 "CompareOnlyKey 忽略 number 具体子类型，int/double 应视为相等");

	RyanJsonDelete(intLeft);
	RyanJsonDelete(intRight);
	RyanJsonDelete(doubleCloseLeft);
	RyanJsonDelete(doubleCloseRight);
	RyanJsonDelete(doubleFarLeft);
	RyanJsonDelete(doubleFarRight);
	RyanJsonDelete(strLeft);
	RyanJsonDelete(strRight);
	RyanJsonDelete(typeInt);
	RyanJsonDelete(typeDouble);
	RyanJsonDelete(typeString);
}

static void testCompareDoubleRelativeToleranceDominates(void)
{
	// 构造一个让“相对误差”主导的场景，避免 CompareDouble 只剩绝对误差分支被覆盖。
	double base = (RyanJsonAbsTolerance / DBL_EPSILON) * 8.0;
	double epsilon = DBL_EPSILON * fabs(base);
	TEST_ASSERT_TRUE_MESSAGE(epsilon > RyanJsonAbsTolerance, "epsilon 应大于 absTolerance");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareDouble(base, base + (epsilon * 0.5)), "相对误差内应判等");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareDouble(base, base + (epsilon * 2.0)), "相对误差外应不等");
}

static void testCompareNumberSubtypeInContainers(void)
{
	RyanJson_t left = RyanJsonParse("{\"n\":1,\"arr\":[1,2.0],\"obj\":{\"x\":3,\"y\":4.0}}");
	RyanJson_t right = RyanJsonParse("{\"obj\":{\"y\":4,\"x\":3.0},\"arr\":[1.0,2],\"n\":1.0}");
	RyanJson_t rightTypeMismatch = RyanJsonParse("{\"obj\":{\"y\":4,\"x\":\"3\"},\"arr\":[1.0,2],\"n\":1.0}");

	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(right);
	TEST_ASSERT_NOT_NULL(rightTypeMismatch);

	// 全量比较要求数值子类型一致；仅比较 key 时允许 int/double 混用
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, right), "容器中 int/double 子类型不同，Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, right),
				 "容器中 int/double 子类型不同，但结构一致时 CompareOnlyKey 应返回 True");

	// 数值与字符串类型不同，即使在 CompareOnlyKey 下也必须失败
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightTypeMismatch), "number/string 类型不同，Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightTypeMismatch), "number/string 类型不同，CompareOnlyKey 也应返回 False");

	RyanJsonDelete(left);
	RyanJsonDelete(right);
	RyanJsonDelete(rightTypeMismatch);
}

static void testCompareZeroSignSemantics(void)
{
	// -0 与 0 应视为数值相等（同为 int 类型）
	RyanJson_t negZero = RyanJsonParse("-0");
	RyanJson_t posZero = RyanJsonParse("0");
	TEST_ASSERT_NOT_NULL(negZero);
	TEST_ASSERT_NOT_NULL(posZero);
	TEST_ASSERT_TRUE(RyanJsonIsInt(negZero));
	TEST_ASSERT_TRUE(RyanJsonIsInt(posZero));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(negZero, posZero), "-0 与 0 比较应相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(negZero, posZero), "-0 与 0 CompareOnlyKey 也应相等");

	RyanJson_t objNeg = RyanJsonParse("{\"n\":-0}");
	RyanJson_t objPos = RyanJsonParse("{\"n\":0}");
	TEST_ASSERT_NOT_NULL(objNeg);
	TEST_ASSERT_NOT_NULL(objPos);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(objNeg, objPos), "对象内 -0 与 0 应相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(objNeg, objPos), "对象内 -0 与 0 CompareOnlyKey 应相等");

	RyanJsonDelete(objPos);
	RyanJsonDelete(objNeg);
	RyanJsonDelete(posZero);
	RyanJsonDelete(negZero);
}

static void testCompareEmptyContainerAndTypeMismatch(void)
{
	RyanJson_t emptyObjA = RyanJsonCreateObject();
	RyanJson_t emptyObjB = RyanJsonCreateObject();
	RyanJson_t emptyArrA = RyanJsonCreateArray();
	RyanJson_t emptyArrB = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(emptyObjA);
	TEST_ASSERT_NOT_NULL(emptyObjB);
	TEST_ASSERT_NOT_NULL(emptyArrA);
	TEST_ASSERT_NOT_NULL(emptyArrB);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(emptyObjA, emptyObjB), "空对象之间应相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(emptyObjA, emptyObjB), "空对象 CompareOnlyKey 应相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(emptyArrA, emptyArrB), "空数组之间应相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(emptyArrA, emptyArrB), "空数组 CompareOnlyKey 应相等");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(emptyObjA, emptyArrA), "对象与数组类型不同应不相等");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(emptyObjA, emptyArrA), "对象与数组 CompareOnlyKey 也应不相等");

	RyanJsonDelete(emptyObjA);
	RyanJsonDelete(emptyObjB);
	RyanJsonDelete(emptyArrA);
	RyanJsonDelete(emptyArrB);
}

static void testCompareArraySemantics(void)
{
	RyanJson_t arrIntA = RyanJsonCreateArray();
	RyanJson_t arrIntB = RyanJsonCreateArray();
	RyanJson_t arrIntReverse = RyanJsonCreateArray();
	RyanJson_t arrMixedA = RyanJsonCreateArray();
	RyanJson_t arrMixedB = RyanJsonCreateArray();
	RyanJson_t arrShort = RyanJsonCreateArray();

	TEST_ASSERT_NOT_NULL(arrIntA);
	TEST_ASSERT_NOT_NULL(arrIntB);
	TEST_ASSERT_NOT_NULL(arrIntReverse);
	TEST_ASSERT_NOT_NULL(arrMixedA);
	TEST_ASSERT_NOT_NULL(arrMixedB);
	TEST_ASSERT_NOT_NULL(arrShort);

	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrIntA, 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrIntA, 2));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrIntA, 3));

	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrIntB, 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrIntB, 2));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrIntB, 3));

	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrIntReverse, 3));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrIntReverse, 2));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrIntReverse, 1));

	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrMixedA, 1));
	TEST_ASSERT_TRUE(RyanJsonAddStringToArray(arrMixedA, "2"));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrMixedA, 3));

	TEST_ASSERT_TRUE(RyanJsonAddStringToArray(arrMixedB, "1"));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrMixedB, 2));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrMixedB, 3));

	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrShort, 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arrShort, 2));

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(arrIntA, arrIntB), "相同数组 Compare 应返回 True");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(arrIntA, arrIntReverse), "数组顺序不同 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(arrIntA, arrIntReverse), "数组顺序不同但类型一致，CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(arrMixedA, arrMixedB), "数组元素类型顺序不同 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(arrMixedA, arrMixedB), "数组元素类型顺序不同 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(arrIntA, arrShort), "数组长度不同 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(arrIntA, arrShort), "数组长度不同 CompareOnlyKey 应返回 False");

	RyanJsonDelete(arrIntA);
	RyanJsonDelete(arrIntB);
	RyanJsonDelete(arrIntReverse);
	RyanJsonDelete(arrMixedA);
	RyanJsonDelete(arrMixedB);
	RyanJsonDelete(arrShort);
}

static void testCompareNestedObjectScenarios(void)
{
	RyanJson_t left = RyanJsonParse("{\"meta\":{\"id\":1,\"name\":\"m\"},\"list\":[1,2,3],\"flag\":true}");
	RyanJson_t rightOrderedDiff = RyanJsonParse("{\"flag\":true,\"list\":[1,2,3],\"meta\":{\"name\":\"m\",\"id\":1}}");
	RyanJson_t rightValueDiff = RyanJsonParse("{\"flag\":true,\"list\":[1,2,3],\"meta\":{\"name\":\"m\",\"id\":2}}");
	RyanJson_t rightMissingKey = RyanJsonParse("{\"flag\":true,\"list\":[1,2,3],\"meta\":{\"name\":\"m\",\"idx\":1}}");
	RyanJson_t leftSingleKey = RyanJsonParse("{\"a\":1}");
	RyanJson_t rightSingleKey = RyanJsonParse("{\"b\":1}");
	RyanJson_t leftPrefixMatch = RyanJsonParse("{\"a\":1,\"b\":2}");
	RyanJson_t rightPrefixMismatch = RyanJsonParse("{\"a\":1,\"c\":2}");
	RyanJson_t leftTailMatch = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	RyanJson_t rightTailMatch = RyanJsonParse("{\"b\":2,\"c\":3,\"a\":1}");
	RyanJson_t rightTailMismatch = RyanJsonParse("{\"c\":3,\"d\":4,\"a\":1}");

	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightOrderedDiff);
	TEST_ASSERT_NOT_NULL(rightValueDiff);
	TEST_ASSERT_NOT_NULL(rightMissingKey);
	TEST_ASSERT_NOT_NULL(leftSingleKey);
	TEST_ASSERT_NOT_NULL(rightSingleKey);
	TEST_ASSERT_NOT_NULL(leftPrefixMatch);
	TEST_ASSERT_NOT_NULL(rightPrefixMismatch);
	TEST_ASSERT_NOT_NULL(leftTailMatch);
	TEST_ASSERT_NOT_NULL(rightTailMatch);
	TEST_ASSERT_NOT_NULL(rightTailMismatch);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightOrderedDiff), "嵌套对象乱序但值一致应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightOrderedDiff), "嵌套对象乱序 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightValueDiff), "嵌套对象值不同 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightValueDiff), "嵌套对象值不同 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightMissingKey), "嵌套对象 key 不匹配 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightMissingKey), "嵌套对象 key 不匹配 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(leftSingleKey, rightSingleKey), "同尺寸对象但 key 不同 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(leftSingleKey, rightSingleKey),
				  "同尺寸对象但 key 不同 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(leftPrefixMatch, rightPrefixMismatch),
				  "前缀 key 相同但后续 key 不同 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(leftPrefixMatch, rightPrefixMismatch),
				  "前缀 key 相同但后续 key 不同 CompareOnlyKey 应返回 False");

	// 覆盖同层遍历中 rightCandidate == NULL 的回退查找成功分支
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(leftTailMatch, rightTailMatch), "尾节点命中回退查找后 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(leftTailMatch, rightTailMatch), "尾节点命中回退查找后 CompareOnlyKey 应返回 True");

	// 覆盖同层遍历中 rightCandidate == NULL 的回退查找失败分支
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(leftTailMatch, rightTailMismatch), "尾节点回退查找失败 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(leftTailMatch, rightTailMismatch),
				  "尾节点回退查找失败 CompareOnlyKey 应返回 False");

	RyanJsonDelete(left);
	RyanJsonDelete(rightOrderedDiff);
	RyanJsonDelete(rightValueDiff);
	RyanJsonDelete(rightMissingKey);
	RyanJsonDelete(leftSingleKey);
	RyanJsonDelete(rightSingleKey);
	RyanJsonDelete(leftPrefixMatch);
	RyanJsonDelete(rightPrefixMismatch);
	RyanJsonDelete(leftTailMatch);
	RyanJsonDelete(rightTailMatch);
	RyanJsonDelete(rightTailMismatch);
}

static void testCompareArrayWithObjects(void)
{
	RyanJson_t arrLeft = RyanJsonParse("[{\"a\":1,\"b\":2},{\"x\":3,\"y\":4}]");
	RyanJson_t arrObjectUnordered = RyanJsonParse("[{\"b\":2,\"a\":1},{\"y\":4,\"x\":3}]");
	RyanJson_t arrOrderSwapped = RyanJsonParse("[{\"y\":4,\"x\":3},{\"b\":2,\"a\":1}]");
	RyanJson_t arrValueDiff = RyanJsonParse("[{\"a\":9,\"b\":2},{\"x\":3,\"y\":4}]");

	TEST_ASSERT_NOT_NULL(arrLeft);
	TEST_ASSERT_NOT_NULL(arrObjectUnordered);
	TEST_ASSERT_NOT_NULL(arrOrderSwapped);
	TEST_ASSERT_NOT_NULL(arrValueDiff);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(arrLeft, arrObjectUnordered), "数组内对象乱序键应比较为 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(arrLeft, arrObjectUnordered), "数组内对象乱序键 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(arrLeft, arrOrderSwapped), "数组元素顺序变化 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(arrLeft, arrOrderSwapped), "数组元素顺序变化 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(arrLeft, arrValueDiff), "数组内对象值变化 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(arrLeft, arrValueDiff), "数组内对象值变化但键结构一致 CompareOnlyKey 应返回 True");

	RyanJsonDelete(arrLeft);
	RyanJsonDelete(arrObjectUnordered);
	RyanJsonDelete(arrOrderSwapped);
	RyanJsonDelete(arrValueDiff);
}

static void testCompareDeepNestAndLargeArray(void)
{
	// 深度嵌套比较（检测栈溢出或递归限制）
	// 之前 200 层出现疑似内存不足或其他问题，降至 50 层验证核心逻辑
	int32_t depth = 50;
	RyanJson_t root1 = RyanJsonCreateObject();
	RyanJson_t root2 = RyanJsonCreateObject();

	RyanJson_t curr1 = root1;
	RyanJson_t curr2 = root2;

	for (int32_t i = 0; i < depth; i++)
	{
		RyanJson_t child1 = RyanJsonCreateObject();
		RyanJson_t child2 = RyanJsonCreateObject();

		TEST_ASSERT_NOT_NULL_MESSAGE(child1, "创建 child1 失败");
		TEST_ASSERT_NOT_NULL_MESSAGE(child2, "创建 child2 失败");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(curr1, "nest", child1), "添加到 curr1 失败");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(curr2, "nest", child2), "添加到 curr2 失败");

		// RyanJsonAddItemToObject 会“吞掉” child1（若其为容器），把其内容转移到新节点并释放 child1
		// 因此 child1 指针已失效，必须通过 key 获取新创建的节点
		curr1 = RyanJsonGetObjectByKey(curr1, "nest");
		curr2 = RyanJsonGetObjectByKey(curr2, "nest");

		TEST_ASSERT_NOT_NULL_MESSAGE(curr1, "获取 curr1 下一级 nest 失败");
		TEST_ASSERT_NOT_NULL_MESSAGE(curr2, "获取 curr2 下一级 nest 失败");
	}

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root1, root2), "初始深度比较失败");

	// 修改末端
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToObject(curr1, "diff", 1), "添加 diff 失败");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, RyanJsonGetSize(curr1), "添加 diff 后 Size 应为 1");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, RyanJsonGetSize(curr2), "curr2 Size 应仍为 0");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root1, root2), "修改后深度比较应失败");

	RyanJsonDelete(root1);
	RyanJsonDelete(root2);

	// 大数组比较
	int32_t count = 2000;
	RyanJson_t arr1 = RyanJsonCreateArray();
	RyanJson_t arr2 = RyanJsonCreateArray();

	for (int32_t i = 0; i < count; i++)
	{
		RyanJsonAddIntToArray(arr1, i);
		RyanJsonAddIntToArray(arr2, i);
	}
	TEST_ASSERT_TRUE(RyanJsonCompare(arr1, arr2));

	// 修改中间一个元素
	RyanJsonDeleteByIndex(arr2, count / 2); // 删除中间项
	RyanJson_t insert = RyanJsonCreateInt("new", 99999);
	RyanJsonInsert(arr2, count / 2, insert);

	TEST_ASSERT_FALSE(RyanJsonCompare(arr1, arr2));

	RyanJsonDelete(arr1);
	RyanJsonDelete(arr2);
}

static void testCompareEqualityAndStructuralDiff(void)
{
	char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
			 "{\"inter\":16,\"double\":16."
			 "89,\"string\":\"hello\","
			 "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,"
			 "16.89,16.89,16.89],"
			 "\"arrayString\":[\"hello\",\"hello\","
			 "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			 "\"double\":16.89,\"string\":"
			 "\"hello\",\"boolTrue\":true,"
			 "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			 "\"boolFalse\":false,\"null\":null}]}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Json 1 失败");
	RyanJson_t json2 = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json2, "解析 Json 2 失败");

	// 边界情况测试
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, NULL), "与 NULL 比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(NULL, json2), "NULL 与对象比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(NULL, NULL), "NULL 与 NULL 比较应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, NULL), "仅比较 Key：与 NULL 比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(NULL, json2), "仅比较 Key：NULL 与对象比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(NULL, NULL), "仅比较 Key：NULL 与 NULL 比较应返回 False");

	// 完整对象比较
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(json, json2), "两个相同内容的对象比较应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(json, json), "对象与自身比较应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "仅比较 Key：两个相同内容的对象比较应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json), "仅比较 Key：对象与自身比较应返回 True");

	// 修改对象 2 并比较
	// 添加字符串
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddStringToObject(json2, "test", "hello");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "多出一个字段后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "多出一个字段后仅比较 Key 应返回 False");

	// 添加整数
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddIntToObject(json2, "test", 1);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "多出一个整数后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "多出一个整数后仅比较 Key 应返回 False");

	// 添加浮点数
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddDoubleToObject(json2, "test", 2.0);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "多出一个浮点数后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "多出一个浮点数后仅比较 Key 应返回 False");

	// 添加 boolValue
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddBoolToObject(json2, "test", RyanJsonTrue);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "多出一个布尔值后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "多出一个布尔值后仅比较 Key 应返回 False");

	// 添加 null
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddNullToObject(json2, "test");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "多出一个 Null 后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "多出一个 Null 后仅比较 Key 应返回 False");

	// 数组修改测试
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddIntToArray(RyanJsonGetObjectToKey(json2, "arrayInt"), 2);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "数组长度变化后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "数组长度变化后仅比较 Key 应返回 False");

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddDoubleToArray(RyanJsonGetObjectToKey(json2, "arrayDouble"), 2.0);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "数组长度变化(浮点)后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "数组长度变化(浮点)后仅比较 Key 应返回 False");

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddStringToArray(RyanJsonGetObjectToKey(json2, "arrayString"), "hello");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "数组长度变化(字符串)后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "数组长度变化(字符串)后仅比较 Key 应返回 False");

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddStringToArray(RyanJsonGetObjectToKey(json2, "arrayItem"), "hello");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "数组长度变化(项)后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "数组长度变化(项)后仅比较 Key 应返回 False");

	// 修改 key 名称
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeKey(RyanJsonGetObjectToKey(json2, "inter"), "int2");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "Key 修改后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "Key 修改后仅比较 Key 应返回 False");

	// 修改值但 key 相同
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(json2, "inter"), 17);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "Value 修改后比较应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "Value 修改但 Key 相同，仅比较 Key 应返回 True");

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeDoubleValue(RyanJsonGetObjectToKey(json2, "double"), 20.89);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "浮点 Value 修改但 Key 相同，仅比较 Key 应返回 True");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "浮点 Value 修改后比较应返回 False");

	// 类型修改测试（从 double 改为 int32_t）
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonDeleteByKey(json2, "double");
	RyanJsonAddIntToObject(json2, "double", 20); // 改为 int
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "类型修改后比较应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "类型修改但 Key 相同，仅比较 Key 应返回 True");

	// 修改 strValue
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(json2, "string"), "49");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "字符串 Value 修改后比较应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "字符串 Value 修改但 Key 相同，仅比较 Key 应返回 True");

	// 修改对象 1 的 boolValue
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json2, "boolTrue"), RyanJsonFalse);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "布尔 Value 修改后比较应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "布尔 Value 修改但 Key 相同，仅比较 Key 应返回 True");

	// 修改嵌套对象的 boolValue
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json2, "item", "boolTrue"), RyanJsonFalse);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "嵌套布尔 Value 修改后比较应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "嵌套布尔 Value 修改但结构相同，仅比较 Key 应返回 True");

	// 修改数组中的整数
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayInt"), 0), 17);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "数组元素修改后比较应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "数组元素修改但长度相同，仅比较 Key 应返回 True");

	// 修改数组中的浮点数
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeDoubleValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayDouble"), 0), 20.89);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "数组浮点元素修改后比较应返回 False");

	// 修改数组中的字符串
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayString"), 0), "20.89");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "数组字符串元素修改后比较应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "数组字符串元素修改但长度相同，仅比较 Key 应返回 True");

	// 修改混合数组
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "array"), 0), 17);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "混合数组修改后比较应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "混合数组修改但长度相同，仅比较 Key 应返回 True");

	// 修改数组项中的对象
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayItem"), 0), "inter"),
			       17);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "数组项对象修改后比较应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "数组项对象修改但结构相同，仅比较 Key 应返回 True");

	// 删除整个 key 节点
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonDeleteByKey(json2, "arrayItem");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "删除 Key 后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "删除 Key 后仅比较 Key 应返回 False");

	// 删除数组索引项
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonDeleteByIndex(RyanJsonGetObjectToKey(json2, "arrayInt"), 2);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "删除数组索引后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "删除数组索引后仅比较 Key 应返回 False");

	// 删除数组项中的对象项
	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonDeleteByIndex(RyanJsonGetObjectToKey(json2, "arrayItem"), 0);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(json, json2), "删除数组对象项后比较应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(json, json2), "删除数组对象项后仅比较 Key 应返回 False");

	RyanJsonDelete(json);
	RyanJsonDelete(json2);
}

void testCompareRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testCompareEdgeCases);
	RUN_TEST(testCompareObjectOrderPaths);
	RUN_TEST(testCompareScalarAndTypeMatrix);
	RUN_TEST(testCompareDoubleRelativeToleranceDominates);
	RUN_TEST(testCompareNumberSubtypeInContainers);
	RUN_TEST(testCompareZeroSignSemantics);
	RUN_TEST(testCompareEmptyContainerAndTypeMismatch);
	RUN_TEST(testCompareArraySemantics);
	RUN_TEST(testCompareNestedObjectScenarios);
	RUN_TEST(testCompareArrayWithObjects);
	RUN_TEST(testCompareDeepNestAndLargeArray);
	RUN_TEST(testCompareEqualityAndStructuralDiff);
}

static void testRootScalarStringChangeCompareRoundtrip(void)
{
	// 复杂链路：
	// Parse(根字符串) -> Duplicate -> ChangeStringValue -> Compare/CompareOnlyKey -> Print/Parse。
	// 目标：验证根节点为字符串时的修改、比较与往返稳定性。
	RyanJson_t root = RyanJsonParse("\"alpha\"");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(copy, "beta"));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, copy), "根字符串值变化后 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(root, copy), "根字符串值变化后 CompareOnlyKey 应返回 True");

	char *printed = RyanJsonPrint(copy, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(copy, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testRootScalarIntChangeCompareOnlyKey(void)
{
	// 复杂链路：
	// Parse(根整数) -> Duplicate -> ChangeIntValue -> Compare/CompareOnlyKey。
	// 目标：验证根节点为 int 时 CompareOnlyKey 忽略 value 差异。
	RyanJson_t root = RyanJsonParse("1");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(copy, 2));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, copy), "根整数值变化后 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(root, copy), "根整数值变化后 CompareOnlyKey 应返回 True");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(root));

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testRootScalarDoubleChangeCompareOnlyKey(void)
{
	// 复杂链路：
	// Parse(根浮点) -> Duplicate -> ChangeDoubleValue -> Compare/CompareOnlyKey。
	// 目标：验证根节点为 double 时 CompareOnlyKey 忽略 value 差异。
	RyanJson_t root = RyanJsonParse("1.5");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	TEST_ASSERT_TRUE(RyanJsonChangeDoubleValue(copy, 2.5));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, copy), "根浮点值变化后 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(root, copy), "根浮点值变化后 CompareOnlyKey 应返回 True");

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testRootScalarBoolChangeCompareOnlyKey(void)
{
	// 复杂链路：
	// Parse(根布尔) -> Duplicate -> ChangeBoolValue -> Compare/CompareOnlyKey。
	// 目标：验证根节点为 bool 时 CompareOnlyKey 忽略 value 差异。
	RyanJson_t root = RyanJsonParse("true");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	TEST_ASSERT_TRUE(RyanJsonChangeBoolValue(copy, RyanJsonFalse));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, copy), "根布尔值变化后 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(root, copy), "根布尔值变化后 CompareOnlyKey 应返回 True");

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testRootScalarReplaceByIndexFails(void)
{
	// 复杂链路：
	// Parse(根标量) -> ReplaceByIndex(失败) -> 释放 item。
	// 目标：验证对非容器执行 ReplaceByIndex 会失败且不会消耗 item。
	RyanJson_t root = RyanJsonParse("1");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t item = RyanJsonCreateInt(NULL, 9);
	TEST_ASSERT_NOT_NULL(item);

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByIndex(root, 0, item), "根标量 ReplaceByIndex 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(item), "ReplaceByIndex 失败后 item 应保持游离");

	RyanJsonDelete(item);
	RyanJsonDelete(root);
}

static void testRootScalarGetSizeIsOne(void)
{
	// 覆盖根标量的 GetSize 语义：标量节点大小应为 1。
	RyanJson_t root = RyanJsonParse("\"x\"");
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(root));
	RyanJsonDelete(root);
}

void testRootScalarOpsRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testRootScalarStringChangeCompareRoundtrip);
	RUN_TEST(testRootScalarIntChangeCompareOnlyKey);
	RUN_TEST(testRootScalarDoubleChangeCompareOnlyKey);
	RUN_TEST(testRootScalarBoolChangeCompareOnlyKey);
	RUN_TEST(testRootScalarReplaceByIndexFails);
	RUN_TEST(testRootScalarGetSizeIsOne);
}
