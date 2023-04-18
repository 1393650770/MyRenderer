#include "RenderScene.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Logic/GameObject.h"
#include "../Mesh/MeshBase.h"
#include "../Mesh/VK_Mesh.h"
#include "../RHI/Vulkan/VK_VertexArray.h"
#include "../RHI/Vulkan/VK_Utils.h"
#include "GPUDriven.h"
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
		newObj.material = get_material_id(game_object->get_material());
		newObj.meshID = get_mesh_id(game_object->get_staticmesh()->get_mesh_data().lock().get());
		newObj.updateIndex = (uint32_t)-1;
		//newObj.customSortKey = object->customSortKey;
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
		
	}

	//void RenderScene::merge_object(VK_GraphicsContext* context)
	//{
	//	size_t total_vertices = 0;
	//	size_t total_indices = 0;

	//	for (auto& m : meshes)
	//	{
	//		m.firstIndex = static_cast<uint32_t>(total_indices);
	//		m.firstVertex = static_cast<uint32_t>(total_vertices);

	//		total_vertices += m.vertexCount;
	//		total_indices += m.indexCount;

	//		m.isMerged = true;
	//	}

	//	merged_vertexBuffer = VK_Utils::Create_buffer(context,total_vertices * sizeof(SimpleVertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	//		VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//	merged_indexBuffer = VK_Utils::Create_buffer(context, total_indices * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	//		VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//	VK_Utils::Immediate_Submit(context,[&](VkCommandBuffer cmd)
	//		{
	//			for (auto& m : meshes)
	//			{
	//				VkBufferCopy vertexCopy;
	//				vertexCopy.dstOffset = m.firstVertex * sizeof(SimpleVertex);
	//				vertexCopy.size = m.vertexCount * sizeof(SimpleVertex);
	//				vertexCopy.srcOffset = 0;
	//				VK_Mesh* vk_mesh = dynamic_cast<VK_Mesh*>(m.original);
	//				if (!context || !vk_mesh) 
	//					continue;
	//				vkCmdCopyBuffer(cmd, vk_mesh->get_mesh_info().vertex_buffer, merged_vertexBuffer._buffer, 1, &vertexCopy);

	//				VkBufferCopy indexCopy;
	//				indexCopy.dstOffset = m.firstIndex * sizeof(uint32_t);
	//				indexCopy.size = m.indexCount * sizeof(uint32_t);
	//				indexCopy.srcOffset = 0;

	//				vkCmdCopyBuffer(cmd, vk_mesh->get_mesh_info().index_buffer, merged_indexBuffer._buffer, 1, &indexCopy);
	//			}
	//		});
	//}

	//void RenderScene::create_indirect_drawcall(VK_GraphicsContext* context)
	//{
	//	indirect_drawcallBuffer = VK_Utils::Create_buffer(context, sizeof(GPUIndirectObject) * meshes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//	GPUIndirectObject* indirect ;
	//	vmaMapMemory(context->_allocator, indirect_drawcallBuffer._allocation, (void**)&indirect);
	//	int dataIndex = 0;
	//	for (auto& it: meshes)
	//	{

	//		indirect[dataIndex].command.firstInstance = 1;//i;
	//		indirect[dataIndex].command.instanceCount = 0;
	//		indirect[dataIndex].command.firstIndex = it.firstIndex;
	//		indirect[dataIndex].command.vertexOffset = it.firstVertex;
	//		indirect[dataIndex].command.indexCount = it.indexCount;
	//		indirect[dataIndex].objectID = 0;
	//		indirect[dataIndex].batchID = 0;

	//		dataIndex++;
	//	}
	//	vmaUnmapMemory(context->_allocator, indirect_drawcallBuffer._allocation);
	//}

	//void RenderScene::create_object_data_buffer(VK_GraphicsContext* context)
	//{	
	//	object_data_Buffer = VK_Utils::Create_buffer(context, sizeof(GPUIndirectObject) * meshes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//	GPUObjectData* object_data;
	//	vmaMapMemory(context->_allocator, indirect_drawcallBuffer._allocation, (void**)&object_data);
	//	int dataIndex = 0;
	//	for (auto& it : meshes)
	//	{

	//		

	//		dataIndex++;
	//	}
	//	vmaUnmapMemory(context->_allocator, indirect_drawcallBuffer._allocation);
	//}

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