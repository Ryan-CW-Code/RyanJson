-- Host (linux) targets for unit and fuzz.

-- 统一构建配置：两个 target 共享同一套流程，仅通过 isFuzz 切换差异
local function setupRyanJsonTarget(isFuzz)
    set_kind("binary")

    -- 编译工具链与平台配置
    set_toolchains("clang")
    set_plat("linux")
    set_arch("x86")
    set_languages("gnu99")

    -- 编译优化策略
    set_policy("build.ccache", false)
    set_optimize("fastest")

    -- 警告设置：启用所有警告（Clang 下相当于 -Weverything）
    set_warnings("everything")

    -- 三个核心配置宏支持由环境变量覆盖：
    --   RYANJSON_STRICT_OBJECT_KEY_CHECK
    --   RYANJSON_DEFAULT_ADD_AT_HEAD
    --   RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC
    local strictObjectKeyCheck = getBooleanEnvDefineValue("RYANJSON_STRICT_OBJECT_KEY_CHECK", "false")
    local defaultAddAtHead = getBooleanEnvDefineValue("RYANJSON_DEFAULT_ADD_AT_HEAD", "false")
    local snprintfSupportScientific = getBooleanEnvDefineValue("RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC", "false")

    add_defines("RyanJsonStrictObjectKeyCheck=" .. strictObjectKeyCheck)
    add_defines("RyanJsonDefaultAddAtHead=" .. defaultAddAtHead)
    -- 声明 snprintf 支持科学计数法，影响 double 序列化策略
    add_defines("RyanJsonSnprintfSupportScientific=" .. snprintfSupportScientific)
    -- 启用 Linux 测试环境分支（用于主机侧测试/兼容代码路径）
    add_defines("RyanJsonLinuxTestEnv")
    add_defines("RyanJsonTestPlatformLinuxFreeRtos")
    add_defines("RyanJsonFreeRtosHeap4")
    -- 向代码注入项目根目录，供测试与样例定位资源
    add_defines("RyanJsonProjectRootPath=\"$(projectdir)\"")
    -- 让 Unity 使用项目自定义配置头
    add_defines("UNITY_INCLUDE_CONFIG_H")
    -- 测试分配模拟参数（可选）：用于评估不同 malloc 头部与对齐策略的影响。
    tryAddNumericEnvDefine("RYANJSON_TEST_ALLOC_HEADER_SIZE", "RyanJsonTestAllocHeaderSize", 0)
    tryAddNumericEnvDefine("RYANJSON_TEST_ALLOC_ALIGN_SIZE", "RyanJsonTestAllocAlignSize", 1)
    -- add_defines("RyanJsonEnableAssert")

    -- fuzz 专用差异：启用 libFuzzer 宏与链接参数（会注入 main）
    if isFuzz then
        add_defines("isEnableFuzzer")
        -- 编译期启用 libFuzzer 插桩
        add_cxflags("-fsanitize=fuzzer", {force = true})
        -- 链接期注入 libFuzzer runtime（包含 fuzz main）
        add_ldflags("-fsanitize=fuzzer", {force = true})
    end

    -- 覆盖率相关
    add_cxflags(
        "-fprofile-instr-generate", -- 生成原始 profile 数据
        "-fcoverage-mapping",       -- 生成源码到覆盖率映射
        {force = true}
    )
    add_ldflags(
        "-fprofile-instr-generate", -- 链接覆盖率 runtime
        "-fcoverage-mapping",       -- 链接覆盖率映射支持
        {force = true}
    )

    -- 链接器安全硬化与优化选项
    add_ldflags(
        "-flto",                   -- 链接时优化（跨文件优化）
        "-fPIE",                   -- 生成位置无关可执行代码
        "-pie",                    -- 启用 PIE 可执行文件
        "-fno-omit-frame-pointer", -- 保留栈帧，便于回溯定位
        "-fstack-clash-protection",-- 栈碰撞防护
        "-Wl,-z,relro",            -- 重定位表只读保护
        "-Wl,-z,now",              -- 启动时完成符号绑定
        "-Wl,-z,noexecstack",      -- 栈不可执行
        "-Wl,-z,separate-code",    -- 代码段与数据段分离
        {force = true}
    )

    add_syslinks("pthread")

    -- 主机侧统一开启 Sanitizer
    add_cxflags(
        "-fsanitize=alignment",            -- 检测未对齐内存访问
        "-fno-sanitize-recover=undefined", -- UB 触发后立即终止
        {force = true}
    )
    add_ldflags(
        "-fsanitize=alignment",            -- 未对齐访问检查
        "-fno-sanitize-recover=undefined", -- UB 触发后立即终止
        "-fsanitize=address",              -- 越界/UAF 等内存错误
        "-fsanitize=leak",                 -- 泄漏检测
        "-fsanitize=undefined",            -- 常见未定义行为检测
        "-fsanitize=pointer-compare",      -- 非法指针比较
        "-fsanitize=pointer-subtract",     -- 非法指针相减
        "-fsanitize=bounds",               -- 数组/边界检查
        "-fsanitize=float-divide-by-zero", -- 浮点除零检查
        "-fsanitize=float-cast-overflow",  -- 浮点转整型溢出检查
        {force = true}
    )

    -- 编译器警告与静态分析
    add_cxflags(
        -- 调试与标准一致性
        "-g3",                    -- 生成详细调试信息
        "-pedantic",              -- 严格遵循标准

        -- 基础与强化告警
        "-Wall",                  -- 常用告警集合
        "-Wextra",                -- 额外告警
        "-Wconversion",           -- 隐式类型转换
        "-Wsign-conversion",      -- 有/无符号转换
        "-Wdouble-promotion",     -- float 隐式提升为 double
        "-Wstrict-prototypes",    -- 函数声明原型检查
        "-Wold-style-definition", -- 旧式函数定义检查
        "-Wimplicit-fallthrough", -- switch 穿透检查
        "-Wshadow",               -- 变量遮蔽检查
        "-Wcast-align",           -- 潜在未对齐转换
        "-Wpointer-arith",        -- 指针算术风险
        "-Warray-bounds",         -- 数组越界检查
        "-Wshift-overflow",       -- 位移溢出检查
        "-Wformat-truncation",    -- 格式化截断检查
        "-Walloc-size",           -- 分配大小异常
        "-Wnull-dereference",     -- 空指针解引用
        "-Wtautological-compare", -- 恒真/恒假比较
        "-Wstrict-overflow",      -- 有符号溢出假设
        "-Wmissing-prototypes",   -- 缺少原型声明
        "-Wmissing-declarations", -- 缺少对外声明
        "-Wredundant-decls",      -- 冗余声明
        "-Wunreachable-code",     -- 不可达代码
        "-Wtype-limits",          -- 类型边界恒真比较
        "-Wshift-negative-value", -- 负值位移检查
        "-Wdiv-by-zero",          -- 除零检查
        "-Wformat-security",      -- 格式化安全检查
        "-Wdisabled-optimization",-- 被禁用优化提示
        "-Wreturn-local-addr",    -- 返回局部地址检查
        "-Wdeprecated",           -- 弃用 API 提示
        "-Wuninitialized",        -- 未初始化变量检查

        -- 代码生成与安全策略
        "-fstack-protector-strong", -- 强栈保护
        "-Wmissing-include-dirs",   -- 头文件目录缺失
        "-Wcast-qual",              -- 丢失 const/volatile 限定
        "-Wconditional-uninitialized", -- 条件分支未初始化
        "-Wcovered-switch-default", -- switch default 覆盖提示
        "-Wformat-nonliteral",      -- 非字面量格式串
        "-Wformat-signedness",      -- 格式化符号位不匹配
        "-Wvla",                    -- 可变长数组检查
        "-fno-common",              -- 禁止旧式多重定义
        "-fno-strict-aliasing",     -- 放宽别名优化，减少 UB 风险
        "-Wdocumentation",          -- 文档注释检查
        "-Wparentheses-equality",   -- 可疑括号比较

        -- 针对当前仓库的降噪项（避免第三方/历史代码噪声淹没关键告警）
        "-Wno-documentation",
        "-Wno-extra-semi-stmt",
        "-Wno-unsafe-buffer-usage",
        "-Wno-declaration-after-statement",
        "-Wno-padded",
        "-Wno-switch-default",
        "-Wno-unused-macros",
        {force = true}
    )

    -- 头文件
    add_includedirs(project_path("./RyanJson"), {public = true})
    add_includedirs(project_path("./example"), {public = true})
    add_includedirs(project_path("./test/fuzzer/include"), {public = true})
    add_includedirs(project_path("./test/unityTest/runner"), {public = true})
    add_includedirs(project_path("./test/unityTest/common"), {public = true})
    add_includedirs(project_path("./test/unityTest/include"), {public = true})
    add_includedirs(project_path("./test/unityTest/cases/core"), {public = true})
    add_includedirs(project_path("./test/unityTest/cases/scenario"), {public = true})
    add_includedirs(project_path("./test/unityTest/cases/usage"), {public = true})
    add_includedirs(project_path("./test/unityTest/cases/edge"), {public = true})
    add_includedirs(project_path("./test/unityTest/cases/stability"), {public = true})
    add_includedirs(project_path("./test/unityTest/cases/equality"), {public = true})
    add_includedirs(project_path("./test/unityTest/cases/utils"), {public = true})

    add_includedirs(project_path("./test/externalModule/valloc"), {public = true})
    add_includedirs(project_path("./test/externalModule/tlsf"), {public = true})
    add_includedirs(project_path("./test/externalModule/cJSON"), {public = true})
    add_includedirs(project_path("./test/externalModule/unity/src"), {public = true})

    add_includedirs(project_path("./test/unityTest/cases/RFC8259"), {public = true})
    -- 头文件仍由主目标暴露，源码改为依赖静态库 yyjsonStatic
    add_includedirs(project_path("./test/externalModule/yyjson"), {public = true})
    -- 依赖第三方静态库（减少重复编译）
    add_deps("yyjsonStatic")

    add_includedirs(project_path("./test/externalModule/FreeRTOS-Kernel/include"), {public = true})
    add_includedirs(project_path("./test/externalModule/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix"), {public = true})

    -- 源文件
    add_files(project_path("./RyanJson/*.c"), {public = true})
    add_files(project_path("./example/*.c"), {public = true})
    add_files(project_path("./test/unityTest/**.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/list.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/queue.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/tasks.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/timers.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/event_groups.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/stream_buffer.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/port.c"),
              {public = true},
              {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c"),
              {public = true},
              {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/portable/MemMang/heap_4.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/valloc/*.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/tlsf/*.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/cJSON/*.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/unity/src/*.c"), {public = true}, {cxflags = "-w"})

    if isFuzz then
        -- fuzz 目标：编入全部 fuzz case
        add_files(project_path("./test/fuzzer/**.c"), {public = true})
    else
        -- unit 目标：仅补最小 fuzzer runtime 依赖，避免注入 libFuzzer main。
        -- 原因：RyanJsonPrint.c 在 RyanJsonLinuxTestEnv 下会引用 RyanJsonFuzzerShouldFail。
        add_files(project_path("./test/fuzzer/utils/fuzzerDriver.c"), {public = true}, {cxflags = "-w"})
        add_files(project_path("./test/fuzzer/utils/fuzzerMemory.c"), {public = true}, {cxflags = "-w"})
    end
end

target("RyanJson", function()
    -- 默认目标：Unit 测试路径（不注入 libFuzzer main）
    setupRyanJsonTarget(false)
end)

target("RyanJsonFuzz", function()
    -- 专用 fuzz 目标：默认不参与普通 xmake 构建
    set_default(false)
    setupRyanJsonTarget(true)
end)
