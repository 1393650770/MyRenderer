#pragma once
#ifndef _STATICMESHCOMPONENT_
#define _STATICMESHCOMPONENT_
#include <vector>
#include <string>
#include "ComponentBase.h"
#include <glm/glm.hpp>
#include <memory>
#include "vulkan/vulkan_core.h"

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
		VkBuffer vertex_buffer;
		VkBuffer index_buffer;
	};

	struct BindMeshInfo
	{
	public:
		GraphicsContext* context;

	};

	struct VK_BindMeshInfo :BindMeshInfo
	{
	public:

		VkBuffer vertex_buffer;
		VkBuffer index_buffer;
		VkDeviceMemory vertex_buffer_memory;
		VkDeviceMemory index_buffer_memory;
	};



	class StaticMeshComponent :public ComponentBase
	{
	private:
	protected:
		std::shared_ptr< MeshBase> mesh_data;

		void setup_vk_vertexbuffer(VK_GraphicsContext* cur_context, VkBuffer vertex_buffer,VkDeviceMemory vertex_buffer_memory);
		void setup_vk_indexbuffer(VK_GraphicsContext* cur_context, VkBuffer index_buffer, VkDeviceMemory index_buffer_memory);

	public:
		StaticMeshComponent()=delete;
		StaticMeshComponent(const std::string& mesh_path);
		virtual ~StaticMeshComponent();
		std::weak_ptr<MeshBase> get_mesh_data();

		void render_mesh(RenderMeshInfo* render_mesh_info);

		void bind_mesh(BindMeshInfo* bind_mesh_info);

		virtual void on_start() override;


		virtual void on_update() override;


		virtual void update(float delta_time) override;


		virtual void on_end() override;


		virtual void on_destroy() override;

	};

}
#endif 
