#pragma once
#ifndef _UI_RENDERPASS_
#define _UI_RENDERPASS_

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

namespace MXRender { class VK_Texture; }

namespace MXRender { class VK_DescriptorSetLayout; }


namespace MXRender
{

    class UI_RenderPass :public VK_RenderPass
    {
    private:
    protected:
        void setup_renderpass();
        void setup_descriptorset_layout();
        void setup_pipelines();
        void setup_framebuffer();
        void setup_uniformbuffer();
        void setup_descriptorpool();
        void setup_descriptorsets();

        void update_uniformbuffer();

        void destroy_framebuffer();
        std::vector<VkFramebuffer> swapchain_framebuffers;

		std::vector<VkBuffer> uniform_buffers;
		std::vector<VkDeviceMemory> uniform_buffers_memory;

        VkDescriptorPool descriptor_pool;
        std::vector<VkDescriptorSet> descriptor_sets;

        std::shared_ptr< VK_DescriptorSetLayout> descriptorset_layout;
        std::shared_ptr< VK_Texture> cubemap_texture;
    public:
        virtual void initialize(const PassInfo& init_info,std::shared_ptr<GraphicsContext> context);
        virtual void initialize(const PassInfo& init_info, std::shared_ptr<VK_GraphicsContext> context);
        virtual void initialize(const PassInfo& init_info, std::shared_ptr<VK_GraphicsContext> context, std::weak_ptr<VK_Viewport> viewport);
        virtual void post_initialize();
        virtual void set_commonInfo(const PassInfo& init_info);
        virtual void prepare_pass_data(const GraphicsContext& context);
        virtual void initialize_ui_renderbackend();
        virtual void draw(GraphicsContext* context);

        UI_RenderPass();
        UI_RenderPass(const PassInfo& init_info);
        virtual ~UI_RenderPass();

        virtual VkRenderPass& get_render_pass() ;
        std::vector<VkFramebuffer>& get_swapchain_framebuffers();
    };



}
#endif
