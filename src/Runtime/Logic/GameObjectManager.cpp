#include "GameObjectManager.h"
#include "../RHI/RenderState.h"
#include "../RHI/RenderEnum.h"
#include "../RHI/Vulkan/VK_Utils.h"
#include "../AssetLoader/prefab_asset.h"
#include "../RHI/Vulkan/VK_Utils.h"
#include "../Render/RenderScene.h"
#include "../Render/Pass/PipelineShaderObject.h"
#include "../AssetLoader/material_asset.h"
#include "../Mesh/TextBase.h"
#include "../Mesh/VK_Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "../Render/DefaultSetting.h"
#include "../Render/TextureManager.h"
#include "../Utils/Singleton.h"
#include "TaskScheduler.h"
#include "optick.h"
#include <xutility>


std::string asset_path(std::string_view path)
{
	return "../../../../assets_export/" + std::string(path);
}



std::string shader_path(std::string_view path)
{
	return "../../../../shaders/" + std::string(path);
}

void MXRender::GameObjectManager::load_objs()
{
	OPTICK_PUSH("Load_Obj")
	object_list.emplace_back("viking_room1", "Resource/Mesh/viking_room.obj");
	object_list.emplace_back("viking_room2", "Resource/Mesh/viking_room.obj");

	_meshes["Resource/Mesh/viking_room.obj"] = object_list.back().get_staticmesh()->get_mesh_data();

	//object_list.emplace_back("viking_room3", "Resource/Mesh/viking_room.obj");
	//object_list.emplace_back("viking_room4", "Resource/Mesh/viking_room.obj");
	//object_list.emplace_back("viking_room5", "Resource/Mesh/viking_room.obj");

	object_list.emplace_back("pbr_stone", "Resource/Mesh/pbr_stone.obj");
	_meshes["Resource/Mesh/pbr_stone.obj"] = object_list.back().get_staticmesh()->get_mesh_data();
	object_list.back().get_transform()->set_scale(glm::vec3(0.005f));
	object_list.back().get_transform()->set_translation(glm::vec3(-8.005f, 0.0f, 0.0f));

	object_list.emplace_back("common_stone");
	object_list.back().get_staticmesh()->reset_mesh(_meshes["Resource/Mesh/pbr_stone.obj"]);
	object_list.back().get_transform()->set_scale(glm::vec3(0.005f));
	object_list.back().get_transform()->set_translation(glm::vec3(-15.005f, 0.0f, 0.0f));
	object_list.emplace_back("Sdf_Text");
	std::cout << "vulkan load ttf 0" << std::endl;
	TextComponent* text_component = new TextComponent("Resource/Text/AlibabaPuHuiTi-3-105-Heavy.ttf");
	_text_ttfes["Resource/Text/AlibabaPuHuiTi-3-105-Heavy.ttf"] = text_component->get_text_data();
	object_list.back().add_component(text_component);
	object_list.back().get_component(TextComponent)->reset_text_content("BCDEFG");
	object_list.back().get_transform()->set_translation(glm::vec3(-15.005f, 0.0f, -400.0f));
	OPTICK_POP()
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
	main_camera.set_position(glm::vec3(-1.0f,30.0f,0.0f));
	//main_camera.set_direction(glm::vec3(0.0f, 1.0f, 0.0f));
	//object_list.emplace_back("Resource/Mesh/viking_room.obj");

	//object_list.emplace_back("Resource/Mesh/rock.obj");

	//object_list.emplace_back("viking_room1","Resource/Mesh/viking_room.obj");
	//object_list.emplace_back("viking_room2", "Resource/Mesh/viking_room.obj");
	//object_list.emplace_back("pbr_stone", "Resource/Mesh/pbr_stone.obj");
	//object_list[2].get_transform()->set_scale(glm::vec3(0.005f));
	//object_list[2].get_transform()->set_translation(glm::vec3(-8.005f,0.0f,0.0f));

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

		for (auto& it : _meshes)
		{
			it.second->destroy_mesh_info(context);
			delete it.second;
			_meshes[it.first]=nullptr;
		}
		_meshes.clear();
		for (auto& it : _text_ttfes)
		{
			it.second->destroy_text_info(context);
			delete it.second;
			_text_ttfes[it.first] = nullptr;
		}
		_text_ttfes.clear();
		for (int i=0;i<object_list.size();i++)
		{
			object_list[i].get_staticmesh()->get_mesh_data()->destroy_mesh_info(context);
		}


		break;
	}

	default:
		break;
	}
	object_list.clear();

}

