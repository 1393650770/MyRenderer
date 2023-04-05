#include "GameObjectManager.h"
#include "../RHI/RenderState.h"
#include "../RHI/RenderEnum.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Mesh/MeshBase.h"
#include "../AssetLoader/prefab_asset.h"
#include "../RHI/Vulkan/VK_Utils.h"
#include "../Render/RenderScene.h"
#include "../Render/Pass/PipelineShaderObject.h"
#include "../AssetLoader/material_asset.h"
#include "../RHI/Vulkan/VK_Texture.h"
#include "../Mesh/VK_Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
std::string asset_path(std::string_view path)
{
	return "../../../../assets_export/" + std::string(path);
}



std::string shader_path(std::string_view path)
{
	return "../../../../shaders/" + std::string(path);
}

bool MXRender::GameObjectManager::load_image_to_cache(GraphicsContext* context,const char* name, const char* path)
{
	VK_GraphicsContext* vk_context = nullptr;
	switch (RenderState::render_api_type)
	{
	case ENUM_RENDER_API_TYPE::Vulkan:
	{
		vk_context = dynamic_cast<VK_GraphicsContext*>(context);
	}
	break;
	default:
		return false;
	}

	CacheTexture newtex;

	if (_loadedTextures.find(name) != _loadedTextures.end()) return true;

	bool result = VK_Utils::Load_Image_From_Asset(vk_context, path, newtex.image);

	if (!result)
	{

		return false;
	}

	newtex.imageView = newtex.image._defaultView;

	_loadedTextures[name] = newtex;
	return true;
}

MXRender::MeshBase* MXRender::GameObjectManager::get_mesh(const std::string& name)
{
	auto it = _meshes.find(name);
	if (it == _meshes.end()) {
		return nullptr;
	}
	else {
		return (*it).second;
	}
}

MXRender::GameObjectManager::GameObjectManager(GraphicsContext* context)
{
	//object_list.emplace_back("Resource/Mesh/viking_room.obj");

	//object_list.emplace_back("Resource/Mesh/rock.obj");

	object_list.emplace_back("viking_room1","Resource/Mesh/viking_room.obj");
	object_list.emplace_back("viking_room2", "Resource/Mesh/viking_room.obj");
	//object_list.emplace_back("Resource/Mesh/sponza.obj");

}

MXRender::GameObjectManager::~GameObjectManager()
{

}

void MXRender::GameObjectManager::destroy_object_list(GraphicsContext* context)
{
	switch (RenderState::render_api_type)
	{
	case ENUM_RENDER_API_TYPE::Vulkan:
	{
		VK_GraphicsContext* vk_context = dynamic_cast<VK_GraphicsContext*>(context);
		if (!vk_context ) return;
		
		for (int i=0;i<object_list.size();i++)
		{
			object_list[i].get_staticmesh()->get_mesh_data().lock()->destroy_mesh_info(context);
		}

		for (auto& it :_meshes)
		{
			it.second->destroy_mesh_info(context);
		}
		break;
	}

	default:
		break;
	}
}

void MXRender::GameObjectManager::start_load_prefabs(GraphicsContext* context)
{
	int dimHelmets = 1;
	for (int x = -dimHelmets; x <= dimHelmets; x++) {
		for (int y = -dimHelmets; y <= dimHelmets; y++) {

			glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(x * 5, 0, y * 5));
			glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(10));

			load_prefab(asset_path("FlightHelmet/FlightHelmet.pfb").c_str(), (translation * scale), context);
		}
	}

	glm::mat4 sponzaMatrix = glm::scale(glm::mat4{ 1.0f }, glm::vec3(3.0f));;

	glm::mat4 unrealFixRotation = glm::rotate(glm::radians(-90.f), glm::vec3{ 1,0,0 });

	load_prefab(asset_path("Sponza/Sponza.pfb").c_str(), sponzaMatrix, context);
}

