#pragma once
#ifndef _PIPELINESHADEROBJECT_
#define _PIPELINESHADEROBJECT_

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
#include "../../RHI/Vulkan/VK_DescriptorSets.h"
#include "../../Logic/GameObjectManager.h"
#include "../../AssetLoader/material_asset.h"

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

	enum MeshpassType : int
	{
		Forward = 0,
		Transparency,
		DirectionalShadow,
		Count
	};



	template<typename T>
	struct PerPassData {

	public:
		T& operator[](MeshpassType pass)
		{

			return data[pass];
			//assert(false);
			//return data[0];
		};

		void clear(T&& val)
		{
			for (int i = 0; i < MeshpassType::Count; i++)
			{
				data[i] = val;
			}
		}

	private:
		T data[MeshpassType::Count];
	};

    class PipelineShaderObject 
    {
    private:
    protected:
        VK_Shader * shader{nullptr};

    public:
		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout pipeline_layout{ VK_NULL_HANDLE };

        PipelineShaderObject();
        virtual ~PipelineShaderObject();

       void init(VkDevice device, VK_Shader* in_shader , PipelineBuilder* in_pipeline_builder, VkRenderPass renderPass );

    };

	struct ShaderParameters
	{

	};

	struct SampledTexture {
		VkSampler sampler;
		VkImageView view;
	};
	struct EffectTemplate {
		PerPassData<PipelineShaderObject*> pass_pso;
		ShaderParameters* default_parameters;
		assets::TransparencyMode transparency;
	};
	class Material
	{
	private:
	protected:
		
	public:
		EffectTemplate* pass_pso;
		PerPassData<VkDescriptorSet> pass_sets;
		std::vector<SampledTexture> textures;
		ShaderParameters* parameters;
        Material();
		virtual ~Material();
        Material& operator=(const Material& other) = default;

	};
	struct MaterialData {
		std::vector<SampledTexture> textures;
		ShaderParameters* parameters;
		EffectTemplate* psos;
		std::string baseTemplate;
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
		std::unordered_map<std::string, EffectTemplate> templateCache;
		std::unordered_map<std::string, Material*> materials;
		std::unordered_map<MaterialData, Material*, MaterialInfoHash> materialCache;
		PipelineShaderObject* build_pso(VkRenderPass renderPass, PipelineBuilder& builder, VK_Shader* effect);
		PipelineShaderObject* build_comp_pso(VK_Shader* effect);
		Material* build_material(const std::string& materialName, const MaterialData& info);
		void build_pipeline_builder();
		void build_default_pso();
		Material* get_material(const std::string& materialName);
		VK_DescriptorPool* get_descript_pool() const;
		MaterialSystem();
		virtual ~MaterialSystem();
		void init (VK_GraphicsContext* context);
		void destroy();
	};
}
#endif
