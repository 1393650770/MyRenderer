#include "Shader.h"
#include"OpenGL/GL_Shader.h"
#include"RenderState.h"

namespace MXRender
{


	Shader::~Shader()
	{
	}




    std::shared_ptr<Shader> Shader::CreateShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const char* computePath)
    {
        switch (RenderState::render_api_type)
        {
        case ENUM_RENDER_API_TYPE::OpenGL:
            return std::make_shared<GL_Shader>(vertexPath, fragmentPath, geometryPath, computePath);
            break;
        default:
            return nullptr;
            break;
        }
        return nullptr;
    }

}


