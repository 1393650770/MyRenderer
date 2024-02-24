set_arch("x64")

add_requires("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","rttr","lz4","nlohmann_json","gli","optick","boost","flatbuffers")
add_requires("imgui v1.88-docking", {configs = {glfw_vulkan = true}})
add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")


    

target("Runtime_static")
    set_kind("static")
    set_languages("c++20")  
    
    add_headerfiles("src/Runtime/**.h")
    add_files("src/Runtime/**.cpp")
    add_headerfiles("src/ThirdParty/**.h")
    add_files("src/ThirdParty/**.cpp")
    add_files("src/ThirdParty/**.c")
    add_includedirs("src/Runtime")
    add_includedirs("src/ThirdParty")
    
    add_packages("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","imgui","rttr","lz4","nlohmann_json","gli","optick","boost","flatbuffers")
    



target("Renderer")

    set_kind("binary")  
    set_languages("c++20")  
    add_deps("Runtime_static")

    add_files("src/RendererApp.cpp") 
    add_includedirs("src/Runtime")
    add_includedirs("src/ThirdParty")
    
    add_packages("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","imgui","boost","rttr")

    before_build(function (target)
        --os.run("$(projectdir)/src/Runtime/Render/Shader/compile-glslangValidator.bat")
    end)

    after_build(function (target)
        --os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)")
        --os.cp("$(projectdir)/src/Resource", "$(buildir)")
        --os.cp("$(projectdir)/src/Setting", "$(buildir)")
    end)

    after_build(
        function (target)
            --os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)/windows/x64/debug")
            --os.cp("$(projectdir)/src/Resource", "$(buildir)/windows/x64/debug")
            --os.cp("$(projectdir)/src/Setting", "$(buildir)/windows/x64/debug")
        end
    )


target("RendererSample-HelloTriangle")
    set_kind("binary")  
    set_languages("c++20")  
    add_deps("Runtime_static")

    add_files("src/Sample/1-HelloTriangle/HelloTriangle.cpp") 
    add_includedirs("src/Runtime")
    add_includedirs("src/ThirdParty")
    
    add_packages("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","imgui","boost","rttr")

    before_build(function (target)
        --os.run("$(projectdir)/src/Runtime/Render/Shader/compile-glslangValidator.bat")
    end)

    after_build(function (target)
        --os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)")
        --os.cp("$(projectdir)/src/Resource", "$(buildir)")
        --os.cp("$(projectdir)/src/Setting", "$(buildir)")
    end)

    after_build(
        function (target)
            --os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)/windows/x64/debug")
            --os.cp("$(projectdir)/src/Resource", "$(buildir)/windows/x64/debug")
            --os.cp("$(projectdir)/src/Setting", "$(buildir)/windows/x64/debug")
        end
    )

