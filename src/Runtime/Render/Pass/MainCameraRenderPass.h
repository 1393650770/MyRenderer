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

namespace MXRender { class VK_DescriptorSetLayout; }


namespace MXRender
{

    class MainCamera_RenderPass:public VK_RenderPass
    {
    private:
    protected:
        void setup_renderpass();
        void setup_descriptorset_layout();
        void setup_pipelines();
        void setup_framebuffer(std::weak_ptr<VK_Viewport> viewport);
        std::vector<VkFramebuffer> swapchain_framebuffers;
        std::shared_ptr< VK_DescriptorSetLayout> layout;
    public:
        virtual void initialize(const PassInfo& init_info,std::shared_ptr<GraphicsContext> context);
        virtual void initialize(const PassInfo& init_info, std::shared_ptr<VK_GraphicsContext> context, std::weak_ptr<VK_Viewport> viewport);
        virtual void post_initialize();
        virtual void set_commonInfo(const PassInfo& init_info);
        virtual void prepare_pass_data(const GraphicsContext& context);
        virtual void initialize_ui_renderbackend();
        virtual void draw(GraphicsContext* context, uint32_t& image_index, VK_Viewport* viewport);
        std::weak_ptr<GraphicsContext> get_context();
        MainCamera_RenderPass();
        MainCamera_RenderPass(const PassInfo& init_info);
        virtual ~MainCamera_RenderPass();

        virtual VkRenderPass& get_render_pass() ;
        std::vector<VkFramebuffer>& get_swapchain_framebuffers();
    };



}
#endif
