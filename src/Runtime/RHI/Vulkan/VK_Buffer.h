#pragma once
#ifndef _VK_BUFFER_
#define _VK_BUFFER_
#include "../Buffer.h"
#include "VK_Device.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Buffer,  public Buffer)

friend class VK_BufferManager;

#pragma region METHOD
public:
	VK_Buffer(VK_Device& in_device, const BufferDesc& in_buffer_desc);
	VIRTUAL ~VK_Buffer() DEFAULT;

	VIRTUAL void* METHOD(Map)() OVERRIDE;
	VIRTUAL void METHOD(Unmap)() OVERRIDE;

	static void METHOD(GenerateBufferCreateInfo)(VkBufferCreateInfo& buffer_create_info, const BufferDesc& desc);
protected:

private:

#pragma endregion


#pragma region MEMBER
public:

protected:
	VK_Device* device = nullptr;
	VkBuffer buffer = VK_NULL_HANDLE;
	VK_Allocation allocation;
	UInt32 size = 0;

private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(VK_BufferManager)

#pragma region METHOD
public:
	VK_BufferManager(VK_Device* in_device);
	VIRTUAL ~VK_BufferManager() DEFAULT;


protected:
	VK_Device* device;
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
