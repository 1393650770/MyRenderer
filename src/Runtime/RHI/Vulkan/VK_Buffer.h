#pragma once
#ifndef _VK_BUFFER_
#define _VK_BUFFER_
#include "RHI/RenderBuffer.h"
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
	VIRTUAL ~VK_Buffer();

	VIRTUAL void* METHOD(Map)() OVERRIDE;
	VIRTUAL void METHOD(Unmap)() OVERRIDE;
	VkBuffer METHOD(GetBuffer)() CONST;
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
	VIRTUAL ~VK_StagingBufferManager() ;

	VK_Buffer* METHOD(GetStagingBuffer)(UInt64 size);
	void METHOD(ReleaseStagingBuffer)(VK_Buffer*& buffer, VK_CommandBuffer* command_buffer);

	void METHOD(ProcessPendingFree)(Bool is_immediate, Bool is_free_to_os);
protected:

	MYRENDERER_BEGIN_STRUCT(PendingItemPerCmdBuffer)
		VK_CommandBuffer* command_buffer = nullptr;
		MYRENDERER_BEGIN_STRUCT(PendingItems)
			Vector<VK_Buffer*> buffer;
			UInt64 fence = 0;
		MYRENDERER_END_STRUCT
		Vector<PendingItems> pending_items;
	inline PendingItems* METHOD(FindOrAddItemsForFence)(UInt64 fence);
	MYRENDERER_END_STRUCT

	PendingItemPerCmdBuffer* METHOD(FindOrAddPendingItemPerCmdBuffer)(VK_CommandBuffer* command_buffer);
private:

#pragma endregion


#pragma region MEMBER
public:

protected:
	VK_Device* device;
	UInt64 max_buffer_size = 0;
	UInt64 current_buffer_size = 0;

	Vector<VK_Buffer*> using_buffers;



		Vector<PendingItemPerCmdBuffer> pending_items_per_cmd_buffer;
	MYRENDERER_BEGIN_STRUCT(FreeEntry)
		VK_Buffer* buffer = nullptr;
		UInt64 frame_num = 0;

		Bool operator==(const FreeEntry& rhs) const
		{
			return buffer == rhs.buffer ;
		}
	MYRENDERER_END_STRUCT
		Vector<FreeEntry> free_buffers;


private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
