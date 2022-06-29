#include "VertexArray.h"
#include"OpenGL/GL_VertexArray.h"
#include"RenderState.h"

namespace MXRender
{
	VertexArray::~VertexArray()
	{
	}
	std::shared_ptr<VertexArray> VertexArray::CreateVertexArray()
	{
		switch (RenderState::render_api_type)
		{
		case ENUM_RENDER_API_TYPE::OpenGL:
			return std::make_shared<GL_VertexArray>();
			break;
		default:
			return nullptr;
			break;
		}
		return nullptr;
	}
}


