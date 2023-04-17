set_arch("x64")

add_requires("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","rttr","lz4","nlohmann_json","gli")
add_requires("imgui v1.88-docking", {configs = {glfw_vulkan = true}})
add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")



target("Runtime")
    set_kind("static")
    set_languages("c++20")  
    
    --add_headerfiles("src/ThirdParty/spv_reflect/*.h") 
    --add_headerfiles("src/ThirdParty/spv_reflect/include/spirv/unified1/*.h") 
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_files("src/**.c")
    --[[
    add_files("src/Runtime/Mesh/*.cpp") 
    add_files("src/Runtime/RHI/OpenGL/*.cpp") 
    add_files("src/Runtime/RHI/Vulkan/*.cpp") 
    add_files("src/Runtime/RHI/*.cpp") 
    add_files("src/Runtime/Utils/*.cpp") 
    add_files("src/Runtime/Render/*.cpp") 
    add_files("src/Runtime/Render/Pass/*.cpp") 
    add_files("src/Runtime/Logic/*.cpp") 
    add_files("src/Runtime/Logic/Component/*.cpp") 
    add_files("src/Runtime/Rttr/*.cpp") 
    add_files("src/Runtime/UI/*.cpp") 
    add_files("src/Runtime/Logic/Camera/*.cpp") 
    add_files("src/Runtime/Logic/Input/*.cpp") 
    add_files("src/Runtime/AssetLoader/*.cpp") 
    add_headerfiles("src/Runtime/AssetLoader/*.h") 
    add_headerfiles("src/Runtime/Logic/Input/*.h") 
    add_headerfiles("src/Runtime/Logic/Camera/*.h") 
    add_headerfiles("src/Runtime/UI/*.h") 
    add_headerfiles("src/Runtime/Rttr/*.h") 
    add_headerfiles("src/Runtime/Logic/Component/*.h") 
    add_headerfiles("src/Runtime/Logic/*.h") 
    add_headerfiles("src/Runtime/Render/*.h") 
    add_headerfiles("src/Runtime/Render/Pass/*.h")  
    add_headerfiles("src/Runtime/Mesh/*.h") 
    add_headerfiles("src/Runtime/RHI/OpenGL/*.h") 
    add_headerfiles("src/Runtime/RHI/Vulkan/*.h") 
    add_headerfiles("src/Runtime/RHI/*.h") 
    add_headerfiles("src/Runtime/Utils/*.h") 
    add_files("src/ThirdParty/spv_reflect/*.c") 
    add_headerfiles("src/ThirdParty/stb_image/*.h") 
    add_headerfiles("src/ThirdParty/spv_reflect/*.h") 
    add_headerfiles("src/ThirdParty/spv_reflect/include/spirv/unified1/*.h") 
    add_headerfiles("src/ThirdParty/vma/*.h") 
    --add_headerfiles("src/ThirdParty/imgui/*.h") 
    --add_files("src/ThirdParty/imgui/*.cpp")
    --add_headerfiles("src/ThirdParty/imgui/backends/*.h")
    --add_files("src/ThirdParty/imgui/backends/*.cpp")
    --]]
    add_packages("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","imgui","rttr","lz4","nlohmann_json","gli")
    



target("Renderer")

    set_kind("binary")  
    set_languages("c++20")  
    add_deps("Runtime")

    add_files("src/*.cpp") 

    add_packages("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","imgui","rttr")

    before_build(function (target)
        os.run("$(projectdir)/src/Runtime/Render/Shader/compile - glslangValidator.bat")
    end)

    after_build(function (target)
        os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)")
        os.cp("$(projectdir)/src/Resource", "$(buildir)")
        os.cp("$(projectdir)/src/Setting", "$(buildir)")
    end)

    after_build(
        function (target)
            os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)/windows/x64/debug")
            os.cp("$(projectdir)/src/Resource", "$(buildir)/windows/x64/debug")
            os.cp("$(projectdir)/src/Setting", "$(buildir)/windows/x64/debug")
        end
    )




    