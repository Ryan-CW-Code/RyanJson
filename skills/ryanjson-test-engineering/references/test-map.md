# RyanJson 单元测试地图

用于标注各测试目录/文件职责，避免重复覆盖，并为后续补测提供定位参考。

最高准则：新增/改动单元测试绝不允许与现有测试形成功能语义重复；若发现重复，必须并入正确归属文件或直接删除，而不是并存。

## test/unityTest/common
- `testCommon.h` / `testCommon.c`：测试运行时通用工具、内存/泄漏钩子、Unity 辅助接口。
- `testPlatform.h`：测试平台抽象与宏封装。
- `unity_config.h`：Unity 配置。
- `FreeRTOSConfig.h`：FreeRTOS 测试环境配置。

## test/unityTest/runner
- `main.c`：Unity runner 入口，注册并执行全部测试 runner。
- `test_list.inc`：runner 列表（被 main.c 编译包含），由 `run_local_base.sh` 自动生成，禁止手改。

## test/unityTest/cases

### RFC8259
- `testRfc8259.c`：RFC 8259 标准用例集（有效/无效 JSON），统一走内嵌数据集。
- `rfc8259Embedded.c` / `rfc8259Embedded.h`：RFC8259 内嵌数据集（由脚本生成）。
- `testRfc8259Util.c` / `testRfc8259Util.h`：RFC8259 语义对比辅助。

### core
- `testChange.c`：ChangeKey/Change*Value 的成功/失败分支与边界；包含 strict/non-strict 重复 key 策略、同文本 key no-op、数字字符串 ID 保真，以及失败链路不污染文档的收敛契约。
- `testCreate.c`：Create/Add/Insert/AddPosition 相关 API 与所有权规则；包含 Create* 参数守护、typed array 零长度/OOM、标量创建 OOM，以及 AddItemToObject/AddItemToArray 仅接受容器节点、失败后可恢复复用的契约。
- `testDelete.c`：DeleteByKey/DeleteByIndex 的边界与失败语义。
- `testDetach.c`：DetachByKey/DetachByIndex 与再插入/迁移行为。
- `testDuplicate.c`：Duplicate 语义、隔离与回归链路。
- `testForEach.c`：for-each 宏与遍历期间的变更行为。
- `testLoadFailure.c`：ParseOptions 失败语义、空白/指数溢出/非法长 key value-error、流式失败隔离、OOM 恢复、非法 UTF-8 透传与内嵌 NUL 防护。
- `testLoadSuccess.c`：解析成功场景、数值边界与 stream 解析。
- `testReplace.c`：ReplaceByKey/ReplaceByIndex 的成功/失败、key 重写、detached 复用与所有权语义。
- `standard/testStandardValueRoundtrip.c`：标准 JSON 值语义；聚焦顶层标量、转义/Unicode 值、字面量类型矩阵、空容器、标准空白包裹文档与非有限数值拒绝，不承载 key API 或流式边界。
- `standard/testStandardStream.c`：标准文档流/切片语义；聚焦 mixed top-level sequence、完整/截断切片、`parseEndPtr==NULL` 分流、多文档解析后相互隔离，不重复 edge 的 terminator/end-pointer 基础边界。
- `standard/testStandardPreprocess.c`：标准文本预处理前置语义；聚焦 UTF-8 BOM 拒绝与“注释文本必须先 Minify 才能进入标准解析”的契约。

