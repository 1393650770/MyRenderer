#include "RenderScene.h"
#include "../Logic/GameObject.h"
#include "../Mesh/MeshBase.h"
namespace MXRender
{

	RenderScene::RenderScene()
	{

	}

	RenderScene::~RenderScene()
	{

	}

	void RenderScene::register_render_object(GameObject* game_object)
	{
		RenderObject newObj;
		//newObj.bounds = object->bounds;
		newObj.transformMatrix = game_object->get_transform()->get_translation_matrix();
		newObj.material = get_material_id(game_object->material);
		newObj.meshID = get_mesh_id(game_object->get_staticmesh()->get_mesh_data().lock().get());
		newObj.updateIndex = (uint32_t)-1;
		newObj.customSortKey = object->customSortKey;
		newObj.passIndices.clear(-1);
		Handle<RenderObject> handle;
		handle.handle = static_cast<uint32_t>(renderables.size());

		renderables.push_back(newObj);

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
		return handle;
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

}