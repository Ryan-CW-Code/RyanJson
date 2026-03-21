# RyanJson 仓库结构与入口地图

## 范围
- 用于快速回答“这个仓库有哪些核心目录、先看哪里”。
- 具体 API 语义、优化策略、测试归属不在本文展开。

## 一屏总览
- `README.md`：项目定位、能力概览、对外文档与本地脚本入口。
- `AGENTS.md`：AI 入口与技能路由。
- `RyanJson/`：核心库源码与头文件。
- `test/`：unit、fuzz、QEMU 与 RFC8259 数据/辅助代码。
- `example/`：最小使用示例。
- `skills/`：AI 技能、共享基线与参考文档。
- `scripts/`：公共脚本库、工具与环境准备脚本。
- `reports/`：脚本生成并被 README 引用的验证报告。
- 仓库根目录 `run_local_*.sh`：本地验证正式入口。

## 新人最短阅读路径
1. `README.md`
2. `AGENTS.md`
3. `example/`
4. 与当前问题匹配的 `run_local_*.sh` 或 skill

## 常见入口选择
- 想看对外 API：`RyanJson/RyanJson.h`
- 想看构建入口：`xmake.lua`
- 想看脚本职责：`scripts/README.md`
- 想看 AI 路由：`AGENTS.md`
- 想看项目能力与报告链接：`README.md`

## 依据（仓库内）
- `README.md`
- `AGENTS.md`
- `xmake.lua`
- `scripts/README.md`
- `RyanJson/`
- `test/`
- `example/`
- `reports/`
