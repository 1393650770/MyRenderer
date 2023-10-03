#pragma once
#ifndef _MESHBASE_
#define _MESHBASE_
#include <vector>
#include <string>
#include "vulkan/vulkan_core.h"
#include "glm/gtx/quaternion.hpp"
#include "../RHI/Vulkan/VK_VertexArray.h"

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
		VkBuffer vertex_buffer=VK_NULL_HANDLE;
		VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;
		VkBuffer index_buffer = VK_NULL_HANDLE;
		VkDeviceMemory index_buffer_memory = VK_NULL_HANDLE;
    };

	struct RenderBounds {
		glm::vec3 origin;
		float radius;
		glm::vec3 extents;
		bool valid;
	};
    class MeshBase
    {
    private:
    protected:
        bool is_already_init=false;
        bool is_prefabs=false;
    public:
        MeshBase();
        virtual ~MeshBase();
        std::vector<SimpleVertex> vertices;
        std::vector<uint32_t> indices;
        RenderBounds bounds;
        void load_model(const std::string& filename);
        bool load_asset(const  char* filename);
        virtual void destroy_mesh_info(GraphicsContext* context);
        virtual void init_mesh_info(GraphicsContext* context);
        virtual void bind(GraphicsContext* context);
        virtual void render(GraphicsContext* context);
    };

}
#endif 
