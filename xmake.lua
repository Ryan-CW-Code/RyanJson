target("RyanJson",function()
    set_kind("static")

    description_csdk()

    --加入代码和头文件
    add_includedirs('./RyanJson', {public = true})

    add_files('./RyanJson/*.c', {public = true})

end)