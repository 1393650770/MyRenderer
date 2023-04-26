#pragma once
#ifndef _COPY_RENDERPASS_
#define _COPY_RENDERPASS_

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


#include "../../RHI/Vulkan/VK_RenderPass.h"

namespace MXRender { class PipelineShaderObject; }


namespace MXRender { class WindowUI; }

namespace MXRender { class VK_Texture; }

namespace MXRender { class VK_DescriptorSetLayout; }


namespace MXRender
{

    class Copy_RenderPass :public VK_RenderPass
    {
    private:
        PipelineBuilder copy_pipeline_builder;
        PipelineShaderObject* pso;

		VkDescriptorSet blitSet;
    protected:
        std::vector<VkFramebuffer> swapchain_framebuffers;
		void setup_renderpass();
        void setup_pipeline();
		void setup_framebuffer();

        void destroy_framebuffer();

    public:
        virtual ~Copy_RenderPass();
        virtual void initialize(const PassInfo& init_info, PassOtherInfo* other_info) override;


        void begin_pass(GraphicsContext* context);
        void end_pass(GraphicsContext* context);
        virtual void draw(GraphicsContext* context, RenderScene* render_scene=nullptr) override;
        
		void build_input_set(VkDescriptorImageInfo image_info);
    };



}
#endif
