#include "Shader.h"
#include"OpenGL/GL_Shader.h"
#include"RenderState.h"

namespace MXRender
{


	Shader::~Shader()
	{
	}


	std::shared_ptr<Shader> Shader::CreateShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const const std::string& computePath)
	{
        switch (RenderState::render_api_type)
        {
        case ENUM_RENDER_API_TYPE::OpenGL:
            return std::make_shared<GL_Shader>(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str(), computePath.c_str());
            break;
        default:
            return nullptr;
            break;
        }
        return nullptr;
	}

}


