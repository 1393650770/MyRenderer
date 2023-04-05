#pragma once

#ifndef _RENDERSCENE_
#define _RENDERSCENE_
#include <memory>
#include <unordered_map>
#include "../Mesh/MeshBase.h"


struct GLFWwindow;
namespace MXRender { class MeshBase; }
namespace MXRender { class Material; }
namespace MXRender { class GameObject; }
namespace MXRender { class WindowUI; }

namespace MXRender { class VK_GraphicsContext; }

namespace MXRender
{
	enum MeshpassType : int
	{
		Forward=0,
		Transparency,
		DirectionalShadow,
		Count
	};

	template<typename T>
	struct Handle {
		uint32_t handle;
	};

	template<typename T>
	struct PerPassData {

	public:
		T& operator[](MeshpassType pass)
		{
			
			return data[pass];
			//assert(false);
			//return data[0];
		};

		void clear(T&& val)
		{
			for (int i = 0; i < MeshpassType::Count; i++)
			{
				data[i] = val;
			}
		}

	private:
		T data[MeshpassType::Count];
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
	class RenderScene
	{
	public:
		RenderScene();
		virtual ~RenderScene();
		void register_render_object(GameObject* game_object);
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

