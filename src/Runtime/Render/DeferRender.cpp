#include "DeferRender.h"
#include<Windows.h>
#include <iostream>
#include <filesystem>
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../RHI/Vulkan/VK_Viewport.h"

MXRender::DeferRender::DeferRender()
{

}

MXRender::DeferRender::~DeferRender()
{
}

void MXRender::DeferRender::run(std::weak_ptr <VK_GraphicsContext> context, VkSwapchainKHR& swapchain)
{
	if(context.expired()) return ;
	context.lock()->wait_for_fences();
	context.lock()->reset_commandbuffer();
	uint32_t image_index;
	context.lock()->pre_pass(swapchain, image_index);

	std::vector<RenderPass*> render_pass_vector;
	context.lock()->draw(render_pass_vector,nullptr);

	context.lock()->submit(&swapchain,1, image_index);
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);

	//shader->bind();

	//vertex_array->bind();
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	////glDrawArrays(GL_TRIANGLES, 0, 3);
	//vertex_array->unbind();

	//shader->unbind();
	

}

void MXRender::DeferRender::init()
{ 
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
