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

namespace MXRender { class VK_Texture; }

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
    public:
        virtual void post_initialize();
        virtual void set_commonInfo(const PassInfo& init_info);
        virtual void prepare_pass_data(const GraphicsContext& context);

        virtual void draw(GraphicsContext* context, RenderScene* render_scene=nullptr);

        void begin_pass(GraphicsContext* context);
        void end_pass(GraphicsContext* context);

        MainCamera_RenderPass();
        MainCamera_RenderPass(const PassInfo& init_info);
        virtual ~MainCamera_RenderPass();

        virtual VkRenderPass& get_render_pass() ;
        std::vector<VkFramebuffer>& get_swapchain_framebuffers();

        virtual void initialize(const PassInfo& init_info, PassOtherInfo* other_info) override;

    };



}
#endif
