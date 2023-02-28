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

namespace MXRender { class PipelineBuilder; }


namespace MXRender { class VK_Shader; }

namespace MXRender { class VK_Texture; }

namespace MXRender { class VK_DescriptorSetLayout; }


namespace MXRender
{

    class PipelineShaderObject 
    {
    private:
    protected:
        VK_Shader * shader{nullptr};
        VkPipeline pipeline{VK_NULL_HANDLE};
        VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
    public:
       

        PipelineShaderObject();
        virtual ~PipelineShaderObject();

       void init(VkDevice device, VK_Shader* in_shader , PipelineBuilder* in_pipeline_builder, VkRenderPass renderPass );

    };



}
#endif
