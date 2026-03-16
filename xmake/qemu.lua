-- QEMU Cortex-M targets for hardware semantics validation.

-- QEMU Cortex-M 目标：用于硬件语义校验（含非对齐访问 fault）
local function setupRyanJsonQemuTarget(options)
    local cpu = options.cpu
    local freertosPort = options.freertosPort
    local isCm4f = options.isCm4f

    set_kind("binary")
    set_default(false)
    set_extension(".elf")

    set_toolchains("gcc")
    set_plat("cross")
    set_arch("arm")
    set_languages("gnu99")
    set_optimize("fastest")
    set_symbols("debug")

    set_toolset("cc", "arm-none-eabi-gcc")
    set_toolset("as", "arm-none-eabi-gcc")
    set_toolset("ld", "arm-none-eabi-gcc")
    set_toolset("ar", "arm-none-eabi-ar")

    local strictObjectKeyCheck = getBooleanEnvDefineValue("RYANJSON_STRICT_OBJECT_KEY_CHECK", "false")
    local defaultAddAtHead = getBooleanEnvDefineValue("RYANJSON_DEFAULT_ADD_AT_HEAD", "false")
    local snprintfSupportScientific = getBooleanEnvDefineValue("RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC", "true")

    add_defines("RyanJsonStrictObjectKeyCheck=" .. strictObjectKeyCheck)
    add_defines("RyanJsonDefaultAddAtHead=" .. defaultAddAtHead)
    add_defines("RyanJsonSnprintfSupportScientific=" .. snprintfSupportScientific)
    add_defines("RyanJsonTestPlatformQemu")
    add_defines("RyanJsonFreeRtosHeap4")
    add_defines("RyanJsonProjectRootPath=\"$(projectdir)\"")
    add_defines("UNITY_INCLUDE_CONFIG_H")
    add_defines("YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS=1")
    -- 测试分配模拟参数（可选）：用于评估不同 malloc 头部与对齐策略的影响。
    tryAddNumericEnvDefine("RYANJSON_TEST_ALLOC_HEADER_SIZE", "RyanJsonTestAllocHeaderSize", 0)
    tryAddNumericEnvDefine("RYANJSON_TEST_ALLOC_ALIGN_SIZE", "RyanJsonTestAllocAlignSize", 1)

    add_cxflags(
        "-mcpu=" .. cpu,
        "-mthumb",
        "-ffunction-sections",
        "-fdata-sections",
        "-fno-common",
        "-fno-strict-aliasing",
        "-Wall",
        "-Wextra",
        "-Wno-unused-parameter",
        {force = true}
    )

    if isCm4f then
        add_cxflags(
            "-mfpu=fpv4-sp-d16",
            "-mfloat-abi=hard",
            {force = true}
        )
    end

    add_asflags(
        "-mcpu=" .. cpu,
        "-mthumb",
        {force = true}
    )

    if isCm4f then
        add_asflags(
            "-mfpu=fpv4-sp-d16",
            "-mfloat-abi=hard",
            {force = true}
        )
    end

    local linkerScript = project_path("test/qemu/platform/linkerMps2An386.ld")
    add_ldflags(
        "-mcpu=" .. cpu,
        "-mthumb",
        "-T" .. linkerScript,
        "-Wl,--gc-sections",
        "-Wl,--print-memory-usage",
        "--specs=nosys.specs",
        "-nostartfiles",
        {force = true}
    )

    if isCm4f then
        add_ldflags(
            "-mfpu=fpv4-sp-d16",
            "-mfloat-abi=hard",
            {force = true}
        )
    end

    add_syslinks("c", "m", "gcc")

    add_includedirs(project_path("./RyanJson"), {public = true})
    add_includedirs(project_path("./test/qemu/platform"), {public = true})
    add_includedirs(project_path("./test/qemu/common"), {public = true})
    add_includedirs(project_path("./example"), {public = true})
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
    add_includedirs(project_path("./test/unityTest/cases/RFC8259"), {public = true})
    add_includedirs(project_path("./test/unityTest/cases/performance"), {public = true})
    add_includedirs(project_path("./test/externalModule/valloc"), {public = true})
    add_includedirs(project_path("./test/externalModule/tlsf"), {public = true})
    add_includedirs(project_path("./test/externalModule/cJSON"), {public = true})
    add_includedirs(project_path("./test/externalModule/unity/src"), {public = true})
    add_includedirs(project_path("./test/externalModule/yyjson"), {public = true})
    add_includedirs(project_path("./test/externalModule/FreeRTOS-Kernel/include"), {public = true})
    add_includedirs(project_path("test/externalModule/FreeRTOS-Kernel/portable/GCC/" .. freertosPort), {public = true})

    add_files(project_path("./RyanJson/*.c"), {public = true})
    add_files(project_path("./example/*.c"), {public = true})
    add_files(project_path("./test/qemu/platform/qemuPlatform.c"), {public = true})
    add_files(project_path("./test/qemu/platform/qemuStartup.c"), {public = true})
    add_files(project_path("./test/qemu/platform/qemuFault.c"), {public = true})
    add_files(project_path("./test/qemu/platform/qemuSyscalls.c"), {public = true})
    add_files(project_path("./test/qemu/platform/qemuFreertosHeap.c"), {public = true})
    add_files(project_path("./test/unityTest/**.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/valloc/*.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/tlsf/*.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/cJSON/*.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/unity/src/*.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/yyjson/*.c"), {public = true}, {cxflags = "-w"})

    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/list.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/queue.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/tasks.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/timers.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/event_groups.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/stream_buffer.c"), {public = true}, {cxflags = "-w"})
    add_files(project_path("test/externalModule/FreeRTOS-Kernel/portable/GCC/" .. freertosPort .. "/port.c"),
              {public = true},
              {cxflags = "-w"})
    add_files(project_path("./test/externalModule/FreeRTOS-Kernel/portable/MemMang/heap_4.c"), {public = true}, {cxflags = "-w"})

    after_build(function(target)
        local elfFile = target:targetfile()
        local outDir = path.directory(elfFile)
        local baseName = path.basename(elfFile)
        local binFile = path.join(outDir, baseName .. ".bin")
        local hexFile = path.join(outDir, baseName .. ".hex")

        os.execv("arm-none-eabi-objcopy", {"-O", "binary", elfFile, binFile})
        os.execv("arm-none-eabi-objcopy", {"-O", "ihex", elfFile, hexFile})
    end)
end

target("RyanJsonQemu", function()
    setupRyanJsonQemuTarget({
        cpu = "cortex-m4",
        freertosPort = "ARM_CM4F",
        isCm4f = true,
    })
end)

target("RyanJsonQemuCm3", function()
    setupRyanJsonQemuTarget({
        cpu = "cortex-m3",
        freertosPort = "ARM_CM3",
        isCm4f = false,
    })
    add_defines("RyanJsonQemuSoftUnalignedTrap")
end)
