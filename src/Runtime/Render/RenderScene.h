#pragma once

#ifndef _RENDERSCENE_
#define _RENDERSCENE_
#include <memory>
#include <unordered_map>
#include "../Mesh/MeshBase.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"



struct GLFWwindow;
namespace MXRender { struct GPUInstance; }
namespace MXRender { struct GPUIndirectObject; }
namespace MXRender { class GPUDrivenSystem; }
namespace MXRender { struct GPUObjectData; }
namespace MXRender { struct GPUObjectData; }
namespace MXRender { struct GPUIndirectObject; }
namespace MXRender { struct GPUIndirectObject; }
namespace MXRender { struct SimpleVertex; }
namespace MXRender { class MeshBase; }
namespace MXRender { class Material; }
namespace MXRender { class GameObject; }
namespace MXRender { class WindowUI; }
namespace MXRender { class RenderBounds ;}
namespace MXRender { class VK_GraphicsContext; }

namespace MXRender
{
	template<typename T>
	struct Handle {
		uint32_t handle;
	};

	struct MeshObject {
		MeshBase* mesh{ nullptr };

		Material* material;
		uint32_t customSortKey;
		glm::mat4 transformMatrix;

		RenderBounds bounds;

		uint32_t bDrawForwardPass : 1;
		uint32_t bDrawShadowPass : 1;
	};

	struct DrawMesh {
		uint32_t firstVertex;
		uint32_t firstIndex;
		uint32_t indexCount;
		uint32_t vertexCount;
		bool isMerged;

		MeshBase* original;
	};
	struct RenderObject
	{
		Handle<DrawMesh> meshID;
		Handle<Material> materialID;

		uint32_t updateIndex;
		uint32_t customSortKey{ 0 };
		uint32_t merge_key{0};
		PerPassData<int32_t> passIndices;

		glm::mat4 transformMatrix;

		RenderBounds bounds;
	};

	//struct GPUIndirectObject {
	//	VkDrawIndexedIndirectCommand command;
	//	uint32_t objectID;
	//	uint32_t batchID;
	//};


	class RenderScene
	{
	public:
		GPUDrivenSystem* gpu_driven;

		AllocatedBuffer<SimpleVertex> merged_vertex_buffer;
		AllocatedBuffer<uint32_t> merged_index_buffer;
		std::unordered_map<uint32_t, std::vector<Handle<RenderObject>>> merge_batch;
		RenderScene();
		virtual ~RenderScene();
		void register_render_object(GameObject* game_object);
		void update_object_transform(GameObject* game_object);
		RenderObject* get_render_object(Handle<RenderObject> objectID);
		Material* get_material(Handle<Material> materialID);
		DrawMesh* get_mesh(Handle<DrawMesh> meshID);
		void merger_mesh();
		void merger_renderobj();
		void update_object(Handle<RenderObject> objectID);
		void clear_dirty_objects();
		void write_object_to_gpudata_buffer(GPUObjectData* target, Handle<RenderObject> objectID);
		void write_object_to_indirectcommand_buffer(GPUIndirectObject* target);
		void write_object_to_instance_buffer(GPUInstance* target);
		std::vector<Handle<RenderObject>>& get_dirty_objects();
		const RenderObject& get_renderable_obj(int index) const;
		int get_renderables_size() const;

	protected:
		std::unordered_map<GameObject* , Handle<RenderObject>> renderObjectConvert;
		std::unordered_map<Material*, Handle<Material>> materialConvert;
		std::unordered_map<MeshBase*, Handle<DrawMesh>> meshConvert;

		std::vector<RenderObject> renderables;
		std::vector<DrawMesh> meshes;
		std::vector<Material*> materials;
		std::vector<Handle<RenderObject>> dirtyObjects;
		Handle<DrawMesh> get_mesh_id(MeshBase* mesh);
		Handle<Material> get_material_id(Material* material);


	private:
		void register_objects(GameObject* game_object);
	};

}
#endif // !_MYRENDER_

