#include "Window.h"
#include<iostream>
#include"../Utils/Singleton.h"
#include"DefaultSetting.h"
#include <glad/glad.h>
#include"MyRender.h"

MXRender::Window::Window()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(Singleton<DefaultSetting>::get_instance().width, Singleton<DefaultSetting>::get_instance().height, "MyRender", NULL, NULL);
    if (window == NULL)
    {
        std::cout << " Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    else
    {
        glfwMakeContextCurrent(window);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;

        }
        else
        {
            glViewport(0, 0, Singleton<DefaultSetting>::get_instance().width, Singleton<DefaultSetting>::get_instance().height);
        }
    }
 }

MXRender::Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void MXRender::Window::run(std::shared_ptr<MyRender> render)
{
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        render->run();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
