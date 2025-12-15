add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
target("RyanJson",function()
    set_kind("binary")

    -- set_toolchains("gcc")  -- 确保使用 GCC
    set_toolchains("clang")
    set_plat("linux")
    set_arch("x86")
    set_languages("gnu99") -- 关键！启用 GNU 扩展
    add_defines("isEnableFuzzer")
    add_cxflags("-fsanitize=fuzzer", "-fprofile-instr-generate", "-fcoverage-mapping", {force = true} )
    add_ldflags("-fsanitize=fuzzer", "-fprofile-instr-generate", "-fcoverage-mapping", {force = true} )

    set_policy("build.ccache", false)
    -- set_optimize("smallest") -- -Os
    -- set_optimize("faster") -- -O2
    set_optimize("fastest") -- -O3
    -- set_optimize("aggressive") -- -Ofast

    -- 启用全部警告
    -- set_warnings("everything") -- -Wall -Wextra -Weffc++ / -Weverything

    -- 链接器选项：生成 map 文件
    -- add_ldflags("-Wl,-Map=$(buildir)/RyanJson.map")

    -- 开启库加固（需与 -O2 以上配合）
    -- add_defines("_FORTIFY_SOURCE=2")   -- glibc 格式/内存函数加固

    -- 链接器安全硬化与优化
    add_ldflags(
        -- "-flto",                    -- 链接时优化（启用 LTO，便于 CFI 等）
        -- "-fPIE",                    -- 位置无关可执行
        -- "-pie",                     -- 与 -fPIE 搭配，启用 ASLR
        -- "-fno-omit-frame-pointer",  -- 保留帧指针，便于崩溃分析
        -- "-fstack-clash-protection", -- 栈碰撞保护（平台支持时有效）
        -- "-Wl,-z,relro",             -- 只读重定位（硬化）
        -- "-Wl,-z,now",               -- 立即绑定（与 relro 搭配）
        -- "-Wl,-z,noexecstack",       -- 栈不可执行
        -- "-Wl,-z,separate-code",     -- 代码与数据段分离
        {force = true}
    )

    -- Sanitizer 检测项（运行时错误）
    add_ldflags(
        "-fsanitize=address", -- 内存越界、释放后使用
        "-fsanitize=leak", -- 内存泄漏
        "-fsanitize=undefined", -- 未定义行为（除零、溢出、无效移位等）
        "-fsanitize=pointer-compare", -- 无效指针比较
        "-fsanitize=pointer-subtract", -- 无效指针相减
        "-fsanitize=bounds", -- 数组越界
        "-fsanitize=float-divide-by-zero", -- 浮点除零
        "-fsanitize=float-cast-overflow", -- 浮点转整数溢出
        -- "-fsanitize=thread",      -- 多线程数据竞争
        -- "-fsanitize=memory",      -- 未初始化内存使用
        -- "-fsanitize=safe-stack",  -- 栈分离机制
        -- "-fsanitize=cfi",         -- 控制流完整性（需 LTO 与 Clang）
        -- "-fsanitize=alignment", -- 检测未对齐的内存访问
        -- "-fno-sanitize=alignment", -- 某些平台不兼容
        {force = true}
    )

    -- 编译器警告与静态分析（开发期错误检测，Clang 兼容）
    add_cxflags(
        "-g3",                  -- 生成调试信息"
        "-pedantic",               -- 强制遵循 ISO C 标准
        "-Wall",                   -- 常见警告
        "-Wextra",                 -- 额外警告
        "-Wconversion",            -- 隐式类型转换风险
        "-Wsign-conversion",       -- 有符号/无符号转换风险
        "-Wdouble-promotion",      -- float 自动提升为 double
        "-Wstrict-prototypes",     -- 函数声明必须带参数类型
        "-Wold-style-definition",  -- 检测旧式函数定义
        "-Wimplicit-fallthrough",  -- switch/case 未显式 fallthrough
        "-Wshadow",                -- 局部变量遮蔽
        "-Wcast-align",            -- 类型转换对齐问题
        "-Wpointer-arith",         -- 指针运算风险
        "-Warray-bounds",          -- 数组越界访问
        "-Wshift-overflow",        -- 位移造成的溢出
        "-Wformat-truncation",     -- 格式化字符串被截断风险（替代 stringop-truncation）
        "-Walloc-size",            -- 分配大小问题（替代 alloc-zero）
        "-Wnull-dereference",      -- 空指针解引用
        "-Wtautological-compare",  -- 恒真/恒假的比较
        "-Wstrict-overflow",       -- 有符号溢出优化假设
        "-Wmissing-prototypes",    -- 全局函数未在头文件声明
        "-Wmissing-declarations",  -- 全局变量/函数未声明
        "-Wredundant-decls",       -- 重复声明
        "-Wunreachable-code",      -- 不可达代码
        "-Wtype-limits",           -- 比较恒真/恒假的表达式（如 unsigned < 0）
        "-Wshift-negative-value",  -- 对负数进行移位
        "-Wdiv-by-zero",           -- 除以零（编译期可分析）
        "-Wformat-security",       -- 格式化字符串安全问题
        "-Wdisabled-optimization", -- 被禁用的优化
        "-Wreturn-local-addr",     -- 返回局部变量地址
        "-Wdeprecated",            -- 使用已弃用的特性
        -- "-Wunsafe-buffer-usage",   -- 不安全的数组/指针用法（Clang 新增）
        "-Wuninitialized",         -- 使用未初始化变量
        "-fstack-protector-strong",-- 栈保护
        -- 进一步增强（可选，按需开启）
        "-Wmissing-include-dirs",      -- 头文件目录缺失
        "-Wcast-qual",                 -- 丢弃 const/volatile 限定符的转换
        "-Wconditional-uninitialized", -- 条件路径未初始化
        "-Wcovered-switch-default",    -- default 覆盖所有枚举值
        "-Wformat-nonliteral",         -- 非字面量格式串
        "-Wformat-signedness",         -- 格式化与符号性不匹配
        "-Wvla",                       -- 可变长度数组（不安全/不建议使用）
        "-fno-common",                 -- 禁止旧式多重定义（链接期更严格）
        "-fno-strict-aliasing",        -- 禁止严格别名优化，减少别名相关 UB 风险
        "-Wno-documentation", -- 临时变比
        "-Wno-parentheses-equality",
        {force = true}
    )

    add_includedirs('./test/fuzzer', {public = true})
    add_files('./test/fuzzer/*.c', {public = true})

    --加入代码和头文件
    add_includedirs('./RyanJson', {public = true})
    add_files('./RyanJson/*.c', {public = true})

    add_includedirs('./example', {public = true})
    add_includedirs('./test', {public = true})
    add_includedirs('./test/valloc', {public = true})
    add_includedirs('./test/baseTest', {public = true})
    add_includedirs('./externalModule/cJSON', {public = true})
    -- add_includedirs('./externalModule/yyjson', {public = true})

    add_files('./example/*.c', {public = true})
    add_files('./test/*.c', {public = true}, {cxflags = "-w"})
    add_files('./test/valloc/*.c', {public = true}, {cxflags = "-w"})
    add_files('./test/baseTest/*.c', {public = true}, {cxflags = "-w"})
    add_files('./externalModule/cJSON/*.c', {public = true}, {cxflags = "-w"})
    -- add_files('./externalModule/yyjson/*.c', {public = true})

end)