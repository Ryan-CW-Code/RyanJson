# 常见坑与排障（RT-Thread）

## 范围
- 本页是公开 API 视角排障，不展开内部实现优化细节。
- 共性语义口径见 `../../shared/ryanJsonCommon.md`。

## 高发误用
- 未先调用 `RyanJsonInitHooks` 就 Parse/Create。
- `Get*` 前不做判空和 `RyanJsonIsXXX` 判型。
- 把 `Change*Value` 当类型切换接口使用。
- 已挂树节点重复 Add/Insert/Replace。
- 忽略 `RyanJsonStrictObjectKeyCheck` 导致重复 key 语义误判。
- Replace 失败后误判 `item` 所有权。
- 误以为 `RyanJsonParse` 默认会严格校验尾部文本。
- 把 `RyanJsonMinify` 输出无条件当作 `\0` 结尾字符串使用。

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
- Parse 明明“看起来成功”但后续仍有脏数据：检查是否需要 `RyanJsonParseOptions(..., RyanJsonTrue, ...)`。
- Minify 后字符串拼接异常：检查返回值是否等于 `textLen`（此时不会自动补 `\0`）。

## 依据（仓库内）
- `RyanJson/RyanJsonParse.c`：Parse 默认非严格尾部语义
- `RyanJson/RyanJsonItem.c`：Insert/Replace 失败所有权路径
- `RyanJson/RyanJson.c`：`RyanJsonMinify` 终止符写入条件
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`、`test/unityTest/cases/utils/testUtils.c`
