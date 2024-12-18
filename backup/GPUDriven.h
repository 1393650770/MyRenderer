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
	

	struct DrawCullData
	{
		glm::mat4 view;
		glm::mat4 proj;
		float P00, P11, znear, zfar; 
		float oct_frustum[4];
		float lodBase, lodStep;
		float pyramidWidth, pyramidHeight; 

		uint32_t drawCount;

		int cullingEnabled;
		int lodEnabled;
		int occlusionEnabled;
		int distCull;
		int isFirstFrame;
	};

	class GPUDrivenSystem
	{
	public:
		AllocatedBuffer<GPUObjectData> objectDataBuffer;
		AllocatedBuffer<GPUIndirectObject> drawIndirectBuffer;
		AllocatedBuffer<uint32_t> instanceIdMapBuffer;
		AllocatedBuffer<GPUInstance> instanceBuffer;
		VkImageView color_imageview;
		VkSampler color_image_sampler;
		VkImage color_image;
		VkDeviceMemory color_image_memory{ VK_NULL_HANDLE };
		VkFormat color_image_format = VK_FORMAT_R32G32B32A32_SFLOAT;

		GPUDrivenSystem();
		virtual ~GPUDrivenSystem();
		void destroy();
		void excute_upload_computepass(RenderScene* render_scene);
		void execute_gpu_culling_computepass(RenderScene* render_scene);
		void execute_reduce_depth_computepass(RenderScene* render_scene);
		void update_descriptorset();
	protected:
	private:
		bool is_fill_indirectcommand=false;
		bool is_first_frame=true;
	};
}
#endif //_GPUDRIVEN_