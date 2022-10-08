#include "DeferRender.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include<Windows.h>
#include <iostream>
#include <filesystem>

#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../RHI/Vulkan/VK_Viewport.h"
#include "../RHI/Vulkan/VK_SwapChain.h"
#include "../Utils/Singleton.h"
#include "DefaultSetting.h"
#include "Pass/MainCameraRenderPass.h"


MXRender::DeferRender::DeferRender()
{

}

MXRender::DeferRender::~DeferRender()
{
}

void MXRender::DeferRender::run(std::weak_ptr <VK_GraphicsContext> context)
{
	if(context.expired()) return ;


	context.lock()->pre_pass();

	main_camera_pass->draw(context.lock().get());

	context.lock()->submit();

}

void MXRender::DeferRender::init(std::weak_ptr <VK_GraphicsContext> context,GLFWwindow* window)
{ 
	main_viewport = std::make_shared<VK_Viewport>(context.lock(), window, Singleton<DefaultSetting>::get_instance().width, Singleton<DefaultSetting>::get_instance().height, false);
	
	main_viewport->create_image_view_from_swapchain();

	PassInfo pass_info;
	main_camera_pass = std::make_shared<MainCamera_RenderPass>();

	main_camera_pass->initialize(pass_info, context.lock());

	//float ve[] = {
	//	// positions          // colors
	//	0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
	//		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
	//		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
	//		-0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f
	//};
	//std::vector<float> vertice({
	//	// positions          // colors
	//	 0.5f,  0.5f, 0.0f,  1.0f,0.0f,0.0f,
	//	 0.5f, -0.5f, 0.0f,  0.0f,1.0f,0.0f,
	//	-0.5f, -0.5f, 0.0f,  0.0f,0.0f,1.0f,
	//	-0.5f,  0.5f, 0.0f,  1.0f,0.0f,0.0f
	//	});

	//std::vector<unsigned int> index({
	//	0, 1, 3, // first triangle
	//	1, 2, 3  // second triangle
	//	});

	//std::shared_ptr<std::vector<float>> vertices_test = std::make_shared<std::vector<float>>(vertice);
	//std::shared_ptr<std::vector<unsigned int>> indices_test = std::make_shared<std::vector<unsigned int>>(index);

	//vertex_array = VertexArray::CreateVertexArray();
	//vertex_array->bind();

	//vertex_buffer = VertexBuffer::CreateVertexBuffer(ve, sizeof(ve),ENUM_RENDER_DATA_USAGE_TYPE::STATIC_DRAW);
	//index_buffer = IndexBuffer::CreateIndexBuffer(indices_test, sizeof(unsigned int)* indices_test.get()->size());
	//
	//vertex_array->set_indexbuffer(index_buffer);

	//vertex_buffer->set_layout(
	//	{ 
	//	{ENUM_RENDER_DATA_TYPE::Float,3,ENUM_RENDER_ATTRIBUTE_TYPE::Position},
	//	{ENUM_RENDER_DATA_TYPE::Float,3,ENUM_RENDER_ATTRIBUTE_TYPE::Color0} 
	//	}
	//);

	//vertex_array->set_vertexbuffer(vertex_buffer);
	//
	//shader = Shader::CreateShader("Shader/test.vs","Shader/test.fs");

}
