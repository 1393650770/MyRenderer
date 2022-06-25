add_requires("glad", "glfw", "glm","assimp")
add_rules("plugin.vsxmake.autoupdate")


target("Runtime")
    set_kind("static")
    add_files("src/Runtime/Mesh/*.cpp") 
    add_files("src/Runtime/Render/*.cpp") 
    add_files("src/Runtime/RHI/OpenGL/*.cpp") 
    add_files("src/Runtime/RHI/*.cpp") 
    add_files("src/Runtime/Utils/*.cpp") 

    add_headerfiles("src/Runtime/Mesh/*.h") 
    add_headerfiles("src/Runtime/Render/*.h") 
    add_headerfiles("src/Runtime/RHI/OpenGL/*.h") 
    add_headerfiles("src/Runtime/RHI/*.h") 
    add_headerfiles("src/Runtime/Utils/*.h") 

    add_headerfiles("src/ThirdParty/stb_image/*.h") 

    add_packages("glad", "glfw", "glm","assimp")
    



target("Renderer")
    set_kind("binary")  
    set_languages("c++20")  
    add_deps("Runtime")
    add_files("src/*.cpp") 
    add_packages("glad", "glfw", "glm","assimp")