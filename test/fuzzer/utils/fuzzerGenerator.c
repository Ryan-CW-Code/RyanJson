#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief 随机节点生成器
 *
 * 用于生成随机 RyanJson 节点，服务于 Create/Insert/Replace 等测试场景。
 * 会覆盖基础类型（Bool/Null/Int/Double/String）与复合类型（Object/Array）。
 *
 * @param pJson 父节点上下文（当前未使用，保留扩展位）
 * @return RyanJson_t 生成的新节点
 */
RyanJson_t RyanJsonFuzzerCreateRandomNodeWithKey(RyanJson_t pJson, const char *key)
{
	(void)pJson;
	RyanJson_t item = NULL;
	uint32_t randomVal = RyanJsonFuzzerNextRand();

	switch (randomVal % 8)
	{
	case 0: item = RyanJsonCreateBool(key, RyanJsonTrue); break;
	case 1: item = RyanJsonCreateBool(key, RyanJsonFalse); break;
	case 2: item = RyanJsonCreateNull(key); break;
	case 3: item = RyanJsonCreateInt(key, (int32_t)RyanJsonFuzzerNextRand()); break;
	case 4: item = RyanJsonCreateDouble(key, 1.0 * RyanJsonFuzzerNextRand() / 1000.0); break;
	case 5:
		item = RyanJsonCreateString(key, "random_string"); // 当前使用固定 String，保证可重复性
		break;
	case 6:
		item = RyanJsonCreateObject();
		if (key) { RyanJsonChangeKey(item, key); } // CreateObject 不接收 key 参数
		break;
	case 7:
		item = RyanJsonCreateArray();
		if (key) { RyanJsonChangeKey(item, key); } // CreateArray 不接收 key 参数
		break;
	}

	return item;
}

RyanJson_t RyanJsonFuzzerCreateRandomNode(RyanJson_t pJson)
{
	return RyanJsonFuzzerCreateRandomNodeWithKey(pJson, NULL);
}
