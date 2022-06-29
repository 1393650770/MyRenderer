#include "VertexBuffer.h"
#include"OpenGL/GL_VertexBuffer.h"
#include"RenderState.h"
namespace MXRender
{
	VertexBuffer::~VertexBuffer()
	{
	}

	Layout::Layout(const std::initializer_list<Layout_Element>& elements):layout(elements)
	{
		stride = 0;
		for (auto& element : layout)
		{
			offset[element.attributr_type] = stride;
			stride += element.num;
			
		}
		
	}


	Layout::~Layout()
	{
	}
	const unsigned& Layout::get_stride() const
	{
		return stride;
	}
	const unsigned int& Layout::get_offset(ENUM_RENDER_ATTRIBUTE_TYPE::Enum attribute_type) const
	{
		return offset[attribute_type];
	}
	std::shared_ptr<VertexBuffer> VertexBuffer::CreateVertexBuffer(const void* vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE usage)
	{
		switch (RenderState::render_api_type)
		{
		case ENUM_RENDER_API_TYPE::OpenGL:
			return std::make_shared<GL_VertexBuffer>(vertices, size, usage);
			break;
		default:
			return nullptr;
			break;
		}
		return nullptr;
	}
	std::shared_ptr<VertexBuffer> VertexBuffer::CreateVertexBuffer(std::shared_ptr<std::vector<float>> vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE usage)
	{
		switch (RenderState::render_api_type)
		{
		case ENUM_RENDER_API_TYPE::OpenGL:
			return std::make_shared<GL_VertexBuffer>(vertices, size, usage);
			break;
		default:
			return nullptr;
			break;
		}
		return nullptr;
	}
}

