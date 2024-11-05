add_requireconfs("**.glfw", {override = true, version = "3.4",configs = {shared = true,debug=true}})
add_requires("vulkansdk", "glm","assimp","tinyobjloader","lz4","nlohmann_json","gli","optick","rttr")
add_requires("imgui v1.89.9-docking", {configs = { glfw_vulkan = true, debug = true, shared = true }})
add_requires("flatbuffers v1.12.0")
add_requires("boost",{ version = "1.84.0",configs = {shared = true,debug=true,cmake=false}})
add_requires("glfw 3.4", {configs = {shared = true,debug=true}})
add_requires("glslang", {configs = {binaryonly = true}})
add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

rule("module")
    on_load(function (target)   
        if is_mode("debug") then
            target:set("kind", "static")
        elseif is_mode("release", "releasedbg") then
            target:set("kind", "shared")
            if is_plat("windows") then
                import("core.project.rule")
                local rule = rule.rule("utils.symbols.export_all")
                target:rule_add(rule)
                target:extraconf_set("rules", "utils.symbols.export_all", {export_classes = true})
            end
        else
            assert(false, "Unknown build kind")
        end
    end)
rule_end()
includes("src/**/xmake.lua")

rule("flatbufferFile")
    set_extensions(".fbs")
    --on_build_file(function (target, sourcefile, opt)
    --    print("Compiler Fbs")
    --    os.cp(sourcefile, path.join(target:targetdir(), path.basename(sourcefile)))
    --end)


function CommonLibrarySetting()
    set_languages("clatest", "cxx20")  
    add_defines("PLATFORM_WIN32")
    add_headerfiles("src/Runtime/**.h")
    add_files("src/Runtime/**.cpp")
    add_headerfiles("src/ThirdParty/**.h")
    add_files("src/ThirdParty/**.cpp")
    add_files("src/ThirdParty/**.c")
    add_includedirs("src/_Generated", {public = true})
    add_includedirs("src/Runtime", {public = true})
    add_includedirs("src/ThirdParty", {public = true})
    add_packages("vulkansdk", "glfw", "glm","assimp","tinyobjloader","imgui","lz4","nlohmann_json","gli","optick","boost","flatbuffers","rttr")
end



function CompileFunc()
    print("[compile c++ reflection code] schema gen code..")
    local folders = {
        "$(projectdir)/src/Runtime/Core",
        "$(projectdir)/src/Runtime/Platform",
        "$(projectdir)/src/Runtime/RHI",
        "$(projectdir)/src/Runtime/Asset",
        "$(projectdir)/src/Runtime/Tool",
        "$(projectdir)/src/Runtime/UI",
        "$(projectdir)/src/Runtime/Application",
        "$(projectdir)/src/Sample",
        "$(projectdir)/src/Editor"
    }
    
    local headers = {}
    for _, folder_path in ipairs(folders) do
        local files_in_folder = os.files(path.join(folder_path, "**.h"), {recursive = true})
        for _, header in ipairs(files_in_folder) do
            table.insert(headers, header)
        end
    end

    local file = io.open("$(projectdir)/src/Reflect/precompile.info", "w")
    if file then
        for _, header in ipairs(headers) do
            file:write(header, ";")
        end
        file:close()
    end
    os.exec("$(projectdir)/src/Reflect/MetaParser.exe $(projectdir)/src/Reflect/precompile.info  $(projectdir)/src/Reflect/parser_header.h E:/GameEngine/MyRenderer/src * MXRender 0")
    print("----\n")
    
    print("[compile shader] shader to spirv..")
    local vulkan_sdk = find_package("vulkansdk") --定位到vulkansdk的路径
    local glslang_validator_dir =vulkan_sdk["bindir"].."\\glslangValidator.exe" --获取到glslangValidator.exe的路径
    -- 遍历$(projectdir)/engine/shaders/中除了.spv后缀的所有文件
    for _, shader_path in ipairs(os.files("$(projectdir)/resource/Shader/**|**.spv|**.bat|**.exe|**.h|**.glsl")) do
        print("[compile shader] : "..shader_path)
        os.runv(glslang_validator_dir,{"-V", shader_path,"-o", shader_path..".spv"}) --执行系统命令
    end

    print("----\n")

    print("[compile flatbuffer] schema gen code..")
    local flatbuffer_dir = "$(projectdir)/src/Runtime/GenCode/Schema" --定位到flatbuffer flatc.exe的路径
    local flatbuffer_gen_dir = flatbuffer_dir.."/FlatbufferGen" 
    
    local flatc_dir =  flatbuffer_dir.."/flatc.exe"--flatbuffer_dir["bindir"].."\\flatc.exe" --获取到glslangValidator.exe的路径
    local flatbuffer_all_fbs_files =""
    print(flatc_dir)
    -- 遍历$(projectdir)/engine/shaders/中除了.spv后缀的所有文件
    for _, dir in ipairs(os.dirs(flatbuffer_dir.."/FlatbufferGen")) do
        print("Get Gen Dir..".. dir)
        flatbuffer_gen_dir = dir
        break
    end
    for _, flatbuffer_path in ipairs(os.files(flatbuffer_dir.."/**.fbs")) do
        flatbuffer_all_fbs_files = flatbuffer_all_fbs_files.." "..flatbuffer_path
        print("[compile flatbuffer] done: "..flatbuffer_path)
        os.runv(flatc_dir,{"-o" ,flatbuffer_gen_dir , "--cpp","--gen-object-api","--gen-compare","--gen-all","--schema","--natural-utf8","--defaults-json","--defaults-json","--gen-onefile","--reflect-types","--reflect-names", flatbuffer_path }) --执行系统命令
    end
    print("----\n")
