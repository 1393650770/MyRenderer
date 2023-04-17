#pragma once

#ifndef _RENDERSCENE_
#define _RENDERSCENE_
#include <memory>
#include <unordered_map>
#include "../Mesh/MeshBase.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"



struct GLFWwindow;
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
		Handle<Material> material;

		uint32_t updateIndex;
		uint32_t customSortKey{ 0 };

		PerPassData<int32_t> passIndices;

		glm::mat4 transformMatrix;

		RenderBounds bounds;
	};

	struct GPUIndirectObject {
		VkDrawIndexedIndirectCommand command;
		uint32_t objectID;
		uint32_t batchID;
	};


	class RenderScene
	{
	public:
		RenderScene();
		virtual ~RenderScene();
		void register_render_object(GameObject* game_object);
		void merge_object(VK_GraphicsContext* context);
		void create_indirect_drawcall(VK_GraphicsContext* context);
		void create_object_data_buffer(VK_GraphicsContext* context);
	protected:
		std::unordered_map<Material*, Handle<Material>> materialConvert;
		std::unordered_map<MeshBase*, Handle<DrawMesh>> meshConvert;
		std::vector<RenderObject> renderables;
		std::vector<DrawMesh> meshes;
		std::vector<Material*> materials;

		Handle<DrawMesh> get_mesh_id(MeshBase* mesh);
		Handle<Material> get_material_id(Material* material);
	private:

	};

}
#endif // !_MYRENDER_

