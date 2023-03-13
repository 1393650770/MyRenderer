#include "PipelineShaderObject.h"
#include "../../RHI/Vulkan/VK_GraphicsContext.h"
#include "../../RHI/Vulkan/VK_Device.h"
#include "../../RHI/Vulkan/VK_Shader.h"
#include "../../RHI/Vulkan/VK_VertexArray.h"
#include "../../RHI/Vulkan/VK_DescriptorSets.h"
#include "../../RHI/Vulkan/VK_Viewport.h"
#include "../../RHI/Vulkan/VK_SwapChain.h"
#include "../DefaultSetting.h"
#include "../../Utils/Singleton.h"
#include "vulkan/vulkan_core.h"
#include "../../RHI/RenderEnum.h"
#include "../../RHI/UniformBuffer.h"
#include "../../RHI/Vulkan/VK_Utils.h"
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../RHI/Vulkan/VK_Texture.h"
#include "../../RHI/Vulkan/VK_Shader.h"
#include <array>
#include "../../RHI/Vulkan/VK_RenderPass.h"
namespace MXRender
{

	
	PipelineShaderObject::PipelineShaderObject()
	{

	}

	PipelineShaderObject::~PipelineShaderObject()
	{

	}

	void PipelineShaderObject::init(VkDevice device, VK_Shader* in_shader, PipelineBuilder* in_pipeline_builder, VkRenderPass in_renderpass )
	{
		shader=in_shader;
		pipeline_layout=in_shader->get_built_layout();
		in_pipeline_builder->set_shaders(shader);
		pipeline = in_pipeline_builder->build_pipeline(device, in_renderpass);
	}

	Material::Material()
	{

	}

	Material::~Material()
	{

	}

	MXRender::PipelineShaderObject* MaterialSystem::build_pso(VkRenderPass renderPass, PipelineBuilder& builder, VK_Shader* effect)
	{
		PipelineShaderObject* new_pso = new PipelineShaderObject();
		new_pso->init(context->device->device,effect,&builder,renderPass);
		return new_pso;
	}

	Material* MaterialSystem::build_material(const std::string& materialName, const MaterialData& info)
	{
		Material* mat;
		//search material in the cache first in case its already built
		auto it = materialCache.find(info);
		if (it != materialCache.end())
		{
			mat = (*it).second;
			materials[materialName] = mat;
		}
		else 
		{

			//need to build the material
			Material* newMat = new Material();
			newMat->pass_pso=info.psos;
			//newMat->original = &templateCache[info.baseTemplate];
			//newMat->parameters = info.parameters;
			////not handled yet
			newMat->pass_sets[MeshpassType::DirectionalShadow] = VK_NULL_HANDLE;

			DescriptorBuilder db = DescriptorBuilder::begin(descriptor_pool);

			for (int i = 0; i < info.textures.size(); i++)
			{
				VkDescriptorImageInfo imageBufferInfo;
				imageBufferInfo.sampler = info.textures[i].textureSampler;
				imageBufferInfo.imageView = info.textures[i].textureImageView;
				imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				
				db.bind_image(i, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			}

			descriptor_pool->allocate_descriptorsets();
				
			db.build(newMat->pass_sets[MeshpassType::Forward]);
			db.build(newMat->pass_sets[MeshpassType::Transparency]);
			//LOG_INFO("Built New Material {}", materialName);
			//add material to cache
			materialCache[info] = (newMat);
			//mat = newMat;
			materials[materialName] = mat;
		}

		return mat;
	}

	void MaterialSystem::build_pipeline_builder()
	{
		
	}

	MaterialSystem::MaterialSystem()
	{

	}

	MaterialSystem::~MaterialSystem()
	{

	}

	void MaterialSystem::init(VK_GraphicsContext* context)
	{
		this->context=context;

		VK_Shader* test = new  VK_Shader(context->device, "Shader/mesh_rock_vert.spv", "Shader/mesh_rock_frag.spv");
		test->reflect_layout(nullptr,0);
		shaders["default_mesh"] = test;

		PipelineShaderObject* mesh_pso =  build_pso(context->mesh_pass, mesh_pass_builder,test);
		psos["mesh_pass"] = mesh_pso;
		MaterialData data;
		Material* mat= build_material("mesh_base",data);

		descriptor_pool=new VK_DescriptorPool(context->device,50);
	}

	size_t MaterialData::hash() const
	{
		using std::size_t;
		using std::hash;

		size_t result =1;

		for (const auto& b : textures)
		{
			//pack the binding data into a single int64. Not fully correct but its ok
			size_t texture_hash = (std::hash<size_t>()((size_t)b.textureSampler) << 3) && (std::hash<size_t>()((size_t)b.textureImageView) >> 7);

			//shuffle the packed binding data and xor it with the main hash
			result ^= std::hash<size_t>()(texture_hash);
		}

		return result;
	}

}
