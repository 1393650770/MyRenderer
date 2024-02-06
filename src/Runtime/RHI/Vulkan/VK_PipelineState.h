#pragma once
#ifndef _VK_RENDERPIPELINESTATE_
#define _VK_RENDERPIPELINESTATE_
#include "RHI/RenderPipelineState.h"
#include "vulkan/vulkan_core.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)
class VK_Device;
class VK_RenderPass;
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_PipelineState, public RenderPipelineState)
#pragma region METHOD
public:
	VK_PipelineState() DEFAULT;
	VK_PipelineState(VK_Device* in_device, CONST RenderGraphiPipelineState& in_desc, VkPipelineCache pipeline_cache,CONST VK_RenderPass* render_pass);
	VIRTUAL ~VK_PipelineState();

protected:
	VkPipelineLayout METHOD(CreatePipelineLayout)(CONST RenderGraphiPipelineState& in_desc);
private:

#pragma endregion

#pragma region MEMBER
public:
protected:
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
	Array<VkDescriptorSetLayout, 4> descriptorset_layouts{ VK_NULL_HANDLE ,VK_NULL_HANDLE ,VK_NULL_HANDLE ,VK_NULL_HANDLE };
	VK_Device* device;
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(VK_PipelineStateManager)
#pragma region METHOD
public:
	VK_PipelineStateManager() DEFAULT;
	VK_PipelineStateManager(VK_Device* in_device);
	~VK_PipelineStateManager();

	VkPipelineCache METHOD(GetPipelineCache)() CONST;

	VK_PipelineState* METHOD(GetPipelineState)(CONST RenderGraphiPipelineState& in_desc, CONST VK_RenderPass* render_pass);
protected:

private:

#pragma endregion

#pragma region MEMBER
public:
protected:
	VK_Device* device;
	VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
	Map<UInt64,VK_PipelineState*> pipeline_states_map;
private:

#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif //_VK_QUEUE_
