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
#include "../../Mesh/GL_Mesh.h"
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

	MXRender::PipelineShaderObject* MaterialSystem::build_comp_pso( VK_Shader* effect)
	{
		PipelineShaderObject* new_pso = new PipelineShaderObject();
		ComputePipelineBuilder builder;
		builder._pipelineLayout = effect->get_built_layout();
		builder._shaderStage = VK_Utils::Pipeline_Shader_Stage_Create_Info(VK_SHADER_STAGE_COMPUTE_BIT, effect->shader_modules[ENUM_SHADER_STAGE::Shader_Compute]);
		new_pso->pipeline_layout = effect->get_built_layout();
		new_pso->pipeline=builder.build_pipeline(context->device->device);
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
			newMat->pass_pso = &templateCache[info.baseTemplate];
			newMat->parameters = info.parameters;
			newMat->textures=info.textures;
			////not handled yet
			newMat->pass_sets[MeshpassType::DirectionalShadow] = VK_NULL_HANDLE;

			DescriptorBuilder db = DescriptorBuilder::begin(descriptor_pool);
			db.image_infos.resize(info.textures.size());
			for (int i = 0; i < info.textures.size(); i++)
			{
				db.image_infos[i].sampler = info.textures[i].sampler;
				db.image_infos[i].imageView = info.textures[i].view;
				db.image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				
				db.bind_image(i, &db.image_infos[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			}

				
			db.build(newMat->pass_sets[MeshpassType::Forward]);
			db.build(newMat->pass_sets[MeshpassType::Transparency]);
			//add material to cache
			materialCache[info] = (newMat);
			mat = newMat;
			materials[materialName] = mat;
		}

		return mat;
	}

	void MaterialSystem::build_pipeline_builder()
	{

		{
			mesh_pass_builder.vertexDescription= SimpleVertex::get_vertex_description();
			
			mesh_pass_builder._inputAssembly = VK_Utils::Input_Assembly_Create_Info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);


			mesh_pass_builder._rasterizer = VK_Utils::Rasterization_State_Create_Info(VK_POLYGON_MODE_FILL);
			mesh_pass_builder._rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;//BACK_BIT;
			mesh_pass_builder._rasterizer.frontFace= VK_FRONT_FACE_COUNTER_CLOCKWISE;
			mesh_pass_builder._multisampling = VK_Utils::Multisampling_State_Create_Info();

			mesh_pass_builder._colorBlendAttachment = VK_Utils::Color_Blend_Attachment_State();

			//default depthtesting
			mesh_pass_builder._depthStencil = VK_Utils::Depth_Stencil_Create_Info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
		}
	}

	void MaterialSystem::build_default_pso()
	{

	}

	MXRender::Material* MaterialSystem::get_material(const std::string& materialName)
	{
		auto it = materials.find(materialName);
		if (it != materials.end())
		{
			return(*it).second;
		}
		else {
			return nullptr;
		}
	}

	MXRender::VK_DescriptorPool* MaterialSystem::get_descript_pool() const
	{
		return descriptor_pool;
	}

	MaterialSystem::MaterialSystem()
	{

	}

	MaterialSystem::~MaterialSystem()
	{
		delete descriptor_pool;
		for (auto& it : shaders)
		{
			delete it.second;
		}

		for (auto& it : psos)
		{
			delete it.second;
		}

		for (auto& it : materials)
		{
			delete it.second;
		}


	}

	void MaterialSystem::init(VK_GraphicsContext* context)
	{
		this->context=context;
		build_pipeline_builder();
		descriptor_pool=new VK_DescriptorPool(context->device,50000);

		VK_Shader* default_color = new  VK_Shader(context->device, "Shader/default_prefabs_mesh_vert.spv", "Shader/default_prefabs_mesh_frag.spv");
		VK_Shader* pbr_material = new  VK_Shader(context->device, "Shader/pbr_mesh_vert.spv", "Shader/pbr_mesh_frag.spv");
		VK_Shader* upload_comp= new VK_Shader(context->device, "", "","","Shader/upload_comp.spv");
		VK_Shader::ReflectionOverrides overrides[] = {
			{"mvp", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC}	
		};
		default_color->reflect_layout(overrides, 1);
		pbr_material->reflect_layout(overrides, 1);
		upload_comp->reflect_layout(nullptr,0);

		shaders["upload_comp"]=upload_comp;
		shaders["default_mesh"] = default_color;
		shaders["pbr_mesh"]=pbr_material;

		PipelineShaderObject* mesh_pso =  build_pso(context->mesh_pass, mesh_pass_builder, default_color);
		PipelineShaderObject* pbr_mesh_pso = build_pso(context->mesh_pass, mesh_pass_builder, pbr_material);
		PipelineShaderObject* upload_comp_pso= build_comp_pso(upload_comp);

		psos["mesh_pass"] = mesh_pso;
		psos["pbr_pass"] = pbr_mesh_pso;
		psos["upload_comp"] = upload_comp_pso;

		templateCache["mesh_base"].pass_pso[MeshpassType::Forward]= mesh_pso;
		templateCache["mesh_base"].pass_pso[MeshpassType::Transparency] = nullptr;
		templateCache["mesh_base"].pass_pso[MeshpassType::DirectionalShadow] = nullptr;

		templateCache["mesh_pbr"].pass_pso[MeshpassType::Forward] = pbr_mesh_pso;
		templateCache["mesh_pbr"].pass_pso[MeshpassType::Transparency] = nullptr;
		templateCache["mesh_pbr"].pass_pso[MeshpassType::DirectionalShadow] = nullptr;

	}

	bool MaterialData::operator==(const MaterialData& other) const
	{
		if (other.baseTemplate.compare(baseTemplate) != 0 || other.parameters != parameters || other.textures.size() != textures.size())
		{
			return false;
		}
		else {
			//binary compare textures
			bool comp = memcmp(other.textures.data(), textures.data(), textures.size() * sizeof(textures[0])) == 0;
			return comp;
		}
	}

	size_t MaterialData::hash() const
	{
		using std::size_t;
		using std::hash;

		size_t result =1;

		for (const auto& b : textures)
		{
			//pack the binding data into a single int64. Not fully correct but its ok
			size_t texture_hash = (std::hash<size_t>()((size_t)b.sampler) << 3) && (std::hash<size_t>()((size_t)b.view) >> 7);

			//shuffle the packed binding data and xor it with the main hash
			result ^= std::hash<size_t>()(texture_hash);
		}

		return result;
	}

}
