# 回归、覆盖与验证矩阵

## 必查语义

- Parse 成功/失败、默认尾部行为与 `ParseOptions` 严格模式。
- Add、Insert、Replace、Detach、Delete 的所有权与失败恢复。
- Compare、CompareOnlyKey、重复 key 与宏分支。
- Minify 终止符：返回值小于输入长度时写 `\0`，相等时不额外写入。
- 字符串边界、打印容量、数值格式、链表结构和所有改动直接影响的契约。

## 执行分层

| 阶段 | 目的 | 入口 |
| --- | --- | --- |
| 定向 | 快速证明改动所属 runner | 使用目标 runner 或最小 build |
| quick unit | 每轮增量反馈 | `UNIT_MODE=quick UNIT_SKIP_COV=1 bash run_local_base.sh` |
| full unit | 合入前的确定性门禁 | `UNIT_MODE=full bash run_local_base.sh` |
| QEMU | 对齐、FreeRTOS、fault 或硬件语义 | `bash run_local_qemu.sh` |
| fuzz | 异常输入、解析/打印/分配与历史 crash | `FUZZ_MODE=quick FUZZ_SKIP_COV=1 bash run_local_fuzz.sh` |

## 覆盖判断

- 先看目标 true/false 分支是否触达，而不是只看总百分比。
- 新防御分支必须有可追溯触达证据。
- 覆盖下降时，与稳定基线或暂存变化比较；解释是主动删除逻辑、遗漏运行期触发，还是报告环境差异。
- unit 覆盖在 `localLogs/unitMatrix/`，fuzz 覆盖在 `test/fuzzer/coverage/`；两者不能互相替代。

## 交付证据

说明测试文件/函数、证明的独立契约、执行模式、命令、结果、泄漏或 sanitizer 状态、报告路径以及仍未覆盖的独立边界。
