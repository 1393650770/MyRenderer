add_requires("glad", "glfw", "glm","assimp")
add_rules("plugin.vsxmake.autoupdate")

target("Utils")
    set_kind("static")
    add_files("src/Utils/*.cpp") 

target("ThirdParty")
    set_kind("static")
    add_files("src/ThirdParty/stb_image/*.cpp") 

target("RHI")
    set_kind("static")
    add_files("src/RHI/*.cpp") 
    add_packages("glad", "glfw", "glm","assimp")

target("Runtime")
    set_kind("static")
    add_files("src/Runtime/*.cpp") 
    add_packages("glad", "glfw", "glm","assimp")



target("Renderer")
    set_kind("binary")    
    add_deps("ThirdParty")
    add_deps("Utils")
    add_deps("RHI")
    add_deps("Runtime")
    add_files("src/Render/*.cpp") 
    add_files("src/*.cpp") 
    add_packages("glad", "glfw", "glm","assimp")