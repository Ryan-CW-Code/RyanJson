-- 自动生成 compile_commands.json，方便 VSCode/Clangd 做代码补全与跳转
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

target("RyanJson", function()
    -- 目标类型：二进制可执行文件
    set_kind("binary")

    -- 编译工具链与平台配置
    -- set_toolchains("gcc")  -- 使用 GCC
    set_toolchains("clang")   -- 使用 Clang 编译器
    set_plat("linux")         -- 平台：Linux
    set_arch("x86")           -- 架构：x86（32位）
    set_languages("gnu99")    -- 使用 GNU C99 标准，启用 GNU 扩展

    -- 定义宏：启用 Fuzzer 功能
    -- Fuzzer 与覆盖率相关编译/链接选项
    add_defines("isEnableFuzzer")
    add_cxflags("-fsanitize=fuzzer", {force = true})
    add_ldflags("-fsanitize=fuzzer", {force = true})
    add_cxflags("-fprofile-instr-generate", "-fcoverage-mapping", {force = true})
    add_ldflags("-fprofile-instr-generate", "-fcoverage-mapping", {force = true})

    -- 编译优化策略
    set_policy("build.ccache", false) -- 禁用 ccache 缓存
    set_optimize("fastest")           -- 使用 -O3，最高级别优化

    -- 警告设置：启用所有警告（Clang 下相当于 -Weverything）
    set_warnings("everything")

    -- 链接器安全硬化与优化选项
    add_ldflags(
        "-flto",                    -- 启用 LTO（链接时优化）
        "-fPIE",                    -- 位置无关可执行
        "-pie",                     -- 与 -fPIE 搭配，启用 ASLR
        "-fno-omit-frame-pointer",  -- 保留帧指针，便于调试和崩溃分析
        "-fstack-clash-protection", -- 栈碰撞保护
        "-Wl,-z,relro",             -- 重定位表只读
        "-Wl,-z,now",               -- 立即绑定符号
        "-Wl,-z,noexecstack",       -- 栈不可执行
        "-Wl,-z,separate-code",     -- 代码段与数据段分离
        {force = true}
    )

    -- Sanitizer 检测项：运行时错误检测
    add_cxflags("-fsanitize=alignment", "-fno-sanitize-recover=undefined", {force = true})
    add_ldflags(
        "-fsanitize=alignment",             -- 检查未对齐访问
        "-fno-sanitize-recover=undefined",  -- 遇到未定义行为立即终止
        "-fsanitize=address",               -- 内存越界、释放后使用
        "-fsanitize=leak",                  -- 内存泄漏
        "-fsanitize=undefined",             -- 常见未定义行为
        "-fsanitize=pointer-compare",       -- 无效指针比较
        "-fsanitize=pointer-subtract",      -- 无效指针相减
        "-fsanitize=bounds",                -- 数组越界
        "-fsanitize=float-divide-by-zero",  -- 浮点除零
        "-fsanitize=float-cast-overflow",   -- 浮点转整数溢出
        -- "-fsanitize=thread",      -- 多线程数据竞争
        -- "-fsanitize=memory",      -- 未初始化内存使用
        -- "-fsanitize=safe-stack",  -- 栈分离机制
        -- "-fsanitize=cfi",         -- 控制流完整性（需 LTO 与 Clang）
        {force = true}
    )

    -- 编译器警告与静态分析
    add_cxflags(
        "-g3",                     -- 生成详细的调试信息
        "-pedantic",               -- 严格遵循 ISO C 标准
        "-Wall",                   -- 常见警告
        "-Wextra",                 -- 额外警告
        "-Wconversion",            -- 隐式类型转换风险
        "-Wsign-conversion",       -- 有符号/无符号转换风险
        "-Wdouble-promotion",      -- float 自动提升为 double
        "-Wstrict-prototypes",     -- 函数声明必须带参数类型
        "-Wold-style-definition",  -- 检测旧式函数定义
        "-Wimplicit-fallthrough",  -- switch/case 未显式 fallthrough
        "-Wshadow",                -- 局部变量遮蔽
        "-Wcast-align",            -- 类型转换可能导致未对齐
        "-Wpointer-arith",         -- 指针运算风险
        "-Warray-bounds",          -- 数组越界访问
        "-Wshift-overflow",        -- 位移溢出
        "-Wformat-truncation",     -- 格式化字符串截断风险
        "-Walloc-size",            -- 分配大小问题
        "-Wnull-dereference",      -- 空指针解引用
        "-Wtautological-compare",  -- 恒真/恒假的比较
        "-Wstrict-overflow",       -- 有符号溢出优化假设
        "-Wmissing-prototypes",    -- 全局函数未在头文件声明
        "-Wmissing-declarations",  -- 全局变量/函数未声明
        "-Wredundant-decls",       -- 重复声明
        "-Wunreachable-code",      -- 不可达代码
        "-Wtype-limits",           -- 比较恒真/恒假的表达式（如 unsigned < 0）
        "-Wshift-negative-value",  -- 对负数移位
        "-Wdiv-by-zero",           -- 除以零（编译期可分析）
        "-Wformat-security",       -- 格式化字符串安全问题
        "-Wdisabled-optimization", -- 被禁用的优化
        "-Wreturn-local-addr",     -- 返回局部变量地址
        "-Wdeprecated",            -- 使用已弃用特性
        -- "-Wunsafe-buffer-usage",   -- 不安全的数组/指针用法
        "-Wuninitialized",         -- 使用未初始化变量
        "-fstack-protector-strong",-- 栈保护
        "-Wmissing-include-dirs",  -- 头文件目录缺失
        "-Wcast-qual",             -- 丢弃 const/volatile 限定符
        "-Wconditional-uninitialized", -- 条件路径未初始化
        "-Wcovered-switch-default",    -- default 覆盖所有枚举值
        "-Wformat-nonliteral",     -- 非字面量格式串
        "-Wformat-signedness",     -- 格式化与符号性不匹配
        "-Wvla",                   -- 可变长度数组
        "-fno-common",             -- 禁止旧式多重定义
        "-fno-strict-aliasing",        -- 禁止严格别名优化，减少别名相关 UB 风险
        "-Wdocumentation",
        "-Wparentheses-equality",
        "-Wno-documentation",      -- 临时关闭文档警告
        -- "-Wno-parentheses-equality", -- 临时关闭括号比较警告
        "-Wno-extra-semi-stmt",     -- 关闭分号警告
        "-Wno-unsafe-buffer-usage", -- 关闭不安全的数组/指针用法警告
        "-Wno-declaration-after-statement", -- 关闭声明在语句后的警告
        "-Wno-padded",              -- 关闭结构体填充警告
        "-Wno-switch-default",      -- 关闭 switch 语句缺少 default 的警告
        "-Wno-unused-macros",       -- 关闭未使用的宏定义警告
        "-Wno-unused-includes",     -- 关闭未使用的头文件警告
        {force = true}
    )

    -- 公共头文件目录
    add_includedirs('./RyanJson', {public = true})
    add_includedirs('./example', {public = true})
    add_includedirs('./test/fuzzer', {public = true})
    add_includedirs('./test', {public = true})
    add_includedirs('./test/baseTest', {public = true})
    add_includedirs('./test/baseTest/equality', {public = true})
    add_includedirs('./test/externalModule/valloc', {public = true})
    add_includedirs('./test/externalModule/tlsf', {public = true})
    add_includedirs('./test/externalModule/cJSON', {public = true})
    add_includedirs('./test/externalModule/yyjson', {public = true})

    -- 源文件分开列出，保持清晰结构
    add_files('./RyanJson/*.c', {public = true})
    add_files('./example/*.c', {public = true})
    add_files('./test/fuzzer/*.c', {public = true})
    add_files('./test/*.c', {public = true}, {cxflags = "-w"})          -- 测试代码，关闭警告
    add_files('./test/baseTest/*.c', {public = true}, {cxflags = "-w"}) -- 基础测试，关闭警告
    add_files('./test/baseTest/equality/*.c', {public = true}, {cxflags = "-w"}) -- 一致性测试
    add_files('./test/externalModule/valloc/*.c', {public = true}, {cxflags = "-w"})   -- valloc，关闭警告
    add_files('./test/externalModule/tlsf/*.c', {public = true}, {cxflags = "-w"})     -- tlsf，关闭警告
    add_files('./test/externalModule/cJSON/*.c', {public = true}, {cxflags = "-w"}) -- 第三方库 cJSON，关闭警告
    add_files('./test/externalModule/yyjson/*.c', {public = true}, {cxflags = "-w"}) -- 第三方库 yyjson，关闭警告
end)
