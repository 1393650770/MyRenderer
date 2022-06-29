#include "IndexBuffer.h"
#include"OpenGL/GL_IndexBuffer.h"
#include"RenderState.h"
namespace MXRender
{
	std::shared_ptr<IndexBuffer> IndexBuffer::CreateIndexBuffer(const void* _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
	{
		switch (RenderState::render_api_type)
		{
		case ENUM_RENDER_API_TYPE::OpenGL:
			return std::make_shared<GL_IndexBuffer>(_index_array, size, data_usage);
			break;
		default:
			return nullptr;
			break;
		}
		return nullptr;
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::CreateIndexBuffer(std::shared_ptr<std::vector<unsigned int>> _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
	{
		switch (RenderState::render_api_type)
		{
		case ENUM_RENDER_API_TYPE::OpenGL:
			return std::make_shared<GL_IndexBuffer>(_index_array, size, data_usage);
			break;
		default:
			return nullptr;
			break;
		}
		return nullptr;
	}

}

