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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
namespace MXRender
{
	struct SimpleVertex {
		glm::vec3  pos;
		//glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() 
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(SimpleVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = 0;

			//attributeDescriptions[1].binding = 0;
			//attributeDescriptions[1].location = 1;
			//attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			//attributeDescriptions[1].offset = sizeof(glm::vec2);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[1].offset = sizeof(glm::vec3);

			return attributeDescriptions;
		}

		bool operator==(const SimpleVertex& other) const {
			return pos== other.pos &&  texCoord == other.texCoord;
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
		return std::hash<glm::vec3>()(vertex.pos)  ^ (std::hash<glm::vec2>()(vertex.texCoord) << 1);
	};
};


#endif
