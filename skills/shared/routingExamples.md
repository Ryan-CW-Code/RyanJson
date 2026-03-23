# RyanJson 路由样例（真实提问版）

## 1. 作用
- 用真实提问样式演示“这个问题应该先走哪个 skill”。
- 适用于边界模糊、同一请求同时包含多个目标的场景。

## 2. 判主 skill 的原则
- 先看用户要的主要结果，不看表面关键词。
- 如果请求同时包含“主问题 + 配套动作”，先处理主问题，再转次 skill。
- 如果用户还没理解仓库，就先补项目心智，不直接下钻细节。

## 3. 真实提问样例
| 用户提问 | 主 skill | 次 skill | 原因 |
|---|---|---|---|
| 我第一次接触 RyanJson，这个仓库先看哪几个文件？ | `ryanjson-project-guide` | 无 | 主目标是建立项目心智与入口顺序。 |
| RT-Thread 下最小接入 RyanJson 应该怎么初始化？ | `ryanjson-api-usage` | `ryanjson-project-guide` | 主目标是接入 API；若用户再问脚本/仓库入口，再转导航。 |
| `RyanJsonReplaceByKey` 失败以后新节点谁释放？ | `ryanjson-api-usage` | 无 | 主目标是公开 API 失败语义。 |
| Parse 变慢了，但我不想改公开接口，应该先看哪里？ | `ryanjson-optimization` | 无 | 主目标是内部热点分析与优化路径。 |
| `CompareOnlyKey` 这里感觉有风险，帮我评估一下。 | `ryanjson-optimization` | 无 | 主目标是内部实现审查与风险判断。 |
| Add/Insert 失败语义缺少单测，应该补在哪个文件？ | `ryanjson-test-engineering` | `ryanjson-api-usage` | 主目标是测试归属；若语义有争议，再回 API 口径。 |
| 帮我看这段 API 用法对不对，顺便补一个最小回归测试。 | `ryanjson-api-usage` | `ryanjson-test-engineering` | 先确认用法和释放口径，再决定补什么测试。 |
| 这个 Replace 路径 crash 了，我想先找根因，再补最小回归。 | `ryanjson-optimization` | `ryanjson-test-engineering` | 先判断 crash 属于实现问题还是使用问题，再收敛回归。 |
| 提交前我本地应该跑哪些脚本？ | `ryanjson-project-guide` | 无 | 主目标是仓库流程与验证入口。 |

## 4. 容易误判的情况
- 看到“测试”二字，不一定就是 `ryanjson-test-engineering`；如果用户先在问 API 语义，还是先走 `ryanjson-api-usage`。
- 看到“崩溃”二字，不一定是用户误用；如果目标是查根因与风险，先走 `ryanjson-optimization`。
- 看到“怎么做”也不一定总是 API 用法；如果用户在问仓库脚本或入口，应先走 `ryanjson-project-guide`。