end


function MoveResource(target)
    local root_taget_path = "$(buildir)"
    if is_plat("windows") then
        root_taget_path = root_taget_path .. "/windows"
    end
    if is_arch("x64") then
        root_taget_path = root_taget_path .. "/x64"
    end
    if is_mode("release") then
        root_taget_path = root_taget_path .. "/release"
    elseif is_mode("debug") then
        root_taget_path = root_taget_path .. "/debug"
    elseif is_mode("releasedbg") then
        root_taget_path = root_taget_path .. "/releasedbg"
    end
    local root_taget_shader_path = root_taget_path .. "/Shader"
    local root_taget_texture_path = root_taget_path .. "/Texture"
    local root_taget_editor_path = root_taget_path .. "/Editor"
    local root_taget_lib_path = root_taget_path
    os.mkdir(root_taget_shader_path)--, root_taget_texture_path,root_taget_editor_path,root_taget_lib_path)
    for _, gen_shader_pth in ipairs(os.files("$(projectdir)/resource/Shader/**.spv")) do
        print("[copy shader] done :"..gen_shader_pth)
        os.cp(gen_shader_pth, root_taget_shader_path)
    end
    os.cp("$(projectdir)/resource/Texture", root_taget_texture_path)
    os.cp("$(projectdir)/resource/Editor", root_taget_editor_path)
    os.cp("$(projectdir)/libs/*", root_taget_lib_path)
    
end

function CommonProjectSetting()
    set_kind("binary")  
    set_languages("clatest", "cxx20") 
    add_deps("Runtime")
    add_files("src/_Generated/**.cpp", {public = true}) 
    add_includedirs("src/_Generated", {public = true})
    add_includedirs("src/Runtime", {public = true})
    add_includedirs("src/ThirdParty", {public = true})
    add_packages("vulkansdk", "glfw", "glm","assimp","tinyobjloader","imgui","boost","flatbuffers","rttr")
end

target("CompileResource")
    set_kind("binary")  
    set_languages("clatest", "cxx20") 
    add_files("src/Runtime/GenCode/Schema/*.fbs", {rule = "flatbufferFile"})
    add_extrafiles("src/Runtime/GenCode/Schema/*.fbs", {build = false})
    add_extrafiles("resource/Shader/**|**.spv|**.bat|**.exe|**.glsl", {build = false})
    add_files("src/CompileResource.cpp") 
    set_group("CompileResource")
    before_build(CompileFunc)

target("Runtime")
    add_rules("module")
    add_packages("glslang")
    add_rules("utils.glsl2spv", {outputdir = "$(projectdir)/src/Runtime/GenCode/Shader",bin2c = true})
    add_files("resource/Shader/**|**.spv|**.bat|**.exe|**.h|**.glsl", {build = false})
    set_group("Runtime")
    CommonLibrarySetting()
    add_deps("CompileResource")


target("Renderer")
    CommonProjectSetting()
    add_files("src/RendererApp.cpp") 
    set_group("Sample")
    after_build(MoveResource)

target("Editor")
    CommonProjectSetting()
    add_defines("IMGUI_DEFINE_MATH_OPERATORS")
    add_defines("IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING")
    add_headerfiles("src/Editor/**.h")
    add_files("src/Editor/**.cpp") 
    add_includedirs("src/Editor")
    set_group("Editor")
    after_build(MoveResource)


target("RendererSample-HelloTriangle")
    CommonProjectSetting()
    add_files("src/Sample/1-HelloTriangle/HelloTriangle.cpp")
    set_group("Sample")
    after_build(MoveResource)

target("RendererSample-Texture")
    CommonProjectSetting()
    add_files("src/Sample/2-Texture/Texture.cpp")
    set_group("Sample")
    after_build(MoveResource)

target("RendererSample-CubeMap")
    CommonProjectSetting()
    add_files("src/Sample/3-CubeMap/CubeMap.cpp")
    set_group("Sample")
    after_build(MoveResource)


target("RendererSample-VirtualTexture")
    CommonProjectSetting()
    add_files("src/Sample/N-VirtualTexture/VirtualTexture.cpp")
    set_group("Sample")
    after_build(MoveResource)

target("RendererSample-Reflection")
    CommonProjectSetting()
    add_headerfiles("src/Sample/4-Reflect/**.h")
    add_files("src/Sample/4-Reflect/**.cpp")
    add_files("src/Sample/4-Reflect/Reflect.cpp")
    set_group("Sample")
    after_build(MoveResource)

