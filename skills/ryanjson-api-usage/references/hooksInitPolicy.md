# RyanJsonInitHooks 初始化规范（RT-Thread 导向）

## 范围
- 本页只定义 hooks 初始化策略与上板检查点。
- 通用 API 调用顺序见 `quickstart.md`。

## 规则
- `RyanJsonInitHooks` 是 RyanJson 前置条件，必须在任何 API 前调用。
- 建议在系统启动阶段只初始化一次。
- 初始化失败必须可观测（日志/错误码），并阻断后续 Json 逻辑。

## RT-Thread 推荐位置
- 应用初始化阶段（如 `INIT_APP_EXPORT`）。
- 或 Json owner 线程启动阶段（需保证先于业务调用）。

## 实现建议
- 映射 `rt_malloc/rt_free/rt_realloc` 或自定义内存池封装。
- 保证 `realloc` 失败返回 `NULL` 且不破坏原指针语义。
- 建议记录分配失败次数和峰值占用，便于板端排障。

## 线程模型建议
- 库本身不承诺线程安全。
- 推荐单 owner 线程独占 Json 树。
- 多线程共享时，用外层锁保护完整事务。

## 上板检查清单
- hooks 是否在首次 Parse/Create 前成功执行。
- hooks 失败时是否正确阻断业务路径。
- 常用 API 成功/失败路径是否都能返回并正确释放。

## 依据（仓库内）
- `RyanJson/RyanJson.c`：`jsonMalloc/jsonFree/jsonRealloc` 为全局 hooks，默认 `NULL`
- `test/unityTest/common/testCommon.c`：测试入口先 `RyanJsonInitHooks(...)`
- `test/fuzzer/entry.c`：`LLVMFuzzerTestOneInput` 内先校验并初始化 hooks
