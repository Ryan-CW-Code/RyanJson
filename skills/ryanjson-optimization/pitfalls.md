# RyanJson 优化雷区（压缩版）

## 1. Parse 路径雷区
- 非法输入下的数字/Unicode 边界最容易引发崩溃。
- 回滚路径必须能清理“半构建”结构。

## 2. Print 路径雷区
- 预分配输出路径对边界计数极其敏感。
- 终止符与长度边界要重点防 off-by-one。

## 3. 所有权雷区
- `Add/Insert` 与 `Replace` 失败语义不同。
- 错把两者当同一规则会导致泄漏或双重释放。

## 4. Compare/遍历雷区
- 大对象比较会进入高成本路径。
- 改动比较逻辑必须覆盖深层与乱序结构回归。

## 5. 宏相关雷区
- `RyanJsonDefaultAddAtHead` 会影响追加顺序与索引语义。
- `RyanJsonStrictObjectKeyCheck` 会影响重复 key 预期。

## 6. 审查检查单
1. 是否改变了公开语义。
2. 是否覆盖了失败路径。
3. 宏敏感断言是否同步。
4. 相关模块是否复跑 fuzz 回归。

## 7. 依据（仓库内）
- `RyanJson/RyanJsonParse.c`：Parse 回滚与尾部语义
- `RyanJson/RyanJsonPrint.c`：预分配边界与失败路径
- `RyanJson/RyanJsonItem.c`：Add/Insert/Replace 失败语义差异
- `test/fuzzer/entry.c`：fuzzer 入口与稳定性回归链路
- `../shared/ryanJsonCommon.md`：统一执行入口与覆盖约定
