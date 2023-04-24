#include "RenderScene.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Logic/GameObject.h"
#include "../Mesh/MeshBase.h"
#include "../Mesh/VK_Mesh.h"
#include "../RHI/Vulkan/VK_VertexArray.h"
#include "../RHI/Vulkan/VK_Utils.h"
#include "GPUDriven.h"
#include "optick.h"
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
		for (int i = 0; i < renderables.size(); i++) {


			target[dataIndex].command.firstInstance = i;//i;
			target[dataIndex].command.instanceCount = 0;
			target[dataIndex].command.firstIndex = 0;
			target[dataIndex].command.vertexOffset = 0;
			target[dataIndex].command.indexCount = get_mesh(renderables[i].meshID)->indexCount;
			target[dataIndex].objectID = i;
			target[dataIndex].batchID = i;

			dataIndex++;
		}
		OPTICK_POP()
	}

	void RenderScene::write_object_to_instance_buffer(GPUInstance* target)
	{
		OPTICK_PUSH("write_object_to_instance_buffer")
		int dataIndex = 0;
		for (int i = 0; i < renderables.size(); i++) {



			target[dataIndex].objectID = i;
			target[dataIndex].batchID = i;

			dataIndex++;
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
		if (game_object->get_component(StaticMeshComponent))
		{

			RenderObject newObj;
			newObj.bounds = game_object->get_staticmesh()->get_mesh_data().lock()->bounds;
			newObj.transformMatrix = game_object->get_transform()->get_model_matrix();
			newObj.materialID = get_material_id(game_object->get_material());
			newObj.meshID = get_mesh_id(game_object->get_staticmesh()->get_mesh_data().lock().get());
			newObj.updateIndex = (uint32_t)-1;
			//newObj.customSortKey = object->customSortKey;
			newObj.passIndices.clear(-1);
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