void MXRender::GameObjectManager::start_load_prefabs(GraphicsContext* context)
{
	int dimHelmets = 15;
	int i=0;
	for (int x = -dimHelmets; x <= dimHelmets; x++) {
		for (int y = -dimHelmets; y <= dimHelmets; y++) {

			glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(-x * 3, -(x+y)*3, y * 3));
			glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(10));
			i++;
			load_prefab("FlightHelmet"+ std::to_string(i), asset_path("FlightHelmet/FlightHelmet.pfb").c_str(), (translation * scale), context);
		}
	}

	glm::mat4 sponzaMatrix = glm::scale(glm::mat4{ 1.0f }, glm::vec3(3.0f));;

	glm::mat4 unrealFixRotation = glm::rotate(glm::radians(-90.f), glm::vec3{ 1,0,0 });

	load_prefab("Sponza", asset_path("Sponza/Sponza.pfb").c_str(), sponzaMatrix, context);
}

void MXRender::GameObjectManager::set_overload_material(GraphicsContext* context)
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
		return ;
	}

	for (int i=0;i<3;i++)
	{
		object_list[i].get_staticmesh()->get_mesh_data()->init_mesh_info(context);
	}

	Material* stoneMaterial = Singleton<DefaultSetting>::get_instance().material_system->get_material("pbr_mesh");
	if (!stoneMaterial)
	{
		VK_Texture* aorm_texture =  Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("pbr_stone_aorm",ENUM_TEXTURE_TYPE::ENUM_TYPE_2D,"Resource/Texture/pbr_stone/pbr_stone_aorm.dds");
		VK_Texture* base_color_texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("pbr_stone_base_color", ENUM_TEXTURE_TYPE::ENUM_TYPE_2D, "Resource/Texture/pbr_stone/pbr_stone_base_color.dds");
		VK_Texture* normal_texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("pbr_stone_normal", ENUM_TEXTURE_TYPE::ENUM_TYPE_2D, "Resource/Texture/pbr_stone/pbr_stone_normal.dds");
		VK_Texture* cubemap_texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("skybox", ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP, "Resource/Texture/Skybox/kyoto_lod.dds");
		VK_Texture* cubemap_irr_texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("skybox_irr", ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP, "Resource/Texture/Skybox/kyoto_irr.dds");
		//VK_Texture* cubemap_texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("skybox", ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP, "Resource/Texture/Skybox/bolonga_lod.dds");
		//VK_Texture* cubemap_irr_texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("skybox_irr", ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP, "Resource/Texture/Skybox/bolonga_lod.dds");

		VK_Texture* lut_texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("ibl_lut", ENUM_TEXTURE_TYPE::ENUM_TYPE_2D, "Resource/Texture/ibl_lut.png");

		MaterialData info;
		info.parameters = nullptr;
		info.textures.clear();
		SampledTexture tex;
		tex.view = base_color_texture->textureImageView;
		tex.sampler = base_color_texture->textureSampler;
		info.textures.push_back(tex);
		tex.view = normal_texture->textureImageView;
		tex.sampler = normal_texture->textureSampler;
		info.textures.push_back(tex);
		tex.view = aorm_texture->textureImageView;
		tex.sampler = aorm_texture->textureSampler;
		info.textures.push_back(tex);
		tex.view = cubemap_texture->textureImageView;
		tex.sampler = cubemap_texture->textureSampler;
		tex.texture_type= ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP;
		info.textures.push_back(tex);
		tex.view = cubemap_irr_texture->textureImageView;
		tex.sampler = cubemap_irr_texture->textureSampler;
		tex.texture_type = ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP;
		info.textures.push_back(tex);
		tex.view = lut_texture->textureImageView;
		tex.sampler = lut_texture->textureSampler;
		tex.texture_type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
		info.textures.push_back(tex);

		info.baseTemplate = Singleton<DefaultSetting>::get_instance().material_system->create_template_name("mesh_pbr");

		stoneMaterial = Singleton<DefaultSetting>::get_instance().material_system->build_material("pbr_stone", info);

		if (!stoneMaterial)
		{
			std::cout << "Error When building material" ;
		}
		else
		{
			object_list[2].set_material(stoneMaterial);
		}
	}
	Material* common_stoneMaterial = Singleton<DefaultSetting>::get_instance().material_system->get_material("command_stone");
	if (!common_stoneMaterial)
	{

		VK_Texture* base_color_texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("pbr_stone_base_color", ENUM_TEXTURE_TYPE::ENUM_TYPE_2D, "Resource/Texture/pbr_stone/pbr_stone_base_color.dds");
		

		MaterialData info;
		info.parameters = nullptr;
		info.textures.clear();
		SampledTexture tex;
		tex.view = base_color_texture->textureImageView;
		tex.sampler = base_color_texture->textureSampler;
		info.textures.push_back(tex);
		

		info.baseTemplate = Singleton<DefaultSetting>::get_instance().material_system->create_template_name("mesh_base_revert_uv");

		common_stoneMaterial = Singleton<DefaultSetting>::get_instance().material_system->build_material("command_stone", info);

		if (!common_stoneMaterial)
		{
			std::cout << "Error When building material";
		}
		else
		{
			object_list[3].set_material(common_stoneMaterial);
		}
	}

	if (Singleton<DefaultSetting>::get_instance().is_enable_gpu_driven)
	{
	
		Material* gpu_driven_test = Singleton<DefaultSetting>::get_instance().material_system->get_material("gpu_driven_default");
		if (!gpu_driven_test)
		{

			std::string texture_path = "Resource/Texture/viking_room.png";
			VK_Texture* texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("viking_room", ENUM_TEXTURE_TYPE::ENUM_TYPE_2D, texture_path);
			MaterialData info;
			info.parameters = nullptr;
			info.textures.clear();
			SampledTexture tex;
			tex.view = texture->textureImageView;
			tex.sampler = texture->textureSampler;
			info.textures.push_back(tex);
			info.baseTemplate = Singleton<DefaultSetting>::get_instance().material_system->create_template_name("mesh");

			gpu_driven_test = Singleton<DefaultSetting>::get_instance().material_system->build_material("gpu_driven_default", info);

			if (!gpu_driven_test)
			{
				std::cout << "Error When building material";
			}
			else
			{
				object_list[0].set_material(gpu_driven_test);
				object_list[1].set_material(gpu_driven_test);
			}
		}
	}
	else
	{
		Material* default_transparency = Singleton<DefaultSetting>::get_instance().material_system->get_material("default_transparency");
		if (!default_transparency)
		{

			std::string texture_path = "Resource/Texture/viking_room.png";
			VK_Texture* texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("viking_room", ENUM_TEXTURE_TYPE::ENUM_TYPE_2D, texture_path);
			MaterialData info;
			info.parameters = nullptr;
			info.textures.clear();
			SampledTexture tex;
			tex.view = texture->textureImageView;
			tex.sampler = texture->textureSampler;
			info.textures.push_back(tex);
			info.baseTemplate = Singleton<DefaultSetting>::get_instance().material_system->create_template_name("default_transparency");

			default_transparency = Singleton<DefaultSetting>::get_instance().material_system->build_material("default_transparency", info);

			if (!default_transparency)
			{
				std::cout << "Error When building material";
			}
			else
			{
				object_list[0].set_material(default_transparency);
				object_list[1].set_material(default_transparency);
			}
		}
	}

	Material* sdf_textMaterial = Singleton<DefaultSetting>::get_instance().material_system->get_material("sdf_text");
	{
		if (!sdf_textMaterial)
		{
			MaterialData info;
			info.parameters = nullptr;
			info.textures.clear();	
			info.baseTemplate="sdf_text";
			sdf_textMaterial = Singleton<DefaultSetting>::get_instance().material_system->build_material("sdf_text", info);

			if (!sdf_textMaterial)
			{
				std::cout << "Error When building material";
			}
			else
			{
				object_list[4].set_material(sdf_textMaterial);
			}
		}
	}
}

