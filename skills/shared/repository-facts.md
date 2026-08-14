# RyanJson 仓库事实

## 仓库形态

- [README.md](../../README.md) 是项目对外总览；[example/](../../example/) 放应用示例。
- [RyanJson/](../../RyanJson/) 是 C 库源码；[test/](../../test/) 包含 Unity、fuzz、QEMU 与 RFC8259 资产。
- [xmake.lua](../../xmake.lua) 是主机侧构建入口；`Makefile` 是历史辅助，[SConscript](../../SConscript) 用于 RT-Thread 软件包集成。
- 根目录 `run_local_*.sh` 是正式本地入口；[scripts/README.md](../../scripts/README.md) 说明其公共逻辑与生成工具。

## 构建与测试模式

| 模式 | Target | 目的 | 优先命令 |
| --- | --- | --- | --- |
| unit | `RyanJson` | 确定性 Unity 契约；不注入 libFuzzer main | `xmake -b RyanJson` 或 `./run_local_base.sh` |
| fuzz | `RyanJsonFuzz` | LibFuzzer 执行与覆盖 | `xmake -b RyanJsonFuzz` 或 `./run_local_fuzz.sh` |
| QEMU | `RyanJsonQemu` / `RyanJsonQemuCm3` | FreeRTOS Cortex-M 与非对齐访问 fault 语义 | `xmake -b RyanJsonQemu` 或 `./run_local_qemu.sh` |

- QEMU 会开启 `SCB->CCR.UNALIGN_TRP=1`。禁止通过 `-mno-unaligned-access` 掩盖真实对齐错误；第三方 `YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS=1` 保持局部防御，除非任务明确涉及它。
- `run_local_base.sh` 会生成 `test/unityTest/runner/test_list.inc`；仅更新 runner 时使用 `UNIT_SYNC_ONLY=1`。

## 支持的本地入口

| 脚本 | 常规用途 |
| --- | --- |
| `./run_local_base.sh` | unit 矩阵；默认 `UNIT_MODE=full` 且跳过覆盖率。 |
| `./run_local_fuzz.sh` | fuzz；默认 100000 runs、1 worker、9 jobs。 |
| `./run_local_qemu.sh` | 完整 QEMU 矩阵与非对齐 fault 校验。 |
| `./run_local_ci.sh` | 本地 `ci-pr`：full unit + quick fuzz。 |
| `./run_local_format.sh --check` | 格式校验。 |
| `./run_local_memory.sh` | host/QEMU 内存报告。 |
| `./run_local_rfc8259.sh` | host/QEMU RFC8259 报告。 |

## 生成产物

- unit 覆盖率在 `localLogs/unitMatrix/` 每次重新生成。
- fuzz 覆盖率在 `test/fuzzer/coverage/` 每次重新生成。
- [reports/](../../reports/) 下的 memory 与 RFC8259 报告是 README 引用的生成文档，不是临时文件。
- [scripts/generators/gen_rfc8259_embedded.py](../../scripts/generators/gen_rfc8259_embedded.py) 将 `test/data/rfc8259/` 嵌入源码；修改数据后需运行它。

## 证据入口

- [xmake.lua](../../xmake.lua)
- [scripts/README.md](../../scripts/README.md)
- [run_local_base.sh](../../run_local_base.sh)
- [run_local_fuzz.sh](../../run_local_fuzz.sh)
- [run_local_qemu.sh](../../run_local_qemu.sh)
