#include "RenderState.h"
#include "OpenGL/GL_State.h"
namespace MXRender
{

	RenderState::RenderState()
	{
	}

	RenderState::~RenderState()
	{
	}


	std::unique_ptr<RenderState> RenderState::CreateRenderState()
	{
		switch (RenderState::render_api_type)
		{
		case ENUM_RENDER_API_TYPE::OpenGL:
			return std::make_unique<GL_State>();
			break;
		default:
			return nullptr;
			break;
		}
		return nullptr;
	}

}



