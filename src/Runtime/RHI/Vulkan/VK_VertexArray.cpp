
#include "VK_VertexArray.h"
#include"../VertexBuffer.h"
#include"../IndexBuffer.h"
#include"VK_Utils.h"
#include"../RenderUtils.h"
#include "VK_RenderPass.h"
#include <stddef.h>
using namespace glm;
namespace MXRender
{

    VK_VertexArray::VK_VertexArray()
    {
        glGenVertexArrays(1, &id);
    }

    VK_VertexArray::~VK_VertexArray()
    {
        glDeleteVertexArrays(1, &id);
    }

    void VK_VertexArray::bind() const
    {
        glBindVertexArray(id);
    }

    void VK_VertexArray::unbind() const
    {
        glBindVertexArray(0);
    }

    void VK_VertexArray::set_vertexbuffer(const std::shared_ptr<VertexBuffer>& _vertex_buffer)
    {
        layout = _vertex_buffer;
        glBindVertexArray(id);
        
        auto& opengl_layout = layout->get_layout();
        unsigned index = 0;

       

    }

    void VK_VertexArray::set_indexbuffer(const std::shared_ptr<IndexBuffer>& _index_buffer)
    {

    }



	MXRender::VertexInputDescription AssetVertex::get_vertex_description()
	{
		VertexInputDescription description;

		VkVertexInputBindingDescription mainBinding = {};
		mainBinding.binding = 0;
		mainBinding.stride = sizeof(AssetVertex);
		mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		description.bindings.push_back(mainBinding);
		//Position will be stored at Location 1
		VkVertexInputAttributeDescription positionAttribute = {};
		positionAttribute.binding = 0;
		positionAttribute.location = 0;
		positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		positionAttribute.offset = offsetof(AssetVertex, position);

		//Normal will be stored at Location 1
		VkVertexInputAttributeDescription normalAttribute = {};
		normalAttribute.binding = 0;
		normalAttribute.location = 1;
		normalAttribute.format = VK_FORMAT_R8G8_UNORM;
		normalAttribute.offset = offsetof(AssetVertex, oct_normal);

		//Position will be stored at Location 2
		VkVertexInputAttributeDescription colorAttribute = {};
		colorAttribute.binding = 0;
		colorAttribute.location = 2;
		colorAttribute.format = VK_FORMAT_R8G8B8_UNORM;
		colorAttribute.offset = offsetof(AssetVertex, color);

		//UV will be stored at Location 3
		VkVertexInputAttributeDescription uvAttribute = {};
		uvAttribute.binding = 0;
		uvAttribute.location = 3;
		uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
		uvAttribute.offset = offsetof(AssetVertex, uv);


		description.attributes.push_back(positionAttribute);
		description.attributes.push_back(normalAttribute);
		description.attributes.push_back(colorAttribute);
		description.attributes.push_back(uvAttribute);
		return description;
	}
	vec2 OctNormalWrap(vec2 v)
	{
		vec2 wrap;
		wrap.x = (1.0f - glm::abs(v.y)) * (v.x >= 0.0f ? 1.0f : -1.0f);
		wrap.y = (1.0f - glm::abs(v.x)) * (v.y >= 0.0f ? 1.0f : -1.0f);
		return wrap;
	}


	vec2 OctNormalEncode(vec3 n)
	{
		n /= (glm::abs(n.x) + glm::abs(n.y) + glm::abs(n.z));

		vec2 wrapped = OctNormalWrap(n);

		vec2 result;
		result.x = n.z >= 0.0f ? n.x : wrapped.x;
		result.y = n.z >= 0.0f ? n.y : wrapped.y;

		result.x = result.x * 0.5f + 0.5f;
		result.y = result.y * 0.5f + 0.5f;

		return result;
	}

	void AssetVertex::pack_normal(glm::vec3 n)
	{
		vec2 oct = OctNormalEncode(n);

		oct_normal.x = uint8_t(oct.x * 255);
		oct_normal.y = uint8_t(oct.y * 255);
	}

	void AssetVertex::pack_color(glm::vec3 c)
	{
		color.r = static_cast<uint8_t>(c.x * 255);
		color.g = static_cast<uint8_t>(c.y * 255);
		color.b = static_cast<uint8_t>(c.z * 255);
	}

}