# RyanJson API 场景模式（压缩版）

## 1. 作用
- 按用户意图快速选公开 API 路径。
- 回答时强制带上失败分支与所有权说明。
- 完整版见 `references/apiPatterns.md`。

## 2. 先做三件事
1. 先确认 `RyanJsonInitHooks`。
2. 先确认宏前提：`RyanJsonStrictObjectKeyCheck`、`RyanJsonDefaultAddAtHead`。
3. 先判定场景再给最小 API 组合。

## 3. 快速选型
| 场景 | 推荐路径 | 关键风险 |
|---|---|---|
| 读取配置 | Parse + Get + IsXXX | 未判型直接取值 |
| 周期上报 | Create + Add + PrintPreallocated(..., RyanJsonFalse, ...) | 缓冲不足、清理遗漏 |
| 同类型更新 | Get + Change*Value | 跨类型误用 Change |
| 跨类型更新 | Create* + ReplaceBy* | Replace 失败后泄漏 |
| 子树迁移 | Detach* + Add/Insert | detach 后未接管 |
| 传输压缩 | Print(format=false) | 误把 Minify 当传输主路径 |
| 文本清洗 | Minify | `\0` 终止符假设错误 |

## 4. 关键语义提醒
- `Replace` 失败不消费 `newItem`，调用方需复用或释放。
- `Add/Insert` 与 `Replace` 失败语义不能混用。
- 传输压缩优先非格式化打印，不推荐“先格式化再 Minify”。
- `RyanJsonDefaultAddAtHead=true` 时，追加顺序可能反转。

## 5. 输出模板
1. 前提：宏前提 + hooks 前提。
2. 路径：推荐 API 顺序（成功 + 失败）。
3. 所有权：每个失败分支谁释放。
4. 验证：返回值/日志/内存检查点。

## 6. 最小依据
- `RyanJson/RyanJsonItem.c`：Add/Insert/Replace/Detach 失败语义
- `RyanJson/RyanJson.c`：Minify 行为
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`
- `test/unityTest/cases/utils/testPrint.c`、`test/unityTest/cases/utils/testUtils.c`
