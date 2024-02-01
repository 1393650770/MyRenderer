#pragma once
#ifndef _STATICMESHCOMPONENT_
#define _STATICMESHCOMPONENT_
#include <vector>
#include <string>
#include "ComponentBase.h"
#include <glm/glm.hpp>
#include <memory>
#include "vulkan/vulkan_core.h"

namespace MXRender { class Material; }

namespace MXRender { class VK_GraphicsContext; }

namespace MXRender { class GraphicsContext; }

namespace MXRender { class MeshBase; }

namespace MXRender
{
	struct RenderMeshInfo
	{
	public:
		GraphicsContext* context;

	};

	struct VK_RenderMeshInfo:RenderMeshInfo
	{
	public:
		VK_RenderMeshInfo(VkBuffer& new_vertex_buffer, VkBuffer& new_index_buffer) :
			vertex_buffer(new_vertex_buffer), index_buffer(new_index_buffer)
		{
		};
		VkBuffer& vertex_buffer;
		VkBuffer& index_buffer;
	};

	struct BindMeshInfo
	{
	public:
		GraphicsContext* context;

	};

	struct VK_BindMeshInfo :BindMeshInfo
	{
	public:
		//VK_BindMeshInfo(){};
		VK_BindMeshInfo(VkBuffer& new_vertex_buffer, VkBuffer& new_index_buffer, VkDeviceMemory& new_vertex_buffer_memory, VkDeviceMemory& new_index_buffer_memory):
			vertex_buffer(new_vertex_buffer),index_buffer(new_index_buffer),vertex_buffer_memory(new_vertex_buffer_memory),index_buffer_memory(new_index_buffer_memory)
			{
			};
		VkBuffer& vertex_buffer;
		VkBuffer& index_buffer;
		VkDeviceMemory& vertex_buffer_memory;
		VkDeviceMemory& index_buffer_memory;
	};



	class StaticMeshComponent :public ComponentBase
	{
	private:
		void spawn_mesh_data_shared_ptr();
	protected:
		MeshBase* mesh_data;
		Material* material=nullptr;
		bool is_already_load_mesh=false;

	public:
		StaticMeshComponent();
		StaticMeshComponent(const std::string& mesh_path);
		virtual ~StaticMeshComponent();

		void load_mesh(const std::string& mesh_path);
		void reset_mesh(MeshBase* mesh);
		void set_overmaterial(Material* material);
		Material* get_material() const;
		MeshBase* get_mesh_data();
		bool get_already_load_mesh() const;
		void set_already_load_mesh_to_true();

		void render_mesh(RenderMeshInfo* render_mesh_info);
		void bind_mesh(BindMeshInfo* bind_mesh_info);

		virtual void on_start() override;


		virtual void on_update() override;


		virtual void update(float delta_time) override;


		virtual void on_end() override;


		virtual void on_destroy() override;


		virtual std::string get_component_type_name() override;

	};

}
#endif 
