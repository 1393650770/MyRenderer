set_arch("x64")

add_requires("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","rttr")
add_requires("imgui v1.88-docking", {configs = {glfw_vulkan = true}})
add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")


target("Runtime")
    set_kind("static")
    set_languages("c++20")  

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

    add_headerfiles("src/ThirdParty/stb_image/*.h") 

    --add_headerfiles("src/ThirdParty/imgui/*.h") 
    --add_files("src/ThirdParty/imgui/*.cpp")
    --add_headerfiles("src/ThirdParty/imgui/backends/*.h")
    --add_files("src/ThirdParty/imgui/backends/*.cpp")


    
    add_packages("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","imgui","rttr")
    



target("Renderer")

    set_kind("binary")  
    set_languages("c++20")  
    add_deps("Runtime")

    add_files("src/*.cpp") 

    add_packages("vulkansdk","glad", "glfw", "glm","assimp","tinyobjloader","imgui","rttr")

    before_build(function (target)
        os.run("$(projectdir)/src/Runtime/Render/Shader/compile.bat")
    end)

    after_build(function (target)
        os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)")
        os.cp("$(projectdir)/src/Resource", "$(buildir)")
    end)

    after_build(
        function (target)
            os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)/windows/x64/debug")
            os.cp("$(projectdir)/src/Resource", "$(buildir)/windows/x64/debug")
        end
    )




    