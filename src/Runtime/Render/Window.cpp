#include "Window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
#include "../RHI/Vulkan/VK_Device.h"
#include <memory>
#include "Pass/MainCameraRenderPass.h"



MXRender::Window::Window()
{


	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//if (window == NULL)
	//{
	//	std::cout << " Failed to create GLFW window" << std::endl;
	//	glfwTerminate();
	//}
	//else
	//{
	//	glfwMakeContextCurrent(window);
	//	

	//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	//	{
	//		std::cout << "Failed to initialize GLAD" << std::endl;

	//	}
	//	else
	//	{
	//		glViewport(0, 0, Singleton<DefaultSetting>::get_instance().width, Singleton<DefaultSetting>::get_instance().height);
	//	}
	//}

 }

MXRender::Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void MXRender::Window::run(std::shared_ptr<MyRender> render)
{

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(Singleton<DefaultSetting>::get_instance().width, Singleton<DefaultSetting>::get_instance().height, "MyRender", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return;
	}

	glfwSetWindowUserPointer(window, this);

	Singleton<DefaultSetting>::get_instance().context->init(window); 
	
	render->init(Singleton<DefaultSetting>::get_instance().context,window);

	while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        render->run(Singleton<DefaultSetting>::get_instance().context);

        //glfwSwapBuffers(window);
    }
	
	vkDeviceWaitIdle(Singleton<DefaultSetting>::get_instance().context->get_device()->device);
}

GLFWwindow* MXRender::Window::GetWindow() const
{
    return window;
}
