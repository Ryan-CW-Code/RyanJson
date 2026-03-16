-- RyanJson build entry
set_project("RyanJson")

-- 自动生成 compile_commands.json，方便 VSCode/Clangd 做代码补全与跳转
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

-- Project path helper for included modules.
function project_path(p)
    return path.join(os.projectdir(), p)
end

-- 从环境变量读取布尔宏值，支持: true/false, 1/0, on/off（大小写不敏感）
function getBooleanEnvDefineValue(envKey, defaultValue)
    local rawValue = os.getenv(envKey)
    if nil == rawValue or "" == rawValue then
        return defaultValue
    end

    local lowerValue = rawValue:lower()
    if "true" == lowerValue or "1" == lowerValue or "on" == lowerValue then
        return "true"
    end
    if "false" == lowerValue or "0" == lowerValue or "off" == lowerValue then
        return "false"
    end

    print("warning: invalid " .. envKey .. "=" .. rawValue .. ", fallback to " .. defaultValue)
    return defaultValue
end

-- 从环境变量读取非负整数宏；不合法则打印 warning 并忽略。
function tryAddNumericEnvDefine(envKey, defineKey, minValue)
    local rawValue = os.getenv(envKey)
    if nil == rawValue or "" == rawValue then
        return
    end

    local numericValue = tonumber(rawValue)
    if nil == numericValue then
        print("warning: invalid " .. envKey .. "=" .. rawValue .. ", ignored")
        return
    end

    numericValue = math.floor(numericValue)
    if numericValue < minValue then
        print("warning: invalid " .. envKey .. "=" .. rawValue .. ", expected >= " .. tostring(minValue) .. ", ignored")
        return
    end

    add_defines(defineKey .. "=" .. tostring(numericValue))
end

-- 把 yyjson 单独编译为静态库，避免每次业务宏变化都重编译其大体积源码
-- 说明：
-- 1) yyjson 基本不依赖 RyanJson 的业务宏，拆分后可显著减少重复编译。
-- 2) binary 目标通过 add_deps("yyjsonStatic") 链接该静态库。

target("yyjsonStatic", function()
    set_kind("static")
    set_default(false)

    set_toolchains("clang")
    set_plat("linux")
    set_arch("x86")
    set_languages("gnu99")
    set_optimize("fastest")

    add_includedirs(project_path("test/externalModule/yyjson"), {public = true})
    add_files(project_path("test/externalModule/yyjson/*.c"), {public = true}, {cxflags = "-w"})
end)

-- Build targets.
includes("xmake/host.lua")
includes("xmake/qemu.lua")
