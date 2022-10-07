add_requires("vulkansdk","glad", "glfw", "glm","assimp")
add_rules("mode.debug", "mode.release")

add_rules("plugin.vsxmake.autoupdate")


target("Runtime")
    set_kind("static")
    set_languages("c++20")  
    add_files("src/Runtime/Mesh/*.cpp") 
    add_files("src/Runtime/Render/*.cpp")  
    add_files("src/Runtime/Render/Pass/*.cpp") 
    add_files("src/Runtime/RHI/OpenGL/*.cpp") 
    add_files("src/Runtime/RHI/Vulkan/*.cpp") 
    add_files("src/Runtime/RHI/*.cpp") 
    add_files("src/Runtime/Utils/*.cpp") 

    add_headerfiles("src/Runtime/Mesh/*.h") 
    add_headerfiles("src/Runtime/Render/*.h") 
    add_headerfiles("src/Runtime/Render/Pass/*.h") 
    add_headerfiles("src/Runtime/RHI/OpenGL/*.h") 
    add_headerfiles("src/Runtime/RHI/Vulkan/*.h") 
    add_headerfiles("src/Runtime/RHI/*.h") 
    add_headerfiles("src/Runtime/Utils/*.h") 

    add_headerfiles("src/ThirdParty/stb_image/*.h") 

    add_packages("vulkansdk","glad", "glfw", "glm","assimp")
    



target("Renderer")
    set_kind("binary")  
    set_languages("c++20")  
    add_deps("Runtime")
    add_files("src/*.cpp") 
    add_packages("vulkansdk","glad", "glfw", "glm","assimp")

    after_build(function (target)
        os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)")
    end)
    after_build(function (target)
        os.cp("$(projectdir)/src/Runtime/Render/Shader", "$(buildir)/windows/x64/debug")
    end)

    