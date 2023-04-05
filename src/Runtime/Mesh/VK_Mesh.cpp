#include "VK_Mesh.h"
#include "../RHI/Vulkan/VK_VertexArray.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../RHI/Vulkan/VK_Device.h"
#include "../RHI/Vulkan/VK_Utils.h"


MXRender::VK_Mesh::VK_Mesh()
{

}

MXRender::VK_Mesh::~VK_Mesh()
{

}

void MXRender::VK_Mesh::destroy_mesh_info(GraphicsContext* context)
{
	VK_GraphicsContext* vk_context=dynamic_cast<VK_GraphicsContext*>(context);
	if(!vk_context) return ; 
	vkDestroyBuffer(vk_context->device->device, vk_meshinfo.index_buffer, nullptr);
	vkFreeMemory(vk_context->device->device, vk_meshinfo.index_buffer_memory, nullptr);

	vkDestroyBuffer(vk_context->device->device, vk_meshinfo.vertex_buffer, nullptr);
	vkFreeMemory(vk_context->device->device, vk_meshinfo.vertex_buffer_memory, nullptr);
}

void MXRender::VK_Mesh::init_mesh_info(GraphicsContext* context)
{
	if (is_already_init)
	{
		return ;
	}
	VK_GraphicsContext* vk_context = dynamic_cast<VK_GraphicsContext*>(context);
	if (!vk_context) return;
	setup_vk_vertexbuffer(vk_context, vk_meshinfo.vertex_buffer, vk_meshinfo.vertex_buffer_memory);
	setup_vk_indexbuffer(vk_context,vk_meshinfo.index_buffer,vk_meshinfo.index_buffer_memory);

	is_already_init=true;
}

MXRender::VK_MeshInfo MXRender::VK_Mesh::get_mesh_info() const
{
	return vk_meshinfo;
}

void MXRender::VK_Mesh::setup_vk_vertexbuffer(VK_GraphicsContext* cur_context, VkBuffer& vertex_buffer, VkDeviceMemory& vertex_buffer_memory)
{

	VkDeviceSize bufferSize = is_prefabs?sizeof(assetvertex_vertices[0])*assetvertex_vertices.size() : sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(cur_context->device->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	if (is_prefabs)
	{
		memcpy(data, assetvertex_vertices.data(), (size_t)bufferSize);
	}
	else
	{ 
		memcpy(data, vertices.data(), (size_t)bufferSize);
	}
	vkUnmapMemory(cur_context->device->device, stagingBufferMemory);


	VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);
	
	VK_Utils::Copy_VKBuffer(cur_context, stagingBuffer, vertex_buffer, bufferSize);

	vkDestroyBuffer(cur_context->device->device, stagingBuffer, nullptr);
	vkFreeMemory(cur_context->device->device, stagingBufferMemory, nullptr);
}

void MXRender::VK_Mesh::setup_vk_indexbuffer(VK_GraphicsContext* cur_context, VkBuffer& index_buffer, VkDeviceMemory& index_buffer_memory)
{

	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(cur_context->device->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(cur_context->device->device, stagingBufferMemory);


	VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);

	VK_Utils::Copy_VKBuffer(cur_context, stagingBuffer, index_buffer, bufferSize);

	vkDestroyBuffer(cur_context->device->device, stagingBuffer, nullptr);
	vkFreeMemory(cur_context->device->device, stagingBufferMemory, nullptr);
}
