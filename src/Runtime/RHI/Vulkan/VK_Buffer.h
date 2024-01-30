#pragma once
#ifndef _VK_BUFFER_
#define _VK_BUFFER_
#include "../Buffer.h"
#include "VK_Device.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_CommandBuffer;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Buffer,  public Buffer)

friend class VK_BufferManager;

#pragma region METHOD
public:
	VK_Buffer(VK_Device* in_device, CONST BufferDesc& in_buffer_desc);
	VIRTUAL ~VK_Buffer() DEFAULT;

	VIRTUAL void* METHOD(Map)() OVERRIDE;
	VIRTUAL void METHOD(Unmap)() OVERRIDE;

	void Destroy();

	static void METHOD(GenerateBufferCreateInfo)(VkBufferCreateInfo& buffer_create_info, CONST BufferDesc& desc);
	static ENUM_VulkanAllocationFlags METHOD(TranslateBufferTypeToVulkanAllocationFlags)(CONST ENUM_BUFFER_TYPE& buffer_usage);
protected:
	void AllocateMemory();


private:

#pragma endregion


#pragma region MEMBER
public:

protected:
	VK_Device* device = nullptr;
	VkBuffer buffer = VK_NULL_HANDLE;
	VK_Allocation allocation;
	UInt32 size = 0;

	enum class LockState : UInt8
	{
		Default,
		Locked,
		Unlocked,
		PersistentMapping
	};

	LockState lock_state = LockState::Default;
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(VK_StagingBufferManager)

#pragma region METHOD
public:
	VK_StagingBufferManager(VK_Device* in_device);
	VIRTUAL ~VK_StagingBufferManager() DEFAULT;

	VK_Buffer* METHOD(GetStagingBuffer)(UInt64 size);
	void METHOD(ReleaseStagingBuffer)(VK_Buffer*& buffer, VK_CommandBuffer* command_buffer);
protected:
	VK_Device* device;
	UInt64 max_buffer_size = 0;
	UInt64 current_buffer_size = 0;

	Vector<VK_Buffer*> free_buffers;
	Vector<VK_Buffer*> using_buffers;

private:

#pragma endregion


#pragma region MEMBER
public:

protected:
	VK_Device* device = nullptr;


private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
