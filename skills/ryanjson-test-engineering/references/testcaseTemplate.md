# 测试任务模板（复用）

## 范围
- 适用于 RyanJson 单元测试与模糊测试的统一交付结构。
- 本模板负责“目标分支/改动清单/证据/回归”骨架；框架差异见 unity/fuzzer playbook。
- 执行入口与覆盖目录口径见 `../../shared/ryanJsonCommon.md`。

## 1) 问题与目标
- 失败现象：
- 目标分支：
- 预期语义（可恢复错误/不可恢复错误）：

## 2) 测试改动清单
- 文件：
- 用例函数：
- 场景意图：
- 所有权断言（Add/Insert vs Replace）：

## 3) 覆盖与稳定性证据
- 执行入口（unit/fuzz + 脚本）：
- 覆盖触达（true/false）：
- 泄漏结果：
- 崩溃结果：
- 覆盖率报告路径：

## 4) 回归与下一步
- 已验证边界：
- 未验证项（推断）：
- 下一批补测建议：

## 依据（仓库内）
- `test/unityTest/cases/core/testCreate.c`：Insert/AddItem 失败断言模板
- `test/unityTest/cases/core/testReplace.c`：Replace 失败不消费断言模板
- `test/fuzzer/cases/fuzzerReplace.c`：fuzz 失败语义复核模板
