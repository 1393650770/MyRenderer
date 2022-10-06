#include "Window.h"
#include<iostream>
#include"../Utils/Singleton.h"
#include"DefaultSetting.h"
#include <glad/glad.h>
#include"MyRender.h"
#include"../RHI/VertexArray.h"
#include"../RHI/VertexBuffer.h"
#include"../RHI/IndexBuffer.h"
#include"../RHI/Shader.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../RHI/Vulkan/VK_Viewport.h"
#include "../RHI/Vulkan/VK_SwapChain.h"
#include <memory>
MXRender::Window::Window()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(Singleton<DefaultSetting>::get_instance().width, Singleton<DefaultSetting>::get_instance().height, "MyRender", NULL, NULL);
	glfwSetWindowUserPointer(window, this);
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
    render->init();
	std::shared_ptr <VK_GraphicsContext> context=std::make_shared<VK_GraphicsContext>() ;
	context->init(window);
	std::shared_ptr < VK_Viewport> viewport= std::make_shared<VK_Viewport>( context,window, Singleton<DefaultSetting>::get_instance().width, Singleton<DefaultSetting>::get_instance().height,false);
	viewport->create_image_view_from_swapchain();
	
	while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();

		VkSwapchainKHR swapchain= (viewport->get_swapchain()->get_swapchain());
        render->run(context, swapchain);

        glfwSwapBuffers(window);
    }
	

}

GLFWwindow* MXRender::Window::GetWindow() const
{
    return window;
}