bool MXRender::GameObjectManager::load_prefab(const char* path, glm::mat4 root,GraphicsContext* context)
{
	int rng = rand();
	VK_GraphicsContext* vk_context=nullptr;
	switch (RenderState::render_api_type)
	{
	case ENUM_RENDER_API_TYPE::Vulkan:
	{
		vk_context = dynamic_cast<VK_GraphicsContext*>(context);
	}
	break;
	default:
		return false;
	}

	auto pf = _prefabCache.find(path);
	if (pf == _prefabCache.end())
	{
		assets::AssetFile file;
		bool loaded = assets::load_binaryfile(path, file);

		if (!loaded) {
			return false;
		}
		else {
		}

		_prefabCache[path] = new assets::PrefabInfo;

		*_prefabCache[path] = assets::read_prefab_info(&file);
	}

	assets::PrefabInfo* prefab = _prefabCache[path];

	VkSamplerCreateInfo samplerInfo = VK_Utils::Sampler_Create_Info(VK_FILTER_LINEAR);
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;


	VkSampler smoothSampler;
	vkCreateSampler(vk_context->device->device, &samplerInfo, nullptr, &smoothSampler);


	std::unordered_map<uint64_t, glm::mat4> node_worldmats;

	std::vector<std::pair<uint64_t, glm::mat4>> pending_nodes;
	for (auto& [k, v] : prefab->node_matrices)
	{

		glm::mat4 nodematrix{ 1.f };

		auto nm = prefab->matrices[v];
		memcpy(&nodematrix, &nm, sizeof(glm::mat4));

		//check if it has parents
		auto matrixIT = prefab->node_parents.find(k);
		if (matrixIT == prefab->node_parents.end()) {
			//add to worldmats 
			node_worldmats[k] = root * nodematrix;
		}
		else {
			//enqueue
			pending_nodes.push_back({ k,nodematrix });
		}
	}

	//process pending nodes list until it empties
	while (pending_nodes.size() > 0)
	{
		for (int i = 0; i < pending_nodes.size(); i++)
		{
			uint64_t node = pending_nodes[i].first;
			uint64_t parent = prefab->node_parents[node];

			//try to find parent in cache
			auto matrixIT = node_worldmats.find(parent);
			if (matrixIT != node_worldmats.end()) {

				//transform with the parent
				glm::mat4 nodematrix = (matrixIT)->second * pending_nodes[i].second;

				node_worldmats[node] = nodematrix;

				//remove from queue, pop last
				pending_nodes[i] = pending_nodes.back();
				pending_nodes.pop_back();
				i--;
			}
		}

	}



	for (auto& [k, v] : prefab->node_meshes)
	{

		//load mesh

		if (v.mesh_path.find("Sky") != std::string::npos) {
			continue;
		}

		if (!get_mesh(v.mesh_path.c_str()))
		{
			MeshBase* mesh=new VK_Mesh();
			mesh->load_asset(asset_path(v.mesh_path).c_str());
			_meshes[v.mesh_path.c_str()] = mesh;
		}


		auto materialName = v.material_path.c_str();
		//load material
		
		Material* objectMaterial = vk_context->material_system.get_material(materialName);
		if (!objectMaterial)
		{
			assets::AssetFile materialFile;
			bool loaded = assets::load_binaryfile(asset_path(materialName).c_str(), materialFile);

			if (loaded)
			{
				assets::MaterialInfo material = assets::read_material_info(&materialFile);

				auto texture = material.textures["baseColor"];
				if (texture.size() <= 3)
				{
					texture = "Sponza/white.tx";
				}

				loaded = load_image_to_cache(vk_context,texture.c_str(), asset_path(texture).c_str());

				if (loaded)
				{

					SampledTexture tex;
					tex.view = _loadedTextures[texture].imageView;
					tex.sampler = smoothSampler;

					MaterialData info;
					info.parameters = nullptr;

					if (material.transparency == assets::TransparencyMode::Transparent)
					{
						info.baseTemplate = "mesh_base";
					}
					else {
						info.baseTemplate = "mesh_base";
					}

					info.textures.push_back(tex);

					objectMaterial = vk_context->material_system.build_material(materialName, info);

					if (!objectMaterial)
					{
						std::cout<<"Error When building material"<< v.material_path;
					}
				}
				else
				{
					std::cout << "Error When loading image at" << v.material_path;
				}
			}
			else
			{
				std::cout << "Error When loading material at path" << v.material_path;
			}
		}

		MeshObject loadmesh;
		//transparent objects will be invisible

		loadmesh.bDrawForwardPass = true;
		loadmesh.bDrawShadowPass = true;


		glm::mat4 nodematrix{ 1.f };

		auto matrixIT = node_worldmats.find(k);
		if (matrixIT != node_worldmats.end()) {
			auto nm = (*matrixIT).second;
			memcpy(&nodematrix, &nm, sizeof(glm::mat4));
		}

		loadmesh.mesh = get_mesh(v.mesh_path.c_str());
		loadmesh.transformMatrix = nodematrix;
		loadmesh.material = objectMaterial;

		//refresh_renderbounds(&loadmesh);

		//sort key from location
		int32_t lx = int(loadmesh.bounds.origin.x / 10.f);
		int32_t ly = int(loadmesh.bounds.origin.y / 10.f);

		uint32_t key = uint32_t(std::hash<int32_t>()(lx) ^ std::hash<int32_t>()(ly ^ 1337));

		loadmesh.customSortKey = 0;// rng;// key;


		prefab_renderables.push_back(loadmesh);
	}
	return true;
}
