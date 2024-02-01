#pragma once
#ifndef _VK_COMMANDBUFFER_
#define _VK_COMMANDBUFFER_
#include <vulkan/vulkan_core.h>

#include "../../Core/ConstDefine.h"
#include "../RenderRource.h"
#include "VK_Device.h"

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
	void METHOD(FreeUnusedCommandBuffer)(VK_Queue* queue);
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
friend VK_CommandBufferPool;
#pragma region METHOD
public:
	VK_CommandBuffer(VK_Device* in_device, VK_CommandBufferPool* in_command_buffer_pool, Bool in_is_upload_only);
	VIRTUAL ~VK_CommandBuffer();

	VkCommandBuffer METHOD(GetCommandBuffer)() CONST;
	VK_Fence* METHOD(GetFence)() CONST;
	Bool METHOD(WaitForFence)(float time_in_seconds_to_wait);

	UInt64 METHOD(GetFenceSignaledCounter)() CONST;

	void  METHOD(TrainsitionImageLayout)(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, CONST VkImageSubresourceRange& subresourceRange, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);
	void  METHOD(FlushBarriers)();
	void  METHOD(MemoryBarrier)(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
	void  METHOD(CopyBufferToImage)(VkBuffer buffer, VkImage image, VkImageLayout imageLayout, UInt32 region_count, CONST VkBufferImageCopy* region);

	__forceinline void METHOD(BeginRenderPass)(VkRenderPass RenderPass,
		VkFramebuffer       Framebuffer,
		uint32_t            FramebufferWidth,
		uint32_t            FramebufferHeight,
		uint32_t            ClearValueCount = 0,
		const VkClearValue* pClearValues = nullptr)
	{

	}

	__forceinline void METHOD(EndRenderPass)()
	{
		vkCmdEndRenderPass(command_buffer);
		state_cache.render_pass = VK_NULL_HANDLE;
		state_cache.framebuffer = VK_NULL_HANDLE;
		state_cache.framebuffer_height = 0;
		state_cache.framebuffer_width = 0;
	}


protected:
	void METHOD(Allocate)();
	void METHOD(Free)();


private:

#pragma endregion

#pragma region MEMBER
public:
	MYRENDERER_BEGIN_STRUCT( StateCache)
		VkRenderPass  render_pass = VK_NULL_HANDLE;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VkPipeline    graphics_pipeline = VK_NULL_HANDLE;
		VkPipeline    compute_pipeline = VK_NULL_HANDLE;
		VkPipeline    raytracing_pipeline = VK_NULL_HANDLE;
		VkBuffer      index_buffer = VK_NULL_HANDLE;
		VkDeviceSize  index_buffer_offset = 0;
		VkIndexType   index_type = VK_INDEX_TYPE_MAX_ENUM;
		UInt32      framebuffer_width = 0;
		UInt32      framebuffer_height = 0;
		UInt32      inside_pass_queries = 0;
		UInt32      outside_pass_queries = 0;
	MYRENDERER_END_STRUCT
protected:
	VK_Device* device;
	VkCommandBuffer command_buffer;
	VK_CommandBufferPool* owner_pool;
	Vector<VkSemaphore*> wait_semaphores;
	Vector<VkSemaphore*> submitted_wait_semaphores;
	VK_Fence* fence=nullptr;
	Bool is_upload_only=false;

	// Last value passed after the fence got signaled
	volatile UInt64 fence_signaled_counter;
	// Last value when we submitted the cmd buffer; useful to track down if something waiting for the fence has actually been submitted
	volatile UInt64 submitted_fence_counter;

	StateCache state_cache;

	MYRENDERER_BEGIN_STRUCT(PipelineBarrier)
	
		VkPipelineStageFlags memory_src_stages = 0;
		VkPipelineStageFlags memory_dst_stages = 0;
		VkAccessFlags        memory_src_access = 0;
		VkAccessFlags        memory_dst_access = 0;

		VkPipelineStageFlags image_src_stages = 0;
		VkPipelineStageFlags image_dst_stages = 0;

		VkPipelineStageFlags supported_stages_mask = ~0u;
		VkAccessFlags        supported_access_mask = ~0u;
	MYRENDERER_END_STRUCT

	PipelineBarrier pipeline_barrier;

	Vector<VkImageMemoryBarrier> image_barriers;
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_CommandBufferManager, public RenderResource)
#pragma region METHOD
public:

VK_CommandBufferManager(VK_Device* in_device );
VIRTUAL ~VK_CommandBufferManager();

VK_CommandBuffer* METHOD(GetOrCreateCommandBuffer)(ENUM_QUEUE_TYPE queue_type);
void METHOD(ReleaseCommandBuffer)(VK_CommandBuffer* cmd_buffer);
void METHOD(FreeUnusedCommandBuffer)(ENUM_QUEUE_TYPE queue_type);
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	VK_Device* device;
	Vector<VK_CommandBuffer*> graphic_cmd_buffers;
	Vector<VK_CommandBuffer*> compute_cmd_buffers;
	Vector<VK_CommandBuffer*> transfer_cmd_buffers;
	Vector<VK_CommandBuffer*> present_cmd_buffers;
	Vector<VK_CommandBufferPool> graphic_cmd_pools;
	Vector<VK_CommandBufferPool> compute_cmd_pools;
	Vector<VK_CommandBufferPool> transfer_cmd_pools;
	Vector<VK_CommandBufferPool> present_cmd_pools;

	Vector<VK_CommandBuffer*> using_graphic_cmd_buffers;
	Vector<VK_CommandBuffer*> using_compute_cmd_buffers;
	Vector<VK_CommandBuffer*> using_transfer_cmd_buffers;
	Vector<VK_CommandBuffer*> using_present_cmd_buffers;
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif 
