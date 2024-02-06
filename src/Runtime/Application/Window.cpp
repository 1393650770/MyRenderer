#include "Window.h"
#include<iostream>
#include <memory>
#include "RHI/RenderRHI.h"
#include "RHI/RenderViewport.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)
/*
static void on_window_size_callback(GLFWwindow* window, int width, int height)
{
	Singleton<MXRender::DefaultSetting>::get_instance().height = height;
	Singleton<MXRender::DefaultSetting>::get_instance().width = width;
};
*/


Window::Window()
{

	if (!glfwInit())
	{
		return;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width,height, "MyRender", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return;
	}
	glfwSetWindowUserPointer(window, this);

	//glfwSetWindowSizeCallback(window, on_window_size_callback);


 }

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}


	/*
	void swap_data(RenderScene* render_scene, GameObjectManager* gameobject_manager)
	{
		OPTICK_EVENT()
		for (auto& it: gameobject_manager->object_list)
		{
			render_scene->register_render_object(&it);
		}
		if (Singleton<DefaultSetting>::get_instance().is_enable_batch)
		{
			render_scene->merger_mesh();
			render_scene->merger_renderobj();
		}
	}

	void update_data(RenderScene* render_scene, GameObjectManager* gameobject_manager)
	{
		OPTICK_EVENT()
		for (auto& it : gameobject_manager->object_list)
		{
			render_scene->update_object_transform(&it);
		}

	}
	*/

void Window::Run(MyRender* render)
{
	
	//Singleton<DefaultSetting>::get_instance().context->init(this);
	//Singleton<DefaultSetting>::get_instance().material_system->init(Singleton<DefaultSetting>::get_instance().context.get());

	//RenderScene render_scene;
	//EditorUI edit_ui;
	//EditorUIInitInfo editui_info;
	//
	//editui_info.context= Singleton<DefaultSetting>::get_instance().context.get();
	//edit_ui.initialize(&editui_info);
	//render->init(Singleton<DefaultSetting>::get_instance().context,window,&edit_ui);


	//Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.set_window(window);
	//Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.set_height(Singleton<DefaultSetting>::get_instance().height);
	//Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.set_width(Singleton<DefaultSetting>::get_instance().width);
	//Singleton<DefaultSetting>::get_instance().gameobject_manager->start_load_asset((GraphicsContext*)(Singleton<DefaultSetting>::get_instance().context.get()));

	//TaskGraph& task_graph = Singleton<DefaultSetting>::get_instance().task_system->task_graph;

	//task_graph.add_task_node(2, "set_overload_material", task_graph.get_task(
	//&(GameObjectManager::set_overload_material),
	//(Singleton<DefaultSetting>::get_instance().gameobject_manager.get()),
	//(Singleton<DefaultSetting>::get_instance().context.get())), 
	//{1});

	//task_graph.add_task_node(3, "swap_data", task_graph.get_task(
	//	(swap_data),
	//	&(render_scene),
	//	(Singleton<DefaultSetting>::get_instance().gameobject_manager.get())),
	//	{ 2 });

	//Singleton<DefaultSetting>::get_instance().task_system->thread_pool.excute_task_graph(&task_graph).wait();


	while (!glfwWindowShouldClose(window))
    {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		//OPTICK_PUSH("ProccessInput")
		//Singleton<DefaultSetting>::get_instance().input_system->process_input(window);
		//OPTICK_POP()
        glfwPollEvents();

		//task_graph.add_task_node(4, "update_object_to_render_scene", task_graph.get_task(
		//	&(update_data),
		//	&(render_scene),
		//	(Singleton<DefaultSetting>::get_instance().gameobject_manager.get())),
		//	{});
		
		//task_graph.add_task_node(5, "render", task_graph.get_task(
		//	&(MyRender::run),
		//	render.get(),
		//	Singleton<DefaultSetting>::get_instance().context,
		//	&render_scene),
		//	{});
		//OPTICK_PUSH("PreRender")
		//auto wait_handle= Singleton<DefaultSetting>::get_instance().task_system->thread_pool.excute_task_graph(&task_graph);
		//OPTICK_POP()


        //render->run(Singleton<DefaultSetting>::get_instance().context,&render_scene);


		//wait_handle.wait();

        glfwSwapBuffers(window);
    }

	//vkDeviceWaitIdle(Singleton<DefaultSetting>::get_instance().context->device->device);
	//render_scene.destroy();
	//Singleton<DefaultSetting>::get_instance().context->cleanup();
	//Singleton<DefaultSetting>::get_instance().destroy();
}

GLFWwindow* Window::GetWindow() CONST
{
    return window;
}

void Window::InitWindow()
{

	RHIInit();
	viewport_rhi = RHICreateViewport((void*)window,width,height, is_full_screen);

}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
