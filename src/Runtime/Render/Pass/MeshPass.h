#pragma once
#ifndef _MESH_RENDERPASS_
#define _MESH_RENDERPASS_

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
#include "../../Mesh/VK_Mesh.h"

namespace MXRender { class VK_Texture; }
namespace MXRender { class VK_Mesh; }
namespace MXRender { class VK_DescriptorSetLayout; }


namespace MXRender
{
    struct Mesh_PassInfo:PassInfo
    {
    public:
        VkRenderPass renderpass;
    };

    class Mesh_RenderPass :public VK_RenderPass
    {
    private:
    protected:

        void setup_descriptorset_layout();
        void setup_pipelines();
        void setup_uniformbuffer();
        void setup_descriptorpool();
        void setup_descriptorsets();
        void setup_vertexbuffer();
        void setup_indexbuffer();
        void setup_mesh_data();
        void update_uniformbuffer();



		std::vector<VkBuffer> uniform_buffers;
		std::vector<VkDeviceMemory> uniform_buffers_memory;

        VkDescriptorPool descriptor_pool;
        std::vector<VkDescriptorSet> descriptor_sets;

        std::shared_ptr< VK_DescriptorSetLayout> descriptorset_layout;
        std::shared_ptr< VK_Texture> cubemap_texture;

        std::string mesh_file_path="Resource/Mesh/viking_room.obj";

        std::shared_ptr<VK_Mesh> mesh_data;

        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
    public:
        virtual void post_initialize();
        virtual void set_commonInfo(const PassInfo& init_info);
        virtual void prepare_pass_data(const GraphicsContext& context);
        virtual void initialize_ui_renderbackend();
        virtual void draw(GraphicsContext* context);

        Mesh_RenderPass();
        Mesh_RenderPass(const PassInfo& init_info);
        virtual ~Mesh_RenderPass();

        virtual VkRenderPass& get_render_pass() ;
        std::vector<VkFramebuffer>& get_swapchain_framebuffers();

        virtual void initialize(const PassInfo& init_info, PassOtherInfo* other_info) override;

    };



}
#endif
