#pragma once
#ifndef _GPUDRIVEN_
#define _GPUDRIVEN_
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "../RHI/Vulkan/VK_Utils.h"

namespace MXRender { struct RenderObject; }

namespace MXRender { class RenderScene; }
namespace MXRender
{
	struct GPUObjectData 
	{
		glm::mat4 modelMatrix;
		glm::vec4 origin_rad; // bounds
		glm::vec4 extents;  // bounds
	};
	struct GPUInstance 
	{
		uint32_t objectID;
		uint32_t batchID;
	};
	struct GPUIndirectObject 
	{
		VkDrawIndexedIndirectCommand command;
		uint32_t objectID;
		uint32_t batchID;
	};

	class GPUDrivenSystem
	{
	public:
		AllocatedBuffer<GPUObjectData> objectDataBuffer;
		AllocatedBuffer<GPUIndirectObject> drawIndirectBuffer;
		AllocatedBuffer<uint32_t> instanceIdMapBuffer;
		AllocatedBuffer<GPUInstance> instanceBuffer;
		VkDescriptorSet object_data_set=VK_NULL_HANDLE;
		GPUDrivenSystem();
		virtual ~GPUDrivenSystem();
		void destroy();
		void excute_upload_computepass(RenderScene* render_scene);
		void execute_gpu_culling_computepass(RenderScene* render_scene);
		void update_descriptorset();
	protected:
	private:
		bool is_fill_indirectcommand=false;
	};
}
#endif //_GPUDRIVEN_