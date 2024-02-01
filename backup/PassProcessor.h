#pragma once
#ifndef _MAINCAMERA_RENDERPASS_
#define _MAINCAMERA_RENDERPASS_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../../RHI/RenderEnum.h"
#include<memory>
#include<string>
#include "vulkan/vulkan_core.h"

#include "../../RHI/Vulkan/VK_RenderPass.h"
#include <unordered_map>

namespace MXRender { class PipelineBuilder; }


namespace MXRender { class VK_Shader; }

namespace MXRender { class VK_Texture; }

namespace MXRender { class VK_DescriptorSetLayout; }


namespace MXRender
{

	class PassProcessor
	{
	private:
	protected:

	public:
		PipelineBuilder builder;

		PassProcessor();
		virtual ~PassProcessor();

		virtual void build_resource(){};
	};


}
#endif
