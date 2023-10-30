#include "RenderScene.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Logic/GameObject.h"
#include "../Mesh/MeshBase.h"
#include "../Mesh/VK_Mesh.h"
#include "../RHI/Vulkan/VK_VertexArray.h"
#include "../RHI/Vulkan/VK_Utils.h"
#include "GPUDriven.h"
#include "optick.h"
#include "DefaultSetting.h"
#include "../Utils/Singleton.h"
namespace MXRender
{

	RenderScene::RenderScene()
	{
		gpu_driven=new GPUDrivenSystem();
	}

	RenderScene::~RenderScene()
	{

	}

	void RenderScene::register_render_object(GameObject* game_object)
	{
		OPTICK_EVENT()

		OPTICK_PUSH("register_render_object")
		register_objects(game_object);
		
		OPTICK_POP()
		//if (object->bDrawForwardPass)
		//{
		//	if (object->material->original->passShaders[MeshpassType::Transparency])
		//	{
		//		_transparentForwardPass.unbatchedObjects.push_back(handle);
		//	}
		//	if (object->material->original->passShaders[MeshpassType::Forward])
		//	{
		//		_forwardPass.unbatchedObjects.push_back(handle);
		//	}
		//}
		//if (object->bDrawShadowPass)
		//{
		//	if (object->material->original->passShaders[MeshpassType::DirectionalShadow])
		//	{
		//		_shadowPass.unbatchedObjects.push_back(handle);
		//	}
		//}
		//update_object(handle);
		
	}

	void RenderScene::update_object_transform(GameObject* game_object)
	{
		OPTICK_EVENT()

		OPTICK_PUSH("update_object_transform")
		if (renderObjectConvert.find(game_object)!=renderObjectConvert.end())
		{
			RenderObject* render_object= get_render_object(renderObjectConvert[game_object]);
			
			render_object->transformMatrix= game_object->get_transform()->get_model_matrix();
		}
		OPTICK_POP()
	}

	MXRender::RenderObject* RenderScene::get_render_object(Handle<RenderObject> objectID)
	{
		return &renderables[objectID.handle];
	}

	MXRender::Material* RenderScene::get_material(Handle<Material> materialID)
	{
		return materials[materialID.handle];
	}

	MXRender::DrawMesh* RenderScene::get_mesh(Handle<DrawMesh> meshID)
	{
		return &(meshes[meshID.handle]);
	}

