#include "PipelineShaderObject.h"
#include "../../RHI/Vulkan/VK_GraphicsContext.h"
#include "../../RHI/Vulkan/VK_Device.h"
#include "../../RHI/Vulkan/VK_Shader.h"
#include "../../RHI/Vulkan/VK_VertexArray.h"
#include "../../RHI/Vulkan/VK_DescriptorSets.h"
#include "../../RHI/Vulkan/VK_Viewport.h"
#include "../../RHI/Vulkan/VK_SwapChain.h"
#include "../DefaultSetting.h"
#include "../../Utils/Singleton.h"
#include "vulkan/vulkan_core.h"
#include "../../RHI/RenderEnum.h"
#include "../../RHI/UniformBuffer.h"
#include "../../RHI/Vulkan/VK_Utils.h"
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../RHI/Vulkan/VK_Texture.h"
#include "../../RHI/Vulkan/VK_Shader.h"
#include <array>
#include "../../RHI/Vulkan/VK_RenderPass.h"
namespace MXRender
{

	
	PipelineShaderObject::PipelineShaderObject()
	{

	}

	PipelineShaderObject::~PipelineShaderObject()
	{

	}

	void PipelineShaderObject::init(VkDevice device, VK_Shader* in_shader, PipelineBuilder* in_pipeline_builder, VkRenderPass in_renderpass )
	{
		shader=in_shader;
		pipeline_layout=in_shader->get_built_layout();
		in_pipeline_builder->set_shaders(shader);
		pipeline = in_pipeline_builder->build_pipeline(device, in_renderpass);
	}

}
