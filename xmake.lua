add_requires("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","rttr","lz4","nlohmann_json","gli","optick","boost","flatbuffers")
add_requires("imgui v1.90.4-docking", {configs = {glfw_vulkan = true, debug = true, shared = true }})
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


function CommonLibrarySetting()
    set_languages("c++20")  
    add_defines("PLATFORM_WIN32")
    add_headerfiles("src/Runtime/**.h")
    add_files("src/Runtime/**.cpp")
    add_headerfiles("src/ThirdParty/**.h")
    add_files("src/ThirdParty/**.cpp")
    add_files("src/ThirdParty/**.c")
    add_includedirs("src/Runtime")
    add_includedirs("src/ThirdParty")
    add_packages("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","imgui","rttr","lz4","nlohmann_json","gli","optick","boost","flatbuffers")
end

target("Runtime")
    add_rules("module")
    CommonLibrarySetting()


function CompileShader(target)
    os.run("$(projectdir)/resource/Shader/compile-glslangValidator.bat")
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
    os.cp("$(projectdir)/resource/Shader", root_taget_shader_path)
    os.cp("$(projectdir)/resource/Texture", root_taget_texture_path)
    os.cp("$(projectdir)/resource/Editor", root_taget_editor_path)
    os.cp("$(projectdir)/libs/*", root_taget_lib_path)
    
end

function CommonProjectSetting()
    set_kind("binary")  
    set_languages("c++20")  
    add_deps("Runtime")
    add_includedirs("src/Runtime")
    add_includedirs("src/ThirdParty")
    add_packages("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","imgui","boost","rttr")
end

target("Renderer")
    CommonProjectSetting()
    add_files("src/RendererApp.cpp") 
    before_build(CompileShader)
    before_build(MoveResource)

target("Editor")
    CommonProjectSetting()
    add_defines("IMGUI_DEFINE_MATH_OPERATORS")
    add_defines("IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING")
    add_headerfiles("src/Editor/**.h")
    add_files("src/Editor/**.cpp")
    add_includedirs("src/Editor")
    before_build(CompileShader)
    before_build(MoveResource)


target("RendererSample-HelloTriangle")
    CommonProjectSetting()
    add_files("src/Sample/1-HelloTriangle/HelloTriangle.cpp")
    before_build(CompileShader)
    before_build(MoveResource)

target("RendererSample-Texture")
    CommonProjectSetting()
    add_files("src/Sample/2-Texture/Texture.cpp")
    before_build(CompileShader)
    before_build(MoveResource)

target("RendererSample-CubeMap")
    CommonProjectSetting()
    add_files("src/Sample/3-CubeMap/CubeMap.cpp")
    before_build(CompileShader)
    before_build(MoveResource)