void MXRender::GameObjectManager::start_load_asset(GraphicsContext* context)
{
	TaskGraph& task_graph= Singleton<DefaultSetting>::get_instance().task_system->task_graph;

	task_graph.add_task_node(0,"Load_Obj",task_graph.get_task(&GameObjectManager::load_objs,this),{});
	task_graph.add_task_node(1, "Load_Prefabs", task_graph.get_task(&GameObjectManager::start_load_prefabs,this,context), {0});

	
	//start_load_prefabs(context);
}


const std::unordered_map<std::string, MXRender::TextBase*>& MXRender::GameObjectManager::get_text_cache() const
{
	return _text_ttfes;
}

const std::unordered_map<std::string, MXRender::MeshBase*>& MXRender::GameObjectManager::get_mesh_cache() const
{
	return _meshes;
}

const std::unordered_map<std::string, MXRender::CacheTexture >& MXRender::GameObjectManager::get_texture_cache() const
{
	return _loadedTextures;
}



const std::unordered_map<std::string, assets::PrefabInfo*>& MXRender::GameObjectManager::get_prefab_cache() const
{
	return _prefabCache;
}

bool MXRender::GameObjectManager::load_prefab(const std::string& name,const char* path, glm::mat4 root,GraphicsContext* context)
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

	vk_context->add_on_shutdown_clean_func([=]() {
		vkDestroySampler(vk_context->device->device,smoothSampler,nullptr);
		});

	std::unordered_map<uint64_t, glm::mat4> node_worldmats;

	std::vector<std::pair<uint64_t, glm::mat4>> pending_nodes;

	object_list.emplace_back(name);
	auto& object = object_list.back();
	object.get_transform()->set_model_matrix(root);
	PrefabObjectInfo prefab_objinfo;
	for (auto& [k, v] : prefab->node_names)
	{
		GameObject* obj=new GameObject(v);
		prefab_objinfo.node_objs[k]=obj;
		prefab_objinfo.node_objs_use[k] = false;
	}

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
			object.sub_objects.push_back(prefab_objinfo.node_objs[k]);
			prefab_objinfo.node_objs[k]->parent_object = &object;
			prefab_objinfo.node_objs[k]->get_transform()->set_model_matrix(node_worldmats[k]);

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
			GameObject* parent_obj= prefab_objinfo.node_objs[parent];
			GameObject* child_obj = prefab_objinfo.node_objs[node];
			//try to find parent in cache
			auto matrixIT = node_worldmats.find(parent);
			if (matrixIT != node_worldmats.end()) {

				//transform with the parent
				glm::mat4 nodematrix = (matrixIT)->second * pending_nodes[i].second;

				node_worldmats[node] = nodematrix;

				parent_obj->sub_objects.push_back(child_obj);
				child_obj->parent_object = parent_obj;
				child_obj->get_transform()->set_model_matrix(node_worldmats[node]);

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
			mesh->init_mesh_info(context);
		}


		auto materialName = v.material_path.c_str();
		//load material
		
		Material* objectMaterial = Singleton<DefaultSetting>::get_instance().material_system->get_material(materialName);
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
						info.baseTemplate = Singleton<DefaultSetting>::get_instance().material_system->create_template_name("mesh_base") ;
					}
					else {
						info.baseTemplate = Singleton<DefaultSetting>::get_instance().material_system->create_template_name("mesh_base");
					}

					info.textures.push_back(tex);

					objectMaterial = Singleton<DefaultSetting>::get_instance().material_system->build_material(materialName, info);

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
		prefab_objinfo.node_objs[k]->get_transform()->set_model_matrix(nodematrix);
		prefab_objinfo.node_objs[k]->get_staticmesh()->reset_mesh(get_mesh(v.mesh_path.c_str()));
		prefab_objinfo.node_objs[k]->set_material(objectMaterial);
		prefab_objinfo.node_objs_use[k] = true;
		prefab_renderables.push_back(loadmesh);
	}

	for (auto& [k, v] : prefab_objinfo.node_objs_use)
	{
		if (v==false)
		{
			if (prefab_objinfo.node_objs[k]->parent_object)
			{
				auto it = std::find(prefab_objinfo.node_objs[k]->parent_object->sub_objects.begin(), prefab_objinfo.node_objs[k]->parent_object->sub_objects.end(), prefab_objinfo.node_objs[k]);
				if ((it)!= prefab_objinfo.node_objs[k]->parent_object->sub_objects.end())
				{
					prefab_objinfo.node_objs[k]->parent_object->sub_objects.erase(it);
				}
			}
			delete prefab_objinfo.node_objs[k];
			prefab_objinfo.node_objs[k]=nullptr;
		}

	}

	return true;
}
