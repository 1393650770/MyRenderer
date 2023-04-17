#include "Window.h"
#include "../RHI/Vulkan/VK_Utils.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../RHI/Vulkan/VK_Device.h"
#include<iostream>
#include"../Utils/Singleton.h"
#include"DefaultSetting.h"

#include"MyRender.h"


#include <memory>
#include "Pass/MainCameraRenderPass.h"

#include "../UI/Editor_UI.h"
#include "../Logic/Input/InputSystem.h"
#include "../Logic/GameObjectManager.h"

static void on_window_size_callback(GLFWwindow* window, int width, int height)
{
	Singleton<MXRender::DefaultSetting>::get_instance().height = height;
	Singleton<MXRender::DefaultSetting>::get_instance().width = width;
};


MXRender::Window::Window()
{

	if (!glfwInit())
	{
		return;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(Singleton<DefaultSetting>::get_instance().width, Singleton<DefaultSetting>::get_instance().height, "MyRender", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return;
	}
	glfwSetWindowUserPointer(window, this);

	glfwSetWindowSizeCallback(window, on_window_size_callback);

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

	Singleton<DefaultSetting>::get_instance().context->init(this);

	EditorUI edit_ui;
	EditorUIInitInfo editui_info;
	
	editui_info.context= Singleton<DefaultSetting>::get_instance().context.get();
	edit_ui.initialize(&editui_info);
	render->init(Singleton<DefaultSetting>::get_instance().context,window,&edit_ui);

	Singleton<DefaultSetting>::get_instance().material_system->init(Singleton<DefaultSetting>::get_instance().context.get());
	Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.set_window(window);
	Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.set_height(Singleton<DefaultSetting>::get_instance().height);
	Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.set_width(Singleton<DefaultSetting>::get_instance().width);
	Singleton<DefaultSetting>::get_instance().gameobject_manager->start_load_prefabs((GraphicsContext*)(Singleton<DefaultSetting>::get_instance().context.get()));
	Singleton<DefaultSetting>::get_instance().gameobject_manager->set_overload_material(Singleton<DefaultSetting>::get_instance().context.get());

	while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		Singleton<DefaultSetting>::get_instance().input_system->process_input(window);

        glfwPollEvents();

        render->run(Singleton<DefaultSetting>::get_instance().context);

        //glfwSwapBuffers(window);
    }
	
	vkDeviceWaitIdle(Singleton<DefaultSetting>::get_instance().context->device->device);
}

GLFWwindow* MXRender::Window::GetWindow() const
{
    return window;
}

