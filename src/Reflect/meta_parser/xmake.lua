
add_requires("llvm" , {version = "18.1.1",configs = {all = true,debug=true}})


target("MetaParser")
    set_kind("binary")  
    set_languages("clatest", "cxx20") 
    set_group("Reflect")

    add_files("parser/main.cpp")

    add_includedirs("3rd_party/LLVM/include")
    add_includedirs("3rd_party/mustache")
    add_includedirs("parser")
    add_headerfiles("3rd_party/**.hpp")
    add_headerfiles("3rd_party/**.h")
    add_headerfiles("parser/**.hpp")
    add_headerfiles("parser/**.h")
    add_files("parser/**.cpp")     
    add_linkdirs("$(projectdir)/src/Reflect/meta_parser/3rd_party/LLVM/lib/x64")
    add_links("$(projectdir)/src/Reflect/meta_parser/3rd_party/LLVM/lib/x64/libclang.lib")