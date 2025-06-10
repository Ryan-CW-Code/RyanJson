add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
target("RyanJson",function()
    set_kind("binary")

    set_toolchains("gcc")  -- 确保使用 GCC
    set_languages("gnu99") -- 关键！启用 GNU 扩展

    set_optimize("smallest")

    add_cxflags(     
                "-pedantic",  
                "-Wall",
                -- "-Wextra",
                {force=true})

    --加入代码和头文件
    add_includedirs('./RyanJson', {public = true})
    add_files('./RyanJson/*.c', {public = true})

    add_includedirs('./cJSON', {public = true})
    add_includedirs('./yyjson', {public = true})
    add_includedirs('./RyanJsonExample', {public = true})
    add_includedirs('./RyanJsonExample/valloc', {public = true})
    add_files('./cJSON/*.c', {public = true})
    add_files('./yyjson/*.c', {public = true})
    add_files('./RyanJsonExample/*.c', {public = true})
    add_files('./RyanJsonExample/valloc/*.c', {public = true})

end)