add_requires("glad", "glfw")

target("Renderer")
    set_kind("binary")
    add_files("src/*.cpp") 
    add_packages("glad", "glfw")