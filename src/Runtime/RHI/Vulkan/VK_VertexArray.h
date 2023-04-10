#pragma once
#ifndef _VK_VERTEXARRAY_
#define _VK_VERTEXARRAY_


#define GLFW_INCLUDE_VULKAN

#include <glm/glm.hpp>
#include"../RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include"../VertexArray.h"
#include <array>

#include <stdexcept>
#include <stddef.h>
#include "vulkan/vulkan_core.h"

namespace MXRender { struct VertexInputDescription; }

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
namespace MXRender
{
	struct SimpleVertex {

		glm::vec3 position;
		//glm::vec3 normal;
		glm::vec<2, uint8_t> oct_normal;//color;
		glm::vec<3, uint8_t> color;
		glm::vec2 uv;
		static VertexInputDescription get_vertex_description();

		void pack_normal(glm::vec3 n);
		void pack_color(glm::vec3 c);

		bool operator==(const SimpleVertex& other) const {
			return position == other.position && oct_normal == other.oct_normal&& color == other.color && uv == other.uv;
		}
	};


    class VK_VertexArray:public VertexArray
    {
    private:
        unsigned id;
        std::shared_ptr<VertexBuffer> layout;
        std::shared_ptr<IndexBuffer> ebo;
    public:
        VK_VertexArray(/* args */);
        virtual ~VK_VertexArray();

        void bind() const override;
        void unbind() const override;

        void set_vertexbuffer(const std::shared_ptr < VertexBuffer>& _vertex_buffer) override;
        void set_indexbuffer(const std::shared_ptr < IndexBuffer>& _index_buffer) override;

    };
    
}


template<> struct std::hash<MXRender::SimpleVertex>
{
	size_t operator()(MXRender::SimpleVertex const& vertex) const 
	{
		return std::hash<glm::vec3>()(vertex.position)  ^ (std::hash<glm::vec2>()(vertex.uv) << 1);
	};
};


#endif
