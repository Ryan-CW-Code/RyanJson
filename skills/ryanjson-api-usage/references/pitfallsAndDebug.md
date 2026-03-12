# 常见坑与排障（RT-Thread）

## 范围
- 本页是公开 API 视角排障，不展开内部实现优化细节。
- 共性语义口径见 `../../shared/ryanJsonCommon.md`。

## 高发误用
- 未先调用 `RyanJsonInitHooks` 就 Parse/Create。
- `Get*` 前不做判空和 `RyanJsonIsXXX` 判型。
- 把 `Change*Value` 当类型切换接口使用。
- 已挂树节点重复 Add/Insert/Replace。
- `RyanJsonAddItemToObject` 后继续使用旧缓存的子容器指针，不从 root 重新取回节点。
- 忽略 `RyanJsonStrictObjectKeyCheck` 导致重复 key 语义误判。
- `RyanJsonGetObjectToKey` 只能按 key 走路径，混用索引会导致取值异常；数组索引要用 `RyanJsonGetObjectToIndex`。
- 对数组元素调用 `ChangeKey` 会失败（数组元素无 key）；需要挂到对象时用 `AddItemToObject`。
- `RyanJsonGetKey` 仅对 `RyanJsonIsKey` 为 true 的节点有效；对数组元素/纯值节点调用会读到无意义内容。
- `AddItemToArray/AddItemToObject` 仅接受容器节点，失败会释放游离 `item`；不要复用原指针。
- `Insert` 失败也会释放游离 `item`；失败后必须新建节点再继续。
- `ReplaceByIndex(Object)` 要求 `item` 必须带 key（`RyanJsonIsKey` 为 true），否则直接失败。
- 非严格模式下 `DeleteByKey/DetachByKey` 只删除首个重复 key，剩余重复项仍在。
- 非严格模式下重复 key 删除/分离后“剩下哪一个”受插入方向影响，避免硬编码具体值。
- `ParseOptions(requireNullTerminator=false)` 返回的 `parseEndPtr` 可能指向空白，后续可直接传给 `ParseOptions` 由其跳过前导空白。
- Replace 失败后误判 `item` 所有权。
- 误以为 `RyanJsonParse` 默认会严格校验尾部文本。
- 把 `RyanJsonMinify` 输出无条件当作 `\0` 结尾字符串使用。
- `GetObjectByKey/HasObjectByKey` 使用“解码后的 key”做匹配；JSON 中的转义（如 `\"`、`\\`、`\t`、`\n`、`\uXXXX`）在解析后已被还原，查询时需传入实际字符/UTF-8 字节而不是转义文本。
- `AddItemToObject/AddItemToArray` 在包装容器节点时会丢弃原节点 key；如果需要保留原 key，请使用 `Insert` 迁移节点。

## 推荐排障顺序
1. 检查 hooks 初始化是否先于任何 RyanJson API。
2. 确认 `RyanJsonConfig.h` 关键宏（尤其严格 key）与预期一致。
3. 还原最小输入，复现并记录返回值/日志。
4. 核对失败分支的释放顺序（Create/Add/Replace/Detach/Print）。
5. 需要语义取证时，按 `example -> unityTest -> fuzzer` 查证。

## 典型症状速查
- `strcmp` 越界：优先排查 key 是否为合法终止字符串。
- leak：优先排查 Replace/Detach 失败后的调用方释放逻辑。
- 打印返回 NULL：优先检查预分配长度边界与输入树合法性。
- `DetachByIndex`/`GetObjectByIndex` 在“看似有效”的容器上异常返回 NULL：优先排查是否在 AddItem 后复用了旧指针。
- Parse 明明“看起来成功”但后续仍有脏数据：检查是否需要 `RyanJsonParseOptions(..., RyanJsonTrue, ...)`。
- Minify 后字符串拼接异常：检查返回值是否等于 `textLen`（此时不会自动补 `\0`）。

## 依据（仓库内）
- `RyanJson/RyanJsonParse.c`：Parse 默认非严格尾部语义
- `RyanJson/RyanJsonItem.c`：Insert/Replace 失败所有权路径
- `RyanJson/RyanJson.c`：`RyanJsonMinify` 终止符写入条件
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`、`test/unityTest/cases/utils/testUtils.c`
