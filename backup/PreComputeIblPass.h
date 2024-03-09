#pragma once
#ifndef _PRECOMPUTEIBL_RENDERPASS_
#define _PRECOMPUTEIBL_RENDERPASS_

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
#include "../../Mesh/MeshBase.h"

namespace MXRender { class GameObject; }

namespace MXRender { class ComponentBase; }

namespace MXRender { class VK_Texture; }
namespace MXRender { class MeshBase; }
namespace MXRender { class VK_DescriptorSetLayout; }


namespace MXRender
{


    class PreComputeIBL_RenderPass :public VK_RenderPass
    {
    private:
        std::vector<float> input_data;
    protected:

        void setup_descriptorset_layout();
        void setup_pipelines();
        void setup_buffer();
        void setup_descriptorpool();
        void setup_descriptorsets();
        
        void update_buffer();

		std::vector<VkBuffer> buffers;
		std::vector<VkDeviceMemory> buffers_memory;

        VkDescriptorPool descriptor_pool;
        std::vector<VkDescriptorSet> descriptor_sets;

        std::shared_ptr< VK_DescriptorSetLayout> descriptorset_layout;
        std::shared_ptr< VK_Texture> cubemap_texture;


    public:
        virtual void post_initialize();
        virtual void set_commonInfo(const PassInfo& init_info);
        virtual void prepare_pass_data(const GraphicsContext& context);
        virtual void draw(GraphicsContext* context, RenderScene* render_scene=nullptr);

        PreComputeIBL_RenderPass();
        PreComputeIBL_RenderPass(const PassInfo& init_info);
        virtual ~PreComputeIBL_RenderPass();

        virtual VkRenderPass& get_render_pass() ;


        virtual void initialize(const PassInfo& init_info, PassOtherInfo* other_info) override;

        void print(GraphicsContext* context);
    };



}
#endif
