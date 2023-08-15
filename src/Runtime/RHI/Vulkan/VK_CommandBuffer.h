#pragma once
#ifndef _VK_COMMANDBUFFER_
#define _VK_COMMANDBUFFER_
#include <vulkan/vulkan_core.h>

#include "../../Core/ConstDefine.h"
#include "optick.h"
#include "../RenderRource.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;
class VK_CommandBuffer; 
class VK_CommandBufferPool;
class VK_CommandBufferManager;
class VK_Fence;
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_CommandBufferPool,public RenderResource)
#pragma region METHOD
public:
	VK_CommandBufferPool(VK_Device* in_device, VK_CommandBufferManager& in_cmd_buffer_manager);
	VIRTUAL ~VK_CommandBufferPool();

	VK_CommandBuffer* METHOD(GetOrCreateCommandBuffer)(Bool is_upload_only);
	void METHOD(Init)(UInt32 queue_family_index);
	VkCommandPool METHOD(GetPool)() CONST;
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	VkCommandPool command_pool = VK_NULL_HANDLE;

	Vector<VK_CommandBuffer*> cmd_buffers;
	Vector<VK_CommandBuffer*> free_cmd_buffers;

	VK_Device* device = nullptr;

	VK_CommandBufferManager& cmd_buffer_manager;
private:
	
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_CommandBuffer, public RenderResource)
#pragma region METHOD
public:
	VK_CommandBuffer(VK_Device* in_device, VK_CommandBufferPool* in_command_buffer_pool, Bool in_is_upload_only);
	VIRTUAL ~VK_CommandBuffer();

	VkCommandBuffer METHOD(GetCommandBuffer)() CONST;
	VK_Fence* METHOD(GetFence)() CONST;
protected:
	void METHOD(Allocate)();
	void METHOD(Free)();
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	VK_Device* device;
	VkCommandBuffer command_buffer;
	VK_CommandBufferPool* owner_pool;
	Vector<VkSemaphore*> wait_semaphores;
	Vector<VkSemaphore*> submitted_wait_semaphores;
	VK_Fence* fence=nullptr;
	Bool is_upload_only=false;
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_CommandBufferManager, public RenderResource)
#pragma region METHOD
public:
VK_CommandBufferManager();
VIRTUAL ~VK_CommandBufferManager();

protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:

private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif 
