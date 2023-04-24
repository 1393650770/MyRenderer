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
#include "../../Mesh/MeshBase.h"
#include <mutex>

namespace MXRender { struct RenderObject; }

namespace MXRender { struct MeshObject; }


namespace MXRender { class GameObject; }

namespace MXRender { class ComponentBase; }

namespace MXRender { class VK_Texture; }
namespace MXRender { class MeshBase; }
namespace MXRender { class VK_DescriptorSetLayout; }


namespace MXRender
{
    struct Mesh_PassInfo:PassInfo
    {
    public:
        VkRenderPass renderpass;
    };

    struct DynamicCPUUniformBuffer
    {
        void init(VK_GraphicsContext* context, VkDeviceMemory gpu_memory);
        void shutdown(VK_GraphicsContext* context, VkDeviceMemory gpu_memory);
		template<typename T>
		uint32_t push(T& data);

		uint32_t push(void* data, size_t size);

        uint32_t update(void* data, size_t size,uint32_t offset);
        void reset();
        uint32_t pad_uniform_buffer_size(uint32_t originalSize);
		uint32_t align;
		uint32_t currentOffset;
        std::mutex mtx;
		void* mapped;
    };

	template<typename T>
	uint32_t DynamicCPUUniformBuffer::push(T& data)
	{
        std::lock_guard<std::mutex> lock(mtx);
		return push(&data, sizeof(T));
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
        
        void update_object_uniform(GameObject* game_object,VkPipelineLayout pipeline_layout);
        void update_camera_uniform();
        void render_mesh(ComponentBase* mesh_component);
        void render_mesh(MeshObject* mesh_component, VkDescriptorSet GlobalSet,VkCommandBuffer command_buffer);

        void render_mesh(RenderScene* render_scene, RenderObject* render_object,VkDescriptorSet global_set,VkCommandBuffer command_buffer);
        void dispatch_render_mesh(RenderScene* render_scene, unsigned int start_index , unsigned int end_index, VkDescriptorSet GlobalSet);
        void dispatch_gpudriven_render_mesh(RenderScene* render_scene, unsigned int start_index, unsigned int end_index, VkDescriptorSet global_set, VkDescriptorSet object_data_set);
		std::vector<VkBuffer> uniform_buffers;
		std::vector<VkDeviceMemory> uniform_buffers_memory;

        VkDescriptorPool descriptor_pool;
        std::vector<VkDescriptorSet> descriptor_sets;

        std::shared_ptr< VK_DescriptorSetLayout> descriptorset_layout;
        std::shared_ptr< VK_DescriptorSetLayout> descriptorset_layout2;
        std::shared_ptr< VK_Texture> cubemap_texture;

        DynamicCPUUniformBuffer cpu_ubo_buffer;
        void draw_gpudriven(GraphicsContext* context, RenderScene* render_scene = nullptr);
    public:
        virtual void post_initialize();
        virtual void set_commonInfo(const PassInfo& init_info);
        virtual void prepare_pass_data(const GraphicsContext& context);
        virtual void draw(GraphicsContext* context, RenderScene* render_scene=nullptr);

       
        Mesh_RenderPass();
        Mesh_RenderPass(const PassInfo& init_info);
        virtual ~Mesh_RenderPass();

        virtual VkRenderPass& get_render_pass() ;


        virtual void initialize(const PassInfo& init_info, PassOtherInfo* other_info) override;

    };



}
#endif
