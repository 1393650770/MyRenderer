#pragma once
#ifndef _VK_QUEUE_
#define _VK_QUEUE_
#include "../../Core/ConstDefine.h"
#include "optick.h"
#include "../RenderRource.h"



MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Queue,public RenderResource)
#pragma region METHOD
public:
	VK_Queue(VK_Device* in_device,UInt32 in_family_index);
	VIRTUAL ~VK_Queue();

	void METHOD(Submit)(VK_CommandBuffer* command_list,  UInt32 NumSignalSemaphores = 0, VkSemaphore* SignalSemaphores = nullptr);
	UInt32 METHOD(GetFamily)() CONST;
	UInt32 METHOD(GetQueueIndex)() CONST;
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:

private:
	VkQueue queue = VK_NULL_HANDLE;
	UInt32 family_index = 0;
	UInt32 queue_index = 0;
	VK_Device* device = nullptr;
#pragma endregion


MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif //_VK_QUEUE_
