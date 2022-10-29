#pragma once
#ifndef _VK_MESH_
#define _VK_MESH_
#include <vector>
#include <string>
#include "MeshBase.h"
#include "vulkan/vulkan_core.h"

namespace MXRender { class VK_GraphicsContext; }

namespace MXRender { class GraphicsContext; }


namespace MXRender
{

    class VK_Mesh:public MeshBase
    {
    private:
    protected:
        VK_MeshInfo vk_meshinfo;
		void setup_vk_vertexbuffer(VK_GraphicsContext* cur_context, VkBuffer& vertex_buffer, VkDeviceMemory& vertex_buffer_memory);
		void setup_vk_indexbuffer(VK_GraphicsContext* cur_context, VkBuffer& index_buffer, VkDeviceMemory& index_buffer_memory);

    public:
        VK_Mesh();
        virtual ~VK_Mesh();

        virtual void destroy_mesh_info(GraphicsContext* context) override final;
        virtual void init_mesh_info(GraphicsContext* context) override final;

        VK_MeshInfo get_mesh_info() const;
    };

}
#endif 
