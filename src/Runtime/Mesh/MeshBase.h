#pragma once
#ifndef _MESHBASE_
#define _MESHBASE_
#include <vector>
#include <string>
#include "vulkan/vulkan_core.h"

namespace MXRender { class GraphicsContext; }


namespace MXRender
{
    struct SimpleVertex;
    struct MeshInfo
    {

    };

    struct GL_MeshInfo:public MeshInfo
    {
    public:
        unsigned int VAO, VBO, EBO;
    };

    struct VK_MeshInfo:public MeshInfo
    {
    public:
		VkBuffer vertex_buffer;
		VkDeviceMemory vertex_buffer_memory;
		VkBuffer index_buffer;
		VkDeviceMemory index_buffer_memory;
    };
    class MeshBase
    {
    private:
    protected:
        bool is_already_init=false;
    public:
        MeshBase();
        virtual ~MeshBase();
        std::vector<SimpleVertex> vertices;
        std::vector<uint32_t> indices;
        void load_model(const std::string& filename);
        virtual void destroy_mesh_info(GraphicsContext* context);
        virtual void init_mesh_info(GraphicsContext* context);
    };

}
#endif 