### edge
- `testEdgeKeyBoundary.c`：key 长度边界、ChangeKey 切换与嵌套长 key roundtrip；不再混放 string value 的边界契约。
- `testEdgeStringBoundary.c`：string value 边界与存储模式契约；聚焦 ChangeStringValue、长 value roundtrip，以及解析路径下按“解码后长度”决定 inline/ptr 的规则。
- `testEdgeWrapRebind.c`：wrap/rebind 边界；聚焦对象/数组之间包装迁移后的 key 丢弃、重新绑定，以及“包装后再回插数组仍保留 key”的语义。
- `edge/container/testEdgeContainerInsertDelete.c`：容器 Insert/Delete/Detach 边界；聚焦超范围插入、同索引回插、删除后重插、对象按索引删除、数组接收 keyed 节点、越界 detach、数组内改 key，以及 `AddItemToObject` 标量失败后的恢复语义。
- `edge/container/testEdgeContainerReplaceGuard.c`：容器 Replace 与直接访问守护边界；聚焦 ReplaceByKey/Index 失败恢复、空容器替换失败保持 item 游离、keyless/non-container 守护、head 替换入口更新、标量/空容器直接访问返回 NULL 等语义。
- `edge/parseprint/testEdgeMinifySlices.c`：`Minify` 在显式长度 slice 上的边界契约；聚焦对象/数组/嵌套容器周围的注释剔除、字符串内注释标记保留，以及内嵌 NUL 截断时不越界覆盖尾部哨兵。
- `edge/parseprint/testEdgeParseOptions.c`：`ParseOptions`/`parseEndPtr`/`Minify` 边界；聚焦严格尾部、顺序续读、zero-size 与 scalar/string/Unicode 截断失败、字符串转义不干扰 end 指针、垃圾段失败与注释剔除。
- `edge/parseprint/testEdgePrintOptions.c`：`Print`/`PrintPreallocated`/`PrintWithStyle` 边界；聚焦 root 标量回环、格式化开关、预分配精确容量/失败恢复、自定义 style 可解析性。

### equality
- `testEqualityBool.c`：布尔解析/打印/往返一致性。
- `testEqualityDouble.c`：double 解析/打印/往返一致性矩阵。
- `testEqualityInt.c`：int 解析/打印/往返一致性矩阵。
- `testEqualityString.c`：字符串解析/打印/往返一致性矩阵。

### performance
- `testDeepRecursion.c`：深度嵌套与栈消耗压力。
- `testMemory.c`：内存占用评估与对比。
- `testStress.c`：大对象/大数组压力与往返校验。

### scenario
- `testScenarioChainMutations.c`：仅保留深层 move/replace/roundtrip、数组对象与对象字段互换等高耦合 mutation 场景；不再承载可由 core 单独证明的跨容器迁移重复语义。
- `testScenarioCompositeFlows.c`：仅保留多视图同步、失败后节点复用、批量索引收集分离等高耦合集成链路，不再承载基础 stream/minify 或 snapshot 分支契约。
- `testScenarioSnapshots.c`：snapshot/rollback 集成链路；聚焦 `Duplicate` 快照回滚，以及“回滚后继续分叉变更时 live/snapshot 仍保持隔离且各自 roundtrip 稳定”的契约。

### stability
- `testStabilityLinkedListBasic.c`：链表基础稳定性；保留跨父节点迁移、对象重建、包装子链表、Duplicate/Print 后稳定性，以及 public detach/replace 回绑 parent/next 的结构不变量；不承载 trivial 的两节点 head/tail 重复语义。
- `testStabilityLinkedListChurn.c`：链表高频 churn 稳定性；聚焦嵌套迁移、回插、replace 后 sibling 遍历无环与尾节点正确，不再承载单节点 detach/回插这类低增益场景。
- `testStabilityLinkedListStress.c`：链表高压混合操作稳定性；保留大规模 churn、多次 replace、重复 key 删除后的结构稳定，不重复基础迁移场景。

### usage
- `testUsageRoundtrip.c`：usage 文档回环契约集；仅保留 usage 层独有的真实文档结构样本，按“root array / mixed literals / deep mixed containers / nested null+empty / sibling lists / business documents”归类，不再堆放与 standard/equality 重复的转义、Unicode、空容器基础矩阵或纯业务换皮样本。
- `testUsageRecipes.c`：仅保留不被 core/edge/stability 覆盖的用户级文档管线 recipe；当前聚焦 `Minify -> Parse -> Add/Replace -> Print -> Parse`，`Minify -> ParseOptions` 顺序消费多文档流后将第二份根容器并入第一份文档，以及 `Create(ArrayRoot)` 顺序聚合 `ParseOptions` 解析出的 mixed top-level documents。
- `testUsageContainers.c`：仅保留不被 edge/stability 吞并的容器 recipe；聚焦 `index==size` 显式尾插、带 key 容器迁移/改名、子数组提升到根级、数组元素改挂对象、对象字段降级为数组元素，以及“独立 Parse 出来的根容器直接并入另一份根文档”、“从一份 parsed 文档 detach 子树再迁到另一份 parsed 文档”，和“从 parsed 文档 detach 子树迁入 created 文档”的用户可见结构流转。

