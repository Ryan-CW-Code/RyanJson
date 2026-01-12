# RyanJson
### *希望有兴趣的大佬多试试，找找bug、提提意见*

📢 **使用过程中遇到问题？欢迎提交 Issue 或在 [RT-Thread 社区](https://club.rt-thread.org/index.html) 提问，感谢支持！**

***一个针对资源受限的嵌入式设备优化的Json库，内存占用极小的通用Json库，简洁高效！***

***示例代码请参考`example`文件夹！***

### 1、介绍

**RyanJson** 是一个针对嵌入式平台深度优化的 C 语言 JSON 解析器。它在保持代码健壮性的同时，实现了极致的内存控制，旨在解决 cJSON 等传统库在复杂场景下内存占用过高的问题。

*初衷：项目重构后 JSON 结构复杂度提升，cJSON 内存占用过高，无法满足嵌入式场景需求。*

#### ✅ 特性亮点

- 💡 **极致内存优化：** 通过动态内存扩展与紧凑结构设计，相比 cJSON 减少 **50% 左右的内存占用**。
- 🔍 **模糊测试保障：** 基于[LLVM Fuzzer](https://llvm.org/docs/LibFuzzer.html) 生成上亿级测试用例，**分支覆盖率 100%**，确保非法输入和极端场景下依旧安全。**[点击在线查看覆盖率信息](https://ryan-cw-code.github.io/RyanJson/)** 
- 🧪 **9 大类专项测试用例：** 覆盖广泛场景，全链路内存泄漏检测，强化稳定性与可靠性
- 🛡️ **运行时安全分析验证：** 使用 **[Sanitizer](https://clang.llvm.org/docs/index.html#sanitizers)** 系列工具，捕获内存越界、Use-after-free、数据竞争、未定义行为、内存泄漏等问题，提升代码健壮性与安全性
- 📐**高质量代码保障：**  引入 **[clang-tidy](https://clang.llvm.org/extra/clang-tidy/#clang-tidy)** 与 **[Cppcheck](https://cppcheck.sourceforge.io/)** 进行静态分析，代码质量接近语法级的"**零缺陷**"
- 🤖  **AI 辅助开发与审查：** 结合  **[Gemini Code Assist](https://codeassist.google/)** 、**[coderabbitai](https://www.coderabbit.ai)** 、 **[Copilot](https://github.com/features/copilot)** ，在编码与代码审查阶段持续优化代码质量，构建多层安全防线
- 👩‍💻 **开发者友好：** 类 cJSON 接口设计，迁移成本低
- 📜 **严格但不严苛：** 符合  **[RFC 8259](https://github.com/nst/JSONTestSuite)** 绝大部分标准，支持无限嵌套（受限于栈空间），支持注释与尾随逗号（可配置）

### 2、设计

**RyanJson设计时借鉴了 [json](https://api.gitee.com/Lamdonn/json) 和 [cJSON](https://github.com/DaveGamble/cJSON) !**  

Json语法是**JavaScript**对象语法的子集，可通过下面两个连接学习json语法。

[JSON规范](https://www.json.org/json-en.html)

[Parsing JSON is a Minefield 建议看看](https://seriot.ch/projects/parsing_json.html)

RyanJson 的核心在于对内存布局的精细控制，**结构体表示最小存储单元（键值对）**，通过单链表组织数据，结构如下：

```c
// Json 的最基础节点，所有 Json 元素都由该节点表示。
// 结构体中仅包含固定的 next 指针，用于单向链表串联。
// 其余数据（flag、key、stringValue、numberValue、doubleValue 等）均通过动态内存分配管理。
struct RyanJsonNode
{
	struct RyanJsonNode *next; // 单链表节点指针

	/**
	 * @brief RyanJson 节点结构体
	 * 每个节点由链表连接，包含元数据标识 (Flag) 与动态载荷存储区。
	 *
	 * 内存布局：
	 * [ next指针 | flag(1字节) | padding/指针空间 | 动态载荷区 ]
	 *
	 * @brief 节点元数据标识 (Flag)
	 * 紧跟 next 指针后，利用 1 字节位域描述节点类型及存储状态。
	 *
	 * flag 位分布定义：
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * -----------------------------------------------------
	 * strMode KeyLen KeyLen HasKey NumExt Type2  Type1  Type0
	 *
	 * 各位含义：
	 * - bit0-2 : 节点类型
	 *            000=Unknown, 001=Null, 010=Bool, 011=Number,
	 *            100=String, 101=Array, 110=Object, 111=Reserved
	 *
	 * - bit3   : 扩展位
	 *            Bool 类型：0=false, 1=true
	 *            Number 类型：0=int(4字节), 1=double(8字节)
	 *
	 * - bit4   : 是否包含 Key
	 *            0=无 Key（数组元素）
	 *            1=有 Key（对象成员）
	 *
	 * - bit5-6 : Key 长度字段字节数
	 *            00=1字节 (≤UINT8_MAX)
	 *            01=2字节 (≤UINT16_MAX)
	 *            10=3字节 (≤UINT24_MAX)
	 *            11=4字节 (≤UINT32_MAX)
	 *
	 * - bit7   : 表示key / strValue 存储模式
	 *            0:inline 模式, 1=ptr 模式
	 *
	 * @brief 动态载荷存储区
     * 目的：
     * - 在保持 API 易用性和稳定性的同时，最大限度减少 malloc 调用次数。
     * - 尤其在嵌入式平台，malloc 代价高昂：不仅有堆头部空间浪费，还会产生内存碎片。
     * - 通过利用结构体内的对齐填充 (Padding) 和指针空间，形成一个灵活的缓冲区。
     *
     * 存储策略：
     * 利用结构体内存对齐产生的 Padding（如 Flag 后的空隙）以及原本用于存储指针的空间，形成一个缓冲区
     * 若节点包含 key / strValue，则可能有两种方案：
     * 1. inline 模式 (小数据优化)
     *    - 当 (KeyLen + Key + Value) 的总长度 ≤ 阈值时，直接存储在结构体内部。
     *    - 阈值计算公式：
     *        阈值 = Padding + sizeof(void*) + (malloc头部空间的一半)，再向上对齐到字节边界。
     *      举例：
     *        - 内存对齐：4字节
     *        - malloc头部空间：8字节
     *        - 可用空间 = 3 (flag后padding) + 4 (指针空间) + 4 (malloc头部一半)
     *        - 向上对齐后得到阈值12字节
     *    - 存储布局：
     *        [ KeyLen | Key | Value ]
     *      起始地址即为 flag 之后，数据紧凑排列，无需额外 malloc。
     *
     * 2. ptr 模式 (大数据)
     *    - 当数据长度 > 阈值时，结构体存储一个指针，指向独立的堆区。
     *    - 存储布局：
     *        [ KeyLen | *ptr ] -> (ptr指向) [ Key | Value ]
     *    - KeyLen 的大小由 flag 中的长度字段决定 (最多 4 字节)。
     *    - 这样保证大数据不会撑爆结构体，同时保持 API 一致性。

     * 其他类型的存储：
	 * - null / bool : 由 flag 位直接表示，无需额外空间。
	 * - number      : 根据 flag 扩展位决定存储 int(4字节) 或 double(8字节)。
	 * - object      : 动态分配空间存储子节点，采用链表结构。
     *
     * 设计考量：
     * - malloc 在嵌入式平台的开销：
     *    * RTT 最小内存管理算法中，malloc 头部约 12 字节(可以考虑tlsf算法头部空间仅4字节，内存碎片也控制的很好，适合物联网应用)。
     *    * 一个 RyanJson 节点本身可能只有个位数字节，头部空间就让内存占用翻倍。
     * - 因此：
     *    * 小数据尽量 inline 存储，避免二次 malloc。
     *    * 大数据 fallback 到 ptr 模式，保证灵活性。
     * - 修改场景：
     *    * 理想情况：节点结构体后面直接跟 key/strValue，修改时释放并重新申请节点。
     *    * 但这样 changKey/changStrValue 接口改动太大，用户层需要修改指针，代价高。
     *    * 实际策略：提供就地修改接口。
     *        - 若新值长度 ≤ 原有 inline 缓冲区，直接覆盖。
     *        - 若超过阈值，自动切换到 ptr 模式，用户层无需关心。
	 *
     * 链表结构示例：
     *   {
     *       "name": "RyanJson",
     *   next (
     *       "version": "xxx",
     *   next (
     *       "repository": "https://github.com/Ryan-CW-Code/RyanJson",
     *   next (
     *       "keywords": [
     *           "json",
     *       next (
     *           "streamlined",
     *       next (
     *           "parser"
     *       ))
     *       ],
     *   next (
     *       "others": { ... }
     *   }
	 */
};

typedef struct RyanJsonNode *RyanJson_t;
```

### 3、测试与质量保障

#### 🧪 专项基础功能测试

| 测试类别                 | 测试目标                                                |
| ------------------------ | ------------------------------------------------------- |
| **修改 Json 节点测试**   | 验证字段动态更新及存储模式（Inline/Ptr）的自动切换逻辑  |
| **比较 Json 节点测试**   | 验证节点及其属性的递归深度一致性比较与逻辑等价性        |
| **创建 Json 节点树测试** | 验证全类型节点的构造初始化及深层嵌套结构的正确性        |
| **删除 Json 节点测试**   | 验证节点及其子项的递归内存回收机制，防止内存泄漏        |
| **分离 Json 节点测试**   | 验证节点的分离操作、所属权转移及引用关系的生命周期管理  |
| **复制 Json 节点测试**   | 验证对象深拷贝 (Deep Copy) 的数据完整性与拓扑结构一致性 |
| **Json 循环测试**        | 验证数组/对象迭代器在不同数据规模下的稳定性与边界行为   |
| **文本解析 Json 测试**   | 验证复杂 JSON 文本解析的健壮性及内存映射的准确性        |
| **替换 Json 节点测试**   | 验证成员节点就地替换时的内存复用策略与逻辑一致性        |

#### 🔍**LLVM Fuzzer 模糊测试**（核心亮点）

LLVM Fuzzing 模糊测试是 RyanJson 的 **核心稳定性保障**。

**[点击在线查看覆盖率信息](https://ryan-cw-code.github.io/RyanJson/)** 

- **上亿级测试样本**：[LLVM Fuzzer](https://llvm.org/docs/LibFuzzer.html)  自动生成并执行上亿级随机与非法 JSON 输入
- **覆盖率极高**：**分支覆盖率 100%**（希望以后也能保持），无崩溃、无泄漏
- **鲁棒性验证**：内存申请失败、扩容失败、非法转义字符、尾随逗号、嵌套过深、随机类型切换
- **内存安全验证**：结合 **[Sanitizer](https://clang.llvm.org/docs/index.html#sanitizers)** 工具链，确保无泄漏、无悬空指针、无越界

| 测试类别                   | 测试目标                                                  |
| -------------------------- | --------------------------------------------------------- |
| **内存故障模拟测试**       | 验证在堆内存耗尽 (OOM) 场景下的异常回滚及系统稳定性       |
| **随机解析鲁棒性测试**     | 验证对非法语法、畸形字符及极端输入的容错与边界防御能力    |
| **循环遍历删除安全性测试** | 验证链表迭代过程中动态删除节点的双向一致性与指针安全性    |
| **循环遍历分离安全性测试** | 验证在高频率迭代中分离节点后的拓扑重构与内存权属逻辑      |
| **转义序列极致压缩测试**   | 验证复杂及异常转义字符在高效压缩过程中的解析完整性        |
| **序列化回环一致性测试**   | 验证 JSON 对象经过“解析-打印-解析”链路后数据的不失真性    |
| **高频迭代值读取测试**     | 验证在不同层级结构下随机访问 Value 字段的寻址效率与稳定性 |
| **随机压力复制安全测试**   | 验证大规模深拷贝过程中内存池的利用效率与拓扑结构安全性    |
| **动态类型切换压力测试**   | 验证节点在运行期进行类型强制转换与动态扩展时的内存安全性  |

#### 🛡️ 工具链全面集成

| 工具                                                         | 用途                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| **[Sanitizer](https://clang.llvm.org/docs/index.html#sanitizers)** | 运行时检测，捕获内存泄漏、越界、数据竞争。杜绝泄漏、越界、悬空指针 |
| **[clang-tidy](https://clang.llvm.org/extra/clang-tidy/#clang-tidy)** | 静态分析潜在缺陷（空指针、资源泄漏等）                       |
| **[Cppcheck](https://cppcheck.sourceforge.io/)**             | 深度扫描内存与资源问题                                       |
| **[ClangFormat](https://clang.llvm.org/docs/ClangFormat.html)** | 统一代码风格                                                 |
| AI 审查                                                      | **[Gemini Code Assist](https://codeassist.google/)** 、**[coderabbitai](https://www.coderabbit.ai)** 、 **[Copilot](https://github.com/features/copilot)** 辅助优化逻辑，构建多层安全防线 |
| **编译器警告**                                               | `-Wall -Wextra`（默认）、`-Weffc++`/`-Weverything`（Clang 可选，CI 强化时开启） |

### 4、基准测试

*测试代码和示例代码可在本项目根目录 `test` 和 `RyanJsonExample` 文件夹查看。*

#### 性能测试

**请移步文末的 RyanDocs 文档中心查看，是基于 [yyjson_benchmark](https://github.com/ibireme/yyjson_benchmark) 的测试结果**。（已经过时，仅供参考）


#### 内存占用测试
(20251222 **malloc头部空间4字节，内存对齐4字节**)测试代码可在本项目根目录`RyanJsonExample`文件夹查看。

```
*****************************************************************************
*************************** RyanJson / cJSON / yyjson 内存对比程序 **************************
*****************************************************************************
┌── [TEST 1 | test/RyanJsonMemoryFootprintTest.c:294] 开始执行: testMixedJsonMemory()
json原始文本长度为 2265, 序列化后RyanJson内存占用: 4912, cJSON内存占用: 11336, yyjson内存占用: 8784
比cJSON节省: 56.67% 内存占用, 比yyjson节省: 44.08% 内存占用
└── [TEST 1 | test/RyanJsonMemoryFootprintTest.c:294] 结束执行: 结果 ✅ | 耗时: 0 ms

┌── [TEST 2 | test/RyanJsonMemoryFootprintTest.c:295] 开始执行: testObjectJsonMemory()
json原始文本长度为 3991, 序列化后RyanJson内存占用: 7944, cJSON内存占用: 16020, yyjson内存占用: 12884
比cJSON节省: 50.41% 内存占用, 比yyjson节省: 38.34% 内存占用
└── [TEST 2 | test/RyanJsonMemoryFootprintTest.c:295] 结束执行: 结果 ✅ | 耗时: 1 ms

┌── [TEST 3 | test/RyanJsonMemoryFootprintTest.c:296] 开始执行: testArrayJsonMemory()
json原始文本长度为 1205, 序列化后RyanJson内存占用: 3696, cJSON内存占用: 8680, yyjson内存占用: 5068
比cJSON节省: 57.42% 内存占用, 比yyjson节省: 27.07% 内存占用
└── [TEST 3 | test/RyanJsonMemoryFootprintTest.c:296] 结束执行: 结果 ✅ | 耗时: 0 ms

┌── [TEST 4 | test/RyanJsonMemoryFootprintTest.c:297] 开始执行: testSmallMixedJsonMemory()
json原始文本长度为 90, 序列化后RyanJson内存占用: 168, cJSON内存占用: 392, yyjson内存占用: 648
比cJSON节省: 57.14% 内存占用, 比yyjson节省: 74.07% 内存占用
└── [TEST 4 | test/RyanJsonMemoryFootprintTest.c:297] 结束执行: 结果 ✅ | 耗时: 0 ms

┌── [TEST 5 | test/RyanJsonMemoryFootprintTest.c:298] 开始执行: testSmallStringJsonMemory()
json原始文本长度为 100, 序列化后RyanJson内存占用: 216, cJSON内存占用: 472, yyjson内存占用: 648
比cJSON节省: 54.24% 内存占用, 比yyjson节省: 66.67% 内存占用
└── [TEST 5 | test/RyanJsonMemoryFootprintTest.c:298] 结束执行: 结果 ✅ | 耗时: 0 ms

┌── [TEST 6 | test/RyanJsonMemoryFootprintTest.c:299] 开始执行: testCompressedBusinessJsonMemory()
json原始文本长度为 551, 序列化后RyanJson内存占用: 1184, cJSON内存占用: 3788, yyjson内存占用: 3020
比cJSON节省: 68.74% 内存占用, 比yyjson节省: 60.79% 内存占用
└── [TEST 6 | test/RyanJsonMemoryFootprintTest.c:299] 结束执行: 结果 ✅ | 耗时: 0 ms
```

RT-Thread平台使用最小内存算法默认 **malloc头部空间12字节，内存对齐8字节**测试代码可在本项目根目录`RyanJsonExample`文件夹查看

```
*****************************************************************************
*************************** RyanJson / cJSON / yyjson 内存对比程序 **************************
*****************************************************************************
┌── [TEST 1 | test/RyanJsonMemoryFootprintTest.c:294] 开始执行: testMixedJsonMemory()
json原始文本长度为 2265, 序列化后RyanJson内存占用: 7292, cJSON内存占用: 14948, yyjson内存占用: 8852
比cJSON节省: 51.22% 内存占用, 比yyjson节省: 17.62% 内存占用
└── [TEST 1 | test/RyanJsonMemoryFootprintTest.c:294] 结束执行: 结果 ✅ | 耗时: 0 ms

┌── [TEST 2 | test/RyanJsonMemoryFootprintTest.c:295] 开始执行: testObjectJsonMemory()
json原始文本长度为 3991, 序列化后RyanJson内存占用: 11308, cJSON内存占用: 21068, yyjson内存占用: 13016
比cJSON节省: 46.33% 内存占用, 比yyjson节省: 13.12% 内存占用
└── [TEST 2 | test/RyanJsonMemoryFootprintTest.c:295] 结束执行: 结果 ✅ | 耗时: 0 ms

┌── [TEST 3 | test/RyanJsonMemoryFootprintTest.c:296] 开始执行: testArrayJsonMemory()
json原始文本长度为 1205, 序列化后RyanJson内存占用: 5644, cJSON内存占用: 11284, yyjson内存占用: 5104
比cJSON节省: 49.98% 内存占用, 比yyjson节省: -10.58% 内存占用
└── [TEST 3 | test/RyanJsonMemoryFootprintTest.c:296] 结束执行: 结果 ✅ | 耗时: 0 ms

┌── [TEST 4 | test/RyanJsonMemoryFootprintTest.c:297] 开始执行: testSmallMixedJsonMemory()
json原始文本长度为 90, 序列化后RyanJson内存占用: 252, cJSON内存占用: 520, yyjson内存占用: 676
比cJSON节省: 51.54% 内存占用, 比yyjson节省: 62.72% 内存占用
└── [TEST 4 | test/RyanJsonMemoryFootprintTest.c:297] 结束执行: 结果 ✅ | 耗时: 0 ms

┌── [TEST 5 | test/RyanJsonMemoryFootprintTest.c:298] 开始执行: testSmallStringJsonMemory()
json原始文本长度为 100, 序列化后RyanJson内存占用: 272, cJSON内存占用: 620, yyjson内存占用: 676
比cJSON节省: 56.13% 内存占用, 比yyjson节省: 59.76% 内存占用
└── [TEST 5 | test/RyanJsonMemoryFootprintTest.c:298] 结束执行: 结果 ✅ | 耗时: 0 ms

┌── [TEST 6 | test/RyanJsonMemoryFootprintTest.c:299] 开始执行: testCompressedBusinessJsonMemory()
json原始文本长度为 551, 序列化后RyanJson内存占用: 2032, cJSON内存占用: 4872, yyjson内存占用: 3056
比cJSON节省: 58.29% 内存占用, 比yyjson节省: 33.51% 内存占用
└── [TEST 6 | test/RyanJsonMemoryFootprintTest.c:299] 结束执行: 结果 ✅ | 耗时: 0 ms
```



#### [RFC 8259](https://github.com/nst/JSONTestSuite) 标准符合性测试

**RyanJson使用Double存储浮点数，超大数字会丢失精度**

***如果项目需要完全兼容Unicode字符集，可以考虑yyjson / json-c***

```
*****************************************************************************
*************************** RyanJson / cJSON / yyjson RFC8259标准测试 **************************
*****************************************************************************
┌── [TEST 1 | test/RFC8259Test/RyanJsonRFC8259JsonTest.c:282] 开始执行: testRFC8259RyanJson()
1 数据不完全一致 -- 原始: [-1e+9999] -- 序列化: [null]
2 数据不完全一致 -- 原始: [123123e100000] -- 序列化: [null]
3 数据不完全一致 -- 原始: [-123123e100000] -- 序列化: [null]
4 数据不完全一致 -- 原始: [0.4e00669999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999969999999006] -- 序列化: [null]
5 数据不完全一致 -- 原始: [1.5e+9999] -- 序列化: [null]
RFC 8259 JSON: (322/322)
└── [TEST 1 | test/RFC8259Test/RyanJsonRFC8259JsonTest.c:282] 结束执行: 结果 ✅ | 耗时: 69 ms

┌── [TEST 2 | test/RFC8259Test/RyanJsonRFC8259JsonTest.c:283] 开始执行: testRFC8259yyjson()
RFC 8259 JSON: (322/322)
└── [TEST 2 | test/RFC8259Test/RyanJsonRFC8259JsonTest.c:283] 结束执行: 结果 ✅ | 耗时: 8 ms

┌── [TEST 3 | test/RFC8259Test/RyanJsonRFC8259JsonTest.c:284] 开始执行: testRFC8259cJSON()
应该失败，但是成功: [012], len: 5
应该失败，但是成功: [2.e+3], len: 7
1 数据不完全一致 -- 原始: [0e1] -- 序列化: [0]
应该失败，但是成功: 123, len: 4
应该失败，但是成功: [0.e1], len: 6
2 数据不完全一致 -- 原始: [-1e+9999] -- 序列化: [null]
3 数据不完全一致 -- 原始: [123123e100000] -- 序列化: [null]
4 数据不完全一致 -- 原始: [ -- 序列化: [""]
应该失败，但是成功: ["new
line"], len: 12
5 数据不完全一致 -- 原始: [-123123e100000] -- 序列化: [null]
6 数据不完全一致 -- 原始: [1E+2] -- 序列化: [100]
应该失败，但是成功: [1.], len: 4
7 数据不完全一致 -- 原始: [0e+1] -- 序列化: [0]
8 数据不完全一致 -- 原始: ["a -- 序列化: ["a"]
应该失败，但是成功: ["a, len: 7
应该失败，但是成功: [-012], len: 6
9 数据不完全一致 -- 原始: [0.4e00669999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999969999999006] -- 序列化: [null]
10 数据不完全一致 -- 原始: ["\uqqqq"] -- 序列化: [""]
应该失败，但是成功: ["\uqqqq"], len: 10
11 数据不完全一致 -- 原始: [
                            ] -- 序列化: []
应该失败，但是成功: [
                     ], len: 3
12 数据不完全一致 -- 原始: [ -- 序列化: []
应该失败，但是成功: [, len: 3
13 数据不完全一致 -- 原始: [1.5e+9999] -- 序列化: [null]
14 数据不完全一致 -- 原始:  -- 序列化: [""]
应该失败，但是成功: [2.e-3], len: 7
应该失败，但是成功: [2.e3], len: 6
应该失败，但是成功: ["  "], len: 5
15 数据不完全一致 -- 原始: [1e+2] -- 序列化: [100]
应该失败，但是成功: [-01], len: 5
应该失败，但是成功: [-.123], len: 7
16 数据不完全一致 -- 原始: {} -- 序列化: {}
17 数据不完全一致 -- 原始: [20e1] -- 序列化: [200]
18 数据不完全一致 -- 原始: [123.456e-789] -- 序列化: [0]
19 数据不完全一致 -- 原始: [123e-10000000] -- 序列化: [0]
应该失败，但是成功: [-2.], len: 5
RFC 8259 JSON: (305/322)
└── [TEST 3 | test/RFC8259Test/RyanJsonRFC8259JsonTest.c:284] 结束执行: 结果 ✅ | 耗时: 7 ms
```

### 5、局限性与注意事项

- **数值精度**：内部使用 `int` / `double` 存储 Number。对于超过 double 精度的 64 位整数或高精度浮点数，double内部使用 snprintf 打印，如果你的平台不支持科学计数法，建议使用 String 类型存储以避免精度丢失。
- **重复 Key**：RyanJson 允许对象中存在重复 Key（解析时不报错），但在查找时只会返回链表中第一个匹配项。

### 6、文档

📂 示例代码：`RyanJsonExample` 文件夹

📖 文档中心：RyanDocs

📧 联系与支持：如有任何疑问或商业合作需求，请联系：`1831931681@qq.com`

