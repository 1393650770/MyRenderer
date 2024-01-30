#include"VK_Buffer.h"
#include "VK_Define.h"
#include "VK_Utils.h"
#include "VK_CommandBuffer.h"

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

VK_Buffer::VK_Buffer(VK_Device* in_device, const BufferDesc& in_buffer_desc) :Buffer(in_buffer_desc),
device(in_device)
{
	VkBufferCreateInfo buffer_create_info = {};
	GenerateBufferCreateInfo(buffer_create_info, in_buffer_desc);
	vkCreateBuffer(device->GetDevice(), &buffer_create_info, VULKAN_CPU_ALLOCATOR, &buffer);
	memory_read_flags = TranslateBufferTypeToVulkanAllocationFlags(buffer_desc.type);
	device->GetMemoryManager()->AllocateBufferMemory(allocation, buffer, 
		TranslateBufferTypeToVulkanAllocationFlags(buffer_desc.type), 0);

}

void VK_Buffer::GenerateBufferCreateInfo(VkBufferCreateInfo& buffer_create_info, const BufferDesc& desc)
{
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = desc.size;
	buffer_create_info.usage =VK_Utils::Translate_Buffer_usage_type_To_VulkanUsageFlag(desc.type); //VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
}

void VK_Buffer::Destroy()
{

}

void* VK_Buffer::Map()
{
	if (lock_state == LockState::Locked)
	{
		CHECK_WITH_LOG(true, "RHI Error :  This buffer has been locked !")
		return nullptr;
	}
	lock_state = LockState::Locked;
	return allocation.GetMappedPointer(device);
}

void VK_Buffer::Unmap()
{
	lock_state = LockState::Unlocked;
}



ENUM_VulkanAllocationFlags VK_Buffer::TranslateBufferTypeToVulkanAllocationFlags(const ENUM_BUFFER_TYPE& buffer_usage)
{
	ENUM_VulkanAllocationFlags allocation_flags = ENUM_VulkanAllocationFlags::None;
	switch (buffer_usage)
	{
	case ENUM_BUFFER_TYPE::Index:
	{
		allocation_flags= allocation_flags| ENUM_VulkanAllocationFlags::AutoBind;
		break;
	}
	case ENUM_BUFFER_TYPE::Vertex:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind | ENUM_VulkanAllocationFlags::HostVisible;
		break;
	}
	case ENUM_BUFFER_TYPE::Uniform:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind ;
		break;
	}
	case ENUM_BUFFER_TYPE::Staging:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind | ENUM_VulkanAllocationFlags::HostVisible;
		break;
	}
	case ENUM_BUFFER_TYPE::None:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind;
		break;
	}
	default:
		CHECK_WITH_LOG(true, "RHI Error :  Not support this buffer usage type !")
		break;
	}
	return allocation_flags;
}

VK_StagingBufferManager::VK_StagingBufferManager(VK_Device* in_device) :device(in_device)
{

}

void VK_StagingBufferManager::ReleaseStagingBuffer(VK_Buffer*& buffer, VK_CommandBuffer* command_buffer)
{
	if (command_buffer)
	{

	}
	else
	{
		free_buffers.push_back(buffer);
	}
	buffer = nullptr;
}

VK_Buffer* VK_StagingBufferManager::GetStagingBuffer(UInt64 size)
{

	for (auto& stagingbuffer : free_buffers)
	{
		if (stagingbuffer->GetBufferDesc().size == size)
		{
			return stagingbuffer;
		}
	}
	
	BufferDesc staging_buffer_desc;

	staging_buffer_desc.size = size;
	staging_buffer_desc.stride = 4;
	staging_buffer_desc.type = ENUM_BUFFER_TYPE::Staging;
	VK_Buffer* staging_buffer = new VK_Buffer(device, staging_buffer_desc);
	using_buffers.push_back(staging_buffer);
	return staging_buffer;
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE