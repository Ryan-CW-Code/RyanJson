# RyanJson
> 面向资源受限设备的嵌入式通用 JSON 库
>
> 🎯 内存占用降低 **50%** | 🛡️ **100%** 模糊测试覆盖 | 🚀 **零递归**栈依赖

📢 **使用中遇到问题，欢迎提交 Issue 或在 [RT-Thread 社区](https://club.rt-thread.org/index.html) 提问。** 

**快速入口：**`example/`、`./run_local_base.sh`、`AGENTS.md`

### 1、介绍

#### ✅ 特性亮点

- 💡 **极致内存优化：** 通过动态内存扩展 + 紧凑布局，兼容未对齐访问，相较 cJSON 内存占用约降低 **50%**（详见 [内存占用对比报告](memoryUsageCompareQemu.md)）。
- 💡 **适配 malloc 头部与对齐策略：** 通过配置匹配不同平台的分配器特性，平衡内联阈值与内存占用。
- 🧱 **零递归栈依赖：** 采用线索化链表 + 迭代 DFS，避免递归栈依赖，深层嵌套栈占用可控。
- 🔍 **模糊测试护城河：** 基于 [LLVM LibFuzzer](https://llvm.org/docs/LibFuzzer.html) 进行上亿纪随机用例生成与回归，分支覆盖率100%。**[点击在线查看覆盖率信息](https://ryan-cw-code.github.io/RyanJson/)** 
- 🧪 **单元测试矩阵与 QEMU 语义验证一体化：** 覆盖 double、重复 key、深层嵌套、链表稳定性等路径，并在 Cortex-M 语义下校验未对齐访问与 HardFault，核心语义宏组合覆盖。
- 🛡️ **运行时安全分析验证：** 使用 **[Sanitizer](https://clang.llvm.org/docs/index.html#sanitizers)** 系列工具，捕获内存越界、Use-after-free、数据竞争、未定义行为、内存泄漏等问题，提升代码健壮性与安全性。
- 📐 **高质量代码保障：** 引入 **[clang-tidy](https://clang.llvm.org/extra/clang-tidy/#clang-tidy)** 与 **[Cppcheck](https://cppcheck.sourceforge.io/)** 进行静态分析，目标是接近语法级“零缺陷”。
- 🤖 **AI 辅助开发与审查：** 结合 **[Codex](https://openai.com/codex)**、**[Gemini Code Assist](https://codeassist.google/)**、**[coderabbitai](https://www.coderabbit.ai)**、**[Copilot](https://github.com/features/copilot)**，用于编码与代码审查，持续优化代码质量。
- 👩‍💻 **开发者友好：** 类 cJSON 接口设计，迁移成本低
- 📜 **严格但不严苛：** 默认配置通过 **[RFC 8259](https://github.com/nst/JSONTestSuite)** 测试集；支持深层嵌套（受限于内存与平台栈配置），支持注释与尾随逗号（可配置）

### 2、设计

**设计参考与语法背景**
- **借鉴：**[json](https://api.gitee.com/Lamdonn/json) 和 [cJSON](https://github.com/DaveGamble/cJSON)
- **语法参考：**[JSON 规范](https://www.json.org/json-en.html)、[Parsing JSON is a Minefield](https://seriot.ch/projects/parsing_json.html)

RyanJson 的核心竞争力在于对内存布局的精细控制，**结构体表示最小存储单元（键值对）**，通过单链表组织数据，结构如下：

```c

// Json 最基础节点，所有 Json 元素都由该节点表示。
// 结构体仅包含固定的 next 指针，用于单向链表串联。
// 其余数据（如 flag/key/strValue/intValue/doubleValue/objValue 等）均通过动态内存分配管理。
struct RyanJsonNode
{
	// 理论上 next 的低 2 位也可复用
	struct RyanJsonNode *next; // 单链表节点指针

	/**
	 * @brief RyanJson 节点结构与载荷布局说明（面向使用者的实现约定）。
	 * @details
	 * `struct RyanJsonNode` 本体仅保存 `next` 指针，所有元数据与真实载荷都放在结构体后面的
	 * 动态区域（payload）。该 payload 的第一个字节就是 flag，紧跟其后的区域根据 flag 语义切分。
	 * 这种布局能缩小节点本体，并让 key/strValue 的存储策略可切换。
	 *
	 * Layout（逻辑示意）:
	 * [ next | flag(1B) | keyLenField(0/1/2/4B) | inline/ptr payload ... | value ]
	 *
	 * Flag Bits（bit7..bit0）:
	 * - bit0-2: Type（Null/Bool/Number/String/Array/Object）
	 * - bit3  : Bool/Number 扩展位（Bool: true/false；Number: Int/Double）
	 * - bit4-5: keyLenField 编码（0/1/2/4 字节）
	 * - bit6  : strMode（inline/ptr）
	 * - bit7  : IsLast（1 表示 next 指向 Parent 线索）
	 *
	 * keyLenField（key 长度字段）:
	 * - 位于 flag 之后，长度由 bit4-5 编码决定。
	 * - 记录 key 的字节长度（不含 '\\0'），按低字节在前写入。
	 * - 编码值 3 表示字段宽度 4 字节（不是 3 字节）。
	 *   这是为了用 2 bit 表达 0/1/2/4 四种宽度，详见 RyanJsonInternalDecodeKeyLenField。
	 *
	 * Payload（key/strValue 存储策略）:
	 * - 固定字符串区（仅当节点有 key 或类型为 String 时存在）：
	 *   位置：flag 后固定长度 `RyanJsonInlineStringSize`。
	 *   起点：先写 keyLenField（宽度由 flag 编码 0/1/2/4 字节）。
	 *
	 * - inline 模式：
	 *   内容：keyLenField 后顺序写 key\\0 与 strValue\\0。
	 *   变体：String 节点有 strValue；key 为空则仅 strValue\\0；非 String 节点仅 key\\0。
	 *
	 * - ptr 模式：
	 *   指针槽：固定在 flag + RyanJsonKeyFeidLenMaxSize，不随 keyLenField 宽度变化。
	 *   说明：读写指针用 memcpy，规避潜在非对齐访问。
	 *   堆区：有 key 则 [key\\0]；String 节点再追加 [strValue\\0]；无 key 则仅 [strValue\\0]。
	 *
	 * - 内联判定：
	 *   条件：key/strValue 字节总和 + keyLenField 宽度 <= `RyanJsonInlineStringSize`。
	 *   说明：无 key 时 keyLenField 宽度为 0。
	 *
	 * Value 存储位置（与 key 是否存在相关）:
	 * - Number/Array/Object 的 value 位于 payload 中固定偏移处。
	 * - String 的 value 存在于 key/strValue 区域，不使用 value 偏移。
	 * - 如果节点带 key，则 value 放在 flag + RyanJsonInlineStringSize 之后；
	 *   这样无论 inline/ptr 模式，value 偏移都稳定。
	 * - 若节点无 key，则 value 紧跟 flag。
	 * - Null/Bool 仅使用 flag 位表达，无额外 payload。
	 * - 实际偏移以 RyanJsonInternalGetValue 的计算为准。
	 *
	 * inline / ptr 简化示意（payload 仅示意，不含 value 区）:
	 * - inline 示例:
	 *   key + strValue: [ flag | keyLenField | key\\0 | strValue\\0 | ... ]
	 *   key only (非 String): [ flag | keyLenField | key\\0 | ... ]
	 *   strValue only (key 为空): [ flag | keyLenField | strValue\\0 | ... ]
	 * - ptr 示例:
	 *   key + strValue: [ flag | keyLenField | (pad) | ptr | ... ]  ptr -> [ key\\0 | strValue\\0 ]
	 *   key only (非 String): [ flag | keyLenField | (pad) | ptr | ... ]  ptr -> [ key\\0 ]
	 *   strValue only (key 为空): [ flag | keyLenField | (pad) | ptr | ... ]  ptr -> [ strValue\\0 ]
	 *   padding 表示内联区未使用的剩余空间或对齐填充。
	 *
	 * Threaded List（线索化链表）:
	 * - 同层兄弟节点通过 `next` 串联。
	 * - 最后一个兄弟节点的 `next` 指向父节点，并设置 IsLast=1。
	 * - 对外遍历必须使用 `RyanJsonGetNext`，它会屏蔽父节点线索。
	 * - 因此 `next` 不是“永远指向兄弟”，IsLast=1 时它是父节点线索。
	 *
	 * Example（Object 子节点链表示意）:
	 *   root(Object)
	 *     |
	 *     +-- "a":1  -> "b":2  -> (IsLast=1, next=root)
	 *   RyanJsonGetNext("b") == NULL
	 *
	 * Array/Object 子节点与父节点线索示意:
	 *   parent(Object)
	 *     |
	 *     +-- child0 -> child1(Last, next=parent)
	 *            |
	 *            +-- grandChild0 -> grandChild1(Last, next=child1)
	 *
	 * Offset 快速对照（从 payload 起点算起，用于理解访问宏偏移）:
	 * - flag: 0
	 * - keyLenField: 1
	 * - inline/ptr payload: 1 + keyLenField 宽度
	 * - value（无 key）: 1
	 * - value（有 key）: 1 + RyanJsonInlineStringSize
	 *
	 * 修改影响范围提示:
	 * - 改动 flag 位语义或 keyLenField 编码时，需同步 RyanJsonGetKey/RyanJsonGetStringValue。
	 * - 改动 payload 布局或内联阈值时，需同步 RyanJsonInternalGetValue 与相关测试。
	 *
	 * @note 该布局依赖 flag 位语义与 keyLenField 编码规则。
	 * @note 常见误解提示:
	 * - IsLast=1 的节点其 next 不是兄弟，而是父节点线索。
	 * - 遍历同层必须使用 RyanJsonGetNext，不能直接读 next。
	 * - value 偏移与是否有 key 强相关，不能用固定结构体偏移理解。
	 * @note 修改内联阈值或 payload 布局时需同步更新注释与测试。
	 */
};

typedef struct RyanJsonNode *RyanJson_t;
```

#### ✅ 核心源码与模块入口
- `RyanJson/RyanJson.h`：公开 API 与节点结构体定义
- `RyanJson/RyanJsonParse.c`：解析主流程与输入校验
- `RyanJson/RyanJsonPrint.c`：序列化输出与格式化策略
- `RyanJson/RyanJsonItem.c`：节点创建、插入、替换、分离等操作
- `RyanJson/RyanJsonUtils.c`：链表与内部工具实现
- `RyanJson/RyanJsonConfig.h`：宏配置与平台差异
- `xmake.lua`、`xmake/`：构建配置
- `run_local_*.sh`：本地脚本入口
- `skills/shared/architecture.md`：架构与内部实现说明（AI 入口）

#### ✅ 目录结构速查
| 目录 | 说明 |
| --- | --- |
| `RyanJson/` | 核心库源码与头文件 |
| `test/` | Unity 测试、fuzz 用例、第三方依赖 |
| `example/` | 使用示例 |
| `skills/` | AI 技能与知识库入口 |
| `scripts/` | 辅助脚本与工具 |
| `xmake/` | 构建脚本 |
| `build/` | 构建输出 |
| `localLogs/` | 覆盖率与脚本日志输出 |

### 3、测试与质量保障

**测试体系概览**
- **组成：**单元测试矩阵 + 模糊测试 + QEMU 语义验证
- **目标：**将稳定性验证前置到研发阶段

#### ✅ 本地测试脚本
| 脚本                            | 用途               | 推荐场景                |
| --- | --- | --- |
| `./run_local_base.sh`           | 单元测试矩阵       | 修改测试用例后          |
| `./run_local_fuzz.sh`           | LibFuzzer 模糊测试 | 解析/打印/内存路径改动  |
| `./run_local_qemu.sh`           | QEMU 语义验证      | 嵌入式语义/对齐相关改动 |
| `./run_local_ci.sh`             | 完整 CI 验证       | 提交前对齐              |
| `./run_local_format.sh --check` | 格式检查           | 代码风格调整            |
| `./run_local_skills.sh`         | skills 同步与校验  | 更新文档后 |

#### 🧪 专项功能单元测试覆盖域

| 分组目录 | 说明 |
| --- | --- |
| `core` | 核心 API 语义与主路径（创建、插入、替换、删除、遍历等） |
| `edge` | 极限/异常输入与边界约束，强调失败语义与资源回收（含解析/打印边界） |
| `equality` | double 精度、科学计数法、往返一致性、Compare/CompareOnlyKey 与重复 key 语义 |
| `performance` | 深层嵌套/递归栈压力与性能回归基线（默认深度 10000，object/array/mixed 模式） |
| `scenario` | 跨 API 的业务场景组合与流程一致性 |
| `stability` | 链表迁移/遍历稳定性与长路径回归 |
| `usage` | 使用方式与集成约定的示例型单元测试 |
| `utils` | 工具函数与辅助路径的正确性验证 |
| `RFC8259` | RFC8259 标准样例与一致性检验 |

#### 🛰️ QEMU 语义验证（嵌入式硬件语义）
- 通过 QEMU 模拟 Cortex-M 语义路径，覆盖未对齐访问与 HardFault 处理等硬件级关键行为（`UNALIGN_TRP=1` 强制触发未对齐异常）。
- 面向嵌入式场景的“语义级一致性”保障，不仅是“能跑通”，更是“语义正确”。
- 脚本入口：`./run_local_qemu.sh`

#### 🔍 **LLVM LibFuzzer 模糊测试**（核心亮点）

LLVM LibFuzzer 模糊测试是 RyanJson 的 **稳定性护城河**，用于持续验证异常输入与极端边界。

**[点击在线查看覆盖率信息](https://ryan-cw-code.github.io/RyanJson/)** 

- **上亿级测试样本**：[LLVM LibFuzzer](https://llvm.org/docs/LibFuzzer.html) 自动生成并执行上亿级随机与非法 JSON 输入
- **覆盖率极高**：覆盖率以在线报告为准，期望长期保持高覆盖、无崩溃、无泄漏
- **鲁棒性验证**：内存申请失败、扩容失败、非法转义字符、尾随逗号、嵌套过深、随机类型切换
- **内存安全验证**：结合 **[Sanitizer](https://clang.llvm.org/docs/index.html#sanitizers)** 工具链，确保无泄漏、无悬空指针、无越界

| 测试类别                   | 测试目标                                                  |
| -------------------------- | --------------------------------------------------------- |
| **内存故障模拟测试**       | 覆盖 OOM 场景下的异常回滚与系统稳定性                     |
| **随机解析鲁棒性测试**     | 覆盖非法语法、畸形字符、极端输入的容错与边界防御           |
| **循环遍历删除安全性测试** | 覆盖迭代中删除节点的链表一致性与指针安全                  |
| **循环遍历分离安全性测试** | 覆盖高频分离后的拓扑重构与内存权属逻辑                    |
| **转义序列压缩测试**       | 覆盖复杂/异常转义字符在压缩路径中的解析完整性            |
| **序列化回环一致性测试**   | 覆盖“解析-打印-解析”链路的数据一致性                     |
| **高频迭代值读取测试**     | 覆盖多层结构下 Value 访问的稳定性与寻址效率              |
| **随机压力复制安全测试**   | 覆盖大规模深拷贝的内存池利用效率与拓扑安全               |
| **动态类型切换压力测试**   | 覆盖运行期类型切换与动态扩展的内存安全                   |

#### 🛡️ 工具链集成

运行时、静态分析与格式化三位一体，形成持续质量防线。

| 工具                                                         | 用途                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| **[Sanitizer](https://clang.llvm.org/docs/index.html#sanitizers)** | 运行时检测，捕获内存泄漏、越界、数据竞争。杜绝泄漏、越界、悬空指针 |
| **[clang-tidy](https://clang.llvm.org/extra/clang-tidy/#clang-tidy)** | 静态分析潜在缺陷（空指针、资源泄漏等）                       |
| **[Cppcheck](https://cppcheck.sourceforge.io/)**             | 深度扫描内存与资源问题                                       |
| **[ClangFormat](https://clang.llvm.org/docs/ClangFormat.html)** | 统一代码风格                                                 |
| AI 代码审查                                                  | **[Codex](https://openai.com/codex)**、**[Gemini Code Assist](https://codeassist.google/)**、**[coderabbitai](https://www.coderabbit.ai)**、**[Copilot](https://github.com/features/copilot)** 辅助优化逻辑，构建多层安全防线 |
| **编译器警告**                                               | `-Wall -Wextra`（默认）、`-Weffc++`/`-Weverything`（Clang 可选，CI 强化时开启） |

#### ✅ 当前主机侧单测基线（脚本默认）
- 默认构建目标：`RyanJson`
- 默认工具链：`clang + linux x86`（当前主要验证环境，见 `xmake/host.lua`）
- 默认启用：Sanitizer 与链接期安全硬化（见 `xmake/host.lua`）

### 4、基准测试

*测试与示例代码见本项目根目录 `test/` 和 `example/` 文件夹。*

#### 性能测试
**性能测试结果见 RyanDocs 文档中心（基于 [yyjson_benchmark](https://github.com/ibireme/yyjson_benchmark)，已过时，仅供参考）。**


#### 内存占用测试
RyanJson 支持按平台配置 `malloc` 头部空间与对齐粒度（见 `RyanJson/RyanJsonConfig.h`）

- Host 报告：[memoryUsageCompareHost.md](memoryUsageCompareHost.md)
- QEMU 报告：[memoryUsageCompareQemu.md](memoryUsageCompareQemu.md)

生成方式：

```
测试用例代码：`test/unityTest/cases/performance/testMemory.c`

bash run_local_memory.sh              			# 同时生成 host + qemu
MEM_PLATFORM=host bash run_local_memory.sh   	# 仅主机侧
MEM_PLATFORM=qemu bash run_local_memory.sh   	# 仅 QEMU
```

#### [RFC 8259](https://github.com/nst/JSONTestSuite) 标准符合性测试
RFC8259 测试结果已独立输出为 Markdown 文档，避免 README 过长。

- Host 报告：[rfc8259ReportHost.md](rfc8259ReportHost.md)
- QEMU 报告：[rfc8259ReportQemu.md](rfc8259ReportQemu.md)

生成方式：

```
测试代码入口：`test/unityTest/cases/RFC8259/testRfc8259.c`。

bash run_local_rfc8259.sh              			# 同时生成 host + qemu
RFC_PLATFORM=host bash run_local_rfc8259.sh   	# 仅主机侧
RFC_PLATFORM=qemu bash run_local_rfc8259.sh   	# 仅 QEMU
```

### 5、使用限制与注意事项

- **数值精度**：内部使用 `int` / `double` 存储 Number。对于超过 double 精度的 64 位整数或高精度浮点数，内部使用 snprintf 打印；如果平台不支持科学计数法，建议使用 String 类型存储以避免精度丢失。
- **重复 Key**：严格模式下解析阶段拒绝重复 key；非严格模式允许重复 key，`GetObjectByKey`/`DetachByKey` 命中首个重复项，`DeleteByKey` 每次删除一个，`Compare` 在非严格模式按同 key 的出现序号对齐比较。

### 6、文档
- 📂 示例代码：`example/` 文件夹
- 📖 文档中心：RyanDocs
- 📧 联系与支持：如有任何疑问或商业合作需求，请联系：`1831931681@qq.com`

### 7、快速开始

#### ✅ 编译工具链需要加入的文件
- `RyanJson/*.c`（核心源码）
- `RyanJson/RyanJson.h`（公开 API）
- `RyanJson/RyanJsonConfig.h`（宏配置与平台差异）
- 头文件搜索路径加入 `RyanJson/`

#### ✅ 最小初始化要求
- 在任何 JSON API 前调用 `RyanJsonInitHooks(malloc, free, realloc)`
- 业务代码包含 `#include "RyanJson.h"`
- 如需调整宏（例如 `RyanJsonStrictObjectKeyCheck`），在包含头文件前定义或在 `RyanJsonConfig.h` 中配置

#### ✅ 示例入口
- 参考 `example/` 目录的最小示例

### 8、AI 与知识库入口
- 使用方式：先阅读 `AGENTS.md`，按问题类型进入对应技能文档，即可快速获得可落地方案。
- 提问建议：说明平台、宏前提、目标场景与是否需要代码示例，AI 会据此给出最短路径与稳妥的失败语义说明。
- 总入口与路由：`AGENTS.md`
- 架构与数据结构：`skills/shared/architecture.md`
- API 使用与所有权：`skills/ryanjson-api-usage/SKILL.md`
- 内部优化与语义边界：`skills/ryanjson-optimization/SKILL.md`
- 单元测试工程：`skills/ryanjson-test-engineering/SKILL.md`

### 9、贡献与验证

#### ✅ xmake 构建快速开始（主机侧）
- xmake 官网：https://xmake.io
- 默认验证环境：linux x86
- `xmake f`
- `xmake -b RyanJson`

#### ✅ 场景与脚本建议
按变更场景选脚本，确保修改可验证、可复现、可回归。
| 变更场景 | 推荐脚本 |
| --- | --- |
| 仅修改单元测试用例 | `./run_local_base.sh` |
| 解析/打印/内存路径改动 | `./run_local_base.sh` + `./run_local_fuzz.sh` |
| 涉及嵌入式语义路径或对齐相关 | `./run_local_qemu.sh` |
| 提交前对齐 CI | `./run_local_ci.sh` |
| 格式或风格调整 | `./run_local_format.sh --check` |
| 更新 skills/agent 文档 | `./run_local_skills.sh` |
