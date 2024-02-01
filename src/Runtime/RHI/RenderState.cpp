#include "RenderState.h"

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
			return nullptr;
			break;
		default:
			return nullptr;
			break;
		}
		return nullptr;
	}

}



