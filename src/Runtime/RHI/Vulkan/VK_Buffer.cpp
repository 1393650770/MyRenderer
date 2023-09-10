#include"VK_Buffer.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)
void* VK_Buffer::Map()
{
	return allocation.GetMappedPointer(device);
}

void VK_Buffer::Unmap()
{
}

VK_Buffer::VK_Buffer(VK_Device& in_device, const BufferDesc& in_buffer_desc) :Buffer(in_buffer_desc),
device(&in_device)
{
	VkBufferCreateInfo buffer_create_info = {};
	GenerateBufferCreateInfo(buffer_create_info, in_buffer_desc);
	vkCreateBuffer(device->GetDevice(), &buffer_create_info, nullptr, &buffer);
}

void VK_Buffer::GenerateBufferCreateInfo(VkBufferCreateInfo& buffer_create_info, const BufferDesc& desc)
{
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = desc.size;
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
}

VK_BufferManager::VK_BufferManager(VK_Device* in_device) :device(&in_device)
{

}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE