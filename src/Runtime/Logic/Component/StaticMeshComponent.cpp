#include "StaticMeshComponent.h"
#include <stdexcept>
#include "glm/ext/matrix_transform.hpp"
#include "../../RHI/RenderState.h"
#include "../../RHI/Vulkan/VK_GraphicsContext.h"
#include "../../Mesh/MeshBase.h"
#include "../../RHI/Vulkan/VK_Utils.h"
#include "../../RHI/Vulkan/VK_VertexArray.h"




void MXRender::StaticMeshComponent::setup_vk_vertexbuffer(VK_GraphicsContext* cur_context, VkBuffer vertex_buffer, VkDeviceMemory vertex_buffer_memory)
{

	VkDeviceSize bufferSize = sizeof(mesh_data->vertices[0]) * mesh_data->vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(cur_context->device->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mesh_data->vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(cur_context->device->device, stagingBufferMemory);

	VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);

	VK_Utils::Copy_VKBuffer(cur_context, stagingBuffer, vertex_buffer, bufferSize);

	vkDestroyBuffer(cur_context->device->device, stagingBuffer, nullptr);
	vkFreeMemory(cur_context->device->device, stagingBufferMemory, nullptr);
}

void MXRender::StaticMeshComponent::setup_vk_indexbuffer(VK_GraphicsContext* cur_context, VkBuffer index_buffer, VkDeviceMemory index_buffer_memory)
{
	VkDeviceSize bufferSize = sizeof(mesh_data->indices[0]) * mesh_data->indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(cur_context->device->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mesh_data->indices.data(), (size_t)bufferSize);
	vkUnmapMemory(cur_context->device->device, stagingBufferMemory);

	VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);

	VK_Utils::Copy_VKBuffer(cur_context, stagingBuffer, index_buffer, bufferSize);

	vkDestroyBuffer(cur_context->device->device, stagingBuffer, nullptr);
	vkFreeMemory(cur_context->device->device, stagingBufferMemory, nullptr);
}



MXRender::StaticMeshComponent::StaticMeshComponent(const std::string& mesh_path)
{
	component_type=ComponentType::STATICMESH;
	mesh_data=std::make_shared<MeshBase>();
	mesh_data->load_model(mesh_path);
}



MXRender::StaticMeshComponent::~StaticMeshComponent()
{

}

std::weak_ptr<MXRender::MeshBase> MXRender::StaticMeshComponent::get_mesh_data()
{
	return mesh_data;
}

void MXRender::StaticMeshComponent::render_mesh(RenderMeshInfo* render_mesh_info)
{
	switch (RenderState::render_api_type)
	{
	case ENUM_RENDER_API_TYPE::Vulkan:
	{
		VK_GraphicsContext* vk_context=dynamic_cast<VK_GraphicsContext*>(render_mesh_info->context);
		VK_RenderMeshInfo* vk_render_mesh_info=static_cast<VK_RenderMeshInfo*>(render_mesh_info);
		if(!vk_context) return ;
		VkBuffer vertexBuffers[] = { vk_render_mesh_info->vertex_buffer};
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(vk_context->get_cur_command_buffer(), 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(vk_context->get_cur_command_buffer(), vk_render_mesh_info->index_buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(vk_context->get_cur_command_buffer(), static_cast<uint32_t>(mesh_data->indices.size()), 1, 0, 0, 0);

		break;
	}
	default:
		break;
	}
}

void MXRender::StaticMeshComponent::bind_mesh(BindMeshInfo* bind_mesh_info)
{
	switch (RenderState::render_api_type)
	{
	case ENUM_RENDER_API_TYPE::Vulkan:
	{
		VK_GraphicsContext* vk_context = dynamic_cast<VK_GraphicsContext*>(bind_mesh_info->context);
		VK_BindMeshInfo* vk_bind_mesh_info = static_cast<VK_BindMeshInfo*>(bind_mesh_info);
		if (!vk_context) return;
		setup_vk_indexbuffer(vk_context,vk_bind_mesh_info->index_buffer,vk_bind_mesh_info->index_buffer_memory);
		setup_vk_indexbuffer(vk_context, vk_bind_mesh_info->vertex_buffer, vk_bind_mesh_info->vertex_buffer_memory);
		break;
	}
	default:
		break;
	}
}

void MXRender::StaticMeshComponent::on_start()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::StaticMeshComponent::on_update()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::StaticMeshComponent::update(float delta_time)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::StaticMeshComponent::on_end()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::StaticMeshComponent::on_destroy()
{
	throw std::logic_error("The method or operation is not implemented.");
}
