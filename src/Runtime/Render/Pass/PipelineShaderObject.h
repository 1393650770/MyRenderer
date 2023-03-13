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
#include "../RenderScene.h"
#include "../../RHI/Vulkan/VK_DescriptorSets.h"

namespace MXRender { class VK_DescriptorPool; }

namespace MXRender { class PassProcessor; }

namespace MXRender { class GraphicsContext; }

namespace MXRender { class VK_GraphicsContext; }

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

	struct ShaderParameters
	{

	};

	class Material
	{
	private:
	protected:
		
	public:
		PerPassData<PipelineShaderObject*> pass_pso;
		PerPassData<VkDescriptorSet> pass_sets;

        Material();
		virtual ~Material();
        Material& operator=(const Material& other) = default;

	};
	struct MaterialData {
		std::vector<VK_Texture> textures;
		ShaderParameters* parameters;
		PerPassData<PipelineShaderObject*> psos;

		bool operator==(const MaterialData& other) const;

		size_t hash() const;
	};

	struct MaterialInfoHash
	{

		std::size_t operator()(const MaterialData& k) const
		{
			return k.hash();
		}
	};

	class MaterialSystem
	{
	private:
	protected:
		VK_GraphicsContext* context;
		VK_DescriptorPool* descriptor_pool;
		PipelineBuilder mesh_pass_builder;
	public:
		std::unordered_map<std::string, VK_Shader*> shaders;
		std::unordered_map<std::string, PipelineShaderObject*> psos;
		std::unordered_map<std::string, Material*> materials;
		std::unordered_map<MaterialData, Material*, MaterialInfoHash> materialCache;
		PipelineShaderObject* build_pso(VkRenderPass renderPass, PipelineBuilder& builder, VK_Shader* effect);
		Material* build_material(const std::string& materialName, const MaterialData& info);
		void build_pipeline_builder();
		MaterialSystem();
		virtual ~MaterialSystem();
		void init (VK_GraphicsContext* context);

	};
}
#endif