### utils
- `utils/print/testPrintGeneral.c`：非样式打印主契约；聚焦 `Print`/`PrintPreallocated` 的根节点保护、参数守护、UTF-8 精确容量、int/double 预留空间、科学计数法、`0.0` / `<1e15` / `1e15` 固定点边界、极小 double 原始输出保真，以及扩容 fallback/OOM 路径。
- `utils/print/testPrintStyle.c`：样式打印主契约；聚焦 `PrintWithStyle`/`PrintPreallocatedWithStyle` 的参数守护、默认/自定义格式特征、tab+CRLF 风格、返回长度与 double 场景下的 headroom 语义。
- `testUtils.c`：测试公共断言 helper 的实现承载文件，并集中保留 `Minify` 的缓冲写入契约（是否补 `\0`、零长度不写、纯注释清空、截断转义不越界、非法参数不写缓冲区）；不再重复承载标准对象样本校验。
- `testInternalApis.c`：内部 API 合约（Internal* 系列）、key 长度编码/读取、InlineStringSize 布局公式、低层链表操作、memory hook 参数守护，以及内部 value 指针/标志位语义。
### core/compare
- `compare/testCompare.c`：Compare/CompareOnlyKey 在常规文档中的语义与差异（含根标量路径），并承接 `RyanJsonCompareDouble` 的绝对/相对容差切换契约。
- `compare/testCompareDuplicateKeyBasic.c`：重复 key 的基础 Compare/CompareOnlyKey 覆盖。
- `compare/testCompareDuplicateKeyAdvanced.c`：重复 key 高阶场景与修复链路。
- `compare/testCompareMutation.c`：由变更链路驱动的 Compare/CompareOnlyKey 合约；聚合“值变化忽略、类型变化失败、嵌套数组长度失配修复、重复 key 计数漂移、包装后结构比较”等非重复语义。

### core/accessor
- `accessor/testAccessor.c`：访问器与遍历 API、参数守护、路径查询成功链路、手工构造树上的便捷宏路径可达性、对象/数组入口节点更新语义，以及 sibling 一致性。
- `accessor/testAccessorPathGuard.c`：路径查询在 array/object/scalar 边界上提前停止的守护语义；验证错误路径不会污染后续合法查询。
- `accessor/testAccessorMutationPaths.c`：结构变更后的路径恢复与稳定性；聚合数组重排、深层 GetObjectByIndexs、replace/detach 后 Has/Path 一致性、类型替换后的 key 路径、反复增删后的 key 查找。

### core/key
- `key/testKeyEscapeLookup.c`：key 转义、UTF-8/空 key 路径查询，以及 Duplicate 后解码 key 查询保持一致；聚焦转义 key 解码、路径 API 与 ChangeKey 后的可达性。
- `key/testKeyDuplicateLookup.c`：重复 key 的查询/分离/删除语义；聚焦 ChangeKey 造重、转义 key 与 UTF-8 等价 key 冲突、GetObjectByKey/DetachByKey 命中一致性，以及 DeleteByKey 对重复 key 仅移除单节点的遍历计数约束。
- `key/testKeyEscapeRoundtrip.c`：key/value 转义打印往返。
- `key/testKeyMutationLookup.c`：变更后的 key 查询/回环语义；聚合空 key、带引号/反斜杠 key、控制字符 key、UTF-8 key 的 ChangeKey + Lookup/Print/Parse 合约。
- `key/testKeyNumericLike.c`：numeric-like key（前导零/指数样式/大整数样式/Infinity/NaN）按文本保真查找、变更与精确匹配语义。