	void RenderScene::merger_mesh()
	{
		VK_GraphicsContext* context = Singleton<DefaultSetting>::get_instance().context.get();
		if (context==nullptr)
		{
			return;
		}
		uint32_t total_vertex=0,total_index=0;
		for (auto& m:meshes)
		{
			m.firstIndex = (total_index);
			m.firstVertex = (total_vertex);

			total_vertex += m.vertexCount;
			total_index += m.indexCount;

			m.isMerged = true;
		}

		if (merged_vertex_buffer._buffer==VK_NULL_HANDLE)
		{
			merged_vertex_buffer=VK_Utils::Create_buffer(context, total_vertex * sizeof(SimpleVertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			merged_index_buffer = VK_Utils::Create_buffer(context, total_index * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}

		VK_Utils::Immediate_Submit(context,[&](VkCommandBuffer cmd)
			{
				for (auto& m : meshes)
				{
					VK_Mesh* vk_mesh=dynamic_cast<VK_Mesh*>(m.original);
					if (vk_mesh==nullptr)
					{
						return;
					}
					vk_mesh->init_mesh_info(context);

					VkBufferCopy vertexCopy;
					vertexCopy.dstOffset = m.firstVertex * sizeof(SimpleVertex);
					vertexCopy.size = m.vertexCount * sizeof(SimpleVertex);
					vertexCopy.srcOffset = 0;

					vkCmdCopyBuffer(cmd, vk_mesh->get_mesh_info().vertex_buffer, merged_vertex_buffer._buffer, 1, &vertexCopy);

					VkBufferCopy indexCopy;
					indexCopy.dstOffset = m.firstIndex * sizeof(uint32_t);
					indexCopy.size = m.indexCount * sizeof(uint32_t);
					indexCopy.srcOffset = 0;

					vkCmdCopyBuffer(cmd, vk_mesh->get_mesh_info().index_buffer, merged_index_buffer._buffer, 1, &indexCopy);
				}
			});
	}

	void RenderScene::merger_renderobj()
	{
		uint32_t i=0;
		for (auto& obj : renderables)
		{
			merge_batch[obj.merge_key].push_back(Handle<RenderObject>(i));
			i++;
		}
	}

	void RenderScene::update_object(Handle<RenderObject> objectID)
	{
		if (get_render_object(objectID)->updateIndex == (uint32_t)-1)
		{

			get_render_object(objectID)->updateIndex = static_cast<uint32_t>(dirtyObjects.size());

			dirtyObjects.push_back(objectID);
		}
	}

	void RenderScene::clear_dirty_objects()
	{
		for (auto obj : dirtyObjects)
		{
			get_render_object(obj)->updateIndex = (uint32_t)-1;
		}
		dirtyObjects.clear();
	}

	void RenderScene::write_object_to_gpudata_buffer(GPUObjectData* target, Handle<RenderObject> objectID)
	{
		RenderObject* renderable = get_render_object(objectID);
		GPUObjectData object;

		object.modelMatrix = renderable->transformMatrix;
		object.origin_rad = glm::vec4(renderable->bounds.origin, renderable->bounds.radius);
		object.extents = glm::vec4(renderable->bounds.extents, renderable->bounds.valid ? 1.f : 0.f);

		memcpy(target, &object, sizeof(GPUObjectData));
	}

	void RenderScene::write_object_to_indirectcommand_buffer(GPUIndirectObject* target)
	{
		OPTICK_PUSH("write_object_to_indirectcommand_buffer")
		int dataIndex = 0;
		if (Singleton<DefaultSetting>::get_instance().is_enable_batch==false)
		{
			for (int i = 0; i < renderables.size(); i++) {
				target[dataIndex].command.firstInstance = i;//i;
				target[dataIndex].command.instanceCount = 0;
				target[dataIndex].command.firstIndex = get_mesh(renderables[i].meshID)->firstIndex;
				target[dataIndex].command.vertexOffset = get_mesh(renderables[i].meshID)->firstVertex;
				target[dataIndex].command.indexCount = get_mesh(renderables[i].meshID)->indexCount;
				target[dataIndex].objectID = i;
				target[dataIndex].batchID = i;
				dataIndex++;
			}
		}
		else
		{ 
			uint32_t firstInstanceID = 0;
			for (auto& [k,v]:merge_batch)
			{
				auto& i =v[0];
				target[dataIndex].command.firstInstance = firstInstanceID;//i;
				target[dataIndex].command.instanceCount = 0;
				target[dataIndex].command.firstIndex = get_mesh(get_render_object(i)->meshID)->firstIndex;
				target[dataIndex].command.vertexOffset = get_mesh(get_render_object(i)->meshID)->firstVertex;
				target[dataIndex].command.indexCount = get_mesh(get_render_object(i)->meshID)->indexCount;
				target[dataIndex].objectID = i.handle;
				target[dataIndex].batchID = dataIndex;
				dataIndex++;
				firstInstanceID+=v.size();
			}
		}
		OPTICK_POP()
	}

	void RenderScene::write_object_to_instance_buffer(GPUInstance* target)
	{
		OPTICK_PUSH("write_object_to_instance_buffer")
		int dataIndex = 0;
		if (Singleton<DefaultSetting>::get_instance().is_enable_batch==false)
		{
			for (int i = 0; i < renderables.size(); i++) {
				target[dataIndex].objectID = i;
				target[dataIndex].batchID = i;
				dataIndex++;
			}
		}
		else
		{ 
			int batchIndex = 0;
			for (auto& [k, v] : merge_batch)
			{
				for (auto& i : v)
				{
					target[dataIndex].objectID = i.handle;
					target[dataIndex].batchID = batchIndex;
					dataIndex++;
				}
				batchIndex++;
			}
		}
		OPTICK_POP()
	}

	std::vector<MXRender::Handle<MXRender::RenderObject>>& RenderScene::get_dirty_objects()
	{
		return dirtyObjects;
	}

	const MXRender::RenderObject& RenderScene::get_renderable_obj(int index) const
	{
		return renderables[index];
	}

	int RenderScene::get_renderables_size() const
	{
		return renderables.size();
	}

	
	void RenderScene::destroy()
	{
		if (Singleton<DefaultSetting>::get_instance().is_enable_gpu_driven)
		{
			gpu_driven->destroy();
		}
		delete gpu_driven;
		VK_GraphicsContext* context = Singleton<DefaultSetting>::get_instance().context.get();
		VK_Utils::Destroy_Buffer(context, merged_index_buffer);
		VK_Utils::Destroy_Buffer(context, merged_vertex_buffer);

	}

	Handle<DrawMesh> RenderScene::get_mesh_id(MeshBase* mesh)
	{
		Handle<DrawMesh> handle;
		auto it = meshConvert.find(mesh);
		if (it == meshConvert.end())
		{
			uint32_t index = static_cast<uint32_t>(meshes.size());

			DrawMesh newMesh;
			newMesh.original = mesh;
			newMesh.firstIndex = 0;
			newMesh.firstVertex = 0;
			newMesh.vertexCount = static_cast<uint32_t>(mesh->vertices.size());
			newMesh.indexCount = static_cast<uint32_t>(mesh->indices.size());

			meshes.push_back(newMesh);

			handle.handle = index;
			meshConvert[mesh] = handle;
		}
		else {
			handle = (*it).second;
		}
		return handle;
	}

	MXRender::Handle<MXRender::Material> RenderScene::get_material_id(Material* material)
	{
		Handle<Material> handle;
		auto it = materialConvert.find(material);
		if (it == materialConvert.end())
		{
			uint32_t index = static_cast<uint32_t>(materials.size());
			materials.push_back(material);

			handle.handle = index;
			materialConvert[material] = handle;
		}
		else 
		{
			handle = (*it).second;
		}
		return handle;
	}

	void RenderScene::register_objects(GameObject* game_object)
	{
		if (game_object->get_component(StaticMeshComponent)&& game_object->get_component(StaticMeshComponent)->get_material()!=nullptr)
		{

			RenderObject newObj;
			newObj.bounds = game_object->get_staticmesh()->get_mesh_data()->bounds;
			newObj.transformMatrix = game_object->get_transform()->get_model_matrix();
			newObj.materialID = get_material_id(game_object->get_material());
			newObj.meshID = get_mesh_id(game_object->get_staticmesh()->get_mesh_data());
			newObj.updateIndex = (uint32_t)-1;
			//newObj.customSortKey = object->customSortKey;
			newObj.passIndices.clear(-1);
			
			uint64_t pipelinehash = std::hash<uint64_t>()(uint64_t(game_object->get_material()->pass_pso->pass_pso[MeshpassType::Forward]->pipeline));
			uint64_t sethash = std::hash<uint64_t>()((uint64_t)game_object->get_material()->pass_sets[MeshpassType::Forward]);

			uint32_t mathash = static_cast<uint32_t>(pipelinehash ^ sethash);

			newObj.merge_key = mathash;
			
			Handle<RenderObject> handle;
			handle.handle = static_cast<uint32_t>(renderables.size());


			renderables.push_back(newObj);

			renderObjectConvert[game_object] = handle;

			update_object(handle);
		}		
		for (int i=0;i< game_object->sub_objects.size();i++)
		{
			if (game_object->sub_objects[i])
			{
				register_objects(game_object->sub_objects[i]);
			}

		}
	}

}