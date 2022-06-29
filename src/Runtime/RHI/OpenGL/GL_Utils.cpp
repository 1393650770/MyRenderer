
#include "GL_Utils.h"
namespace MXRender
{
    GLenum GL_Utils::Translate_API_UsageEnum_To_Opengl(ENUM_RENDER_DATA_USAGE_TYPE usage)
    {
        switch (usage)
        {

        case MXRender::ENUM_RENDER_DATA_USAGE_TYPE::STATIC_DRAW:
            return GL_STATIC_DRAW;
            break;
        case MXRender::ENUM_RENDER_DATA_USAGE_TYPE::DYNAMIC_DRAW:
            return GL_DYNAMIC_DRAW;
            break;
        default:
            return 0;
            break;
        }
        return 0;
    }
    GLenum GL_Utils::Translate_API_DataTypeEnum_To_Opengl(ENUM_RENDER_DATA_TYPE data_type)
    {
        switch (data_type)
        {
        case MXRender::ENUM_RENDER_DATA_TYPE::Float:
        case MXRender::ENUM_RENDER_DATA_TYPE::Half:
        case MXRender::ENUM_RENDER_DATA_TYPE::Mat3:
        case MXRender::ENUM_RENDER_DATA_TYPE::Mat4:
            return GL_FLOAT;
            break;
        case MXRender::ENUM_RENDER_DATA_TYPE::Int:
        case MXRender::ENUM_RENDER_DATA_TYPE::Uint8:
        case MXRender::ENUM_RENDER_DATA_TYPE::Uint10:
        case MXRender::ENUM_RENDER_DATA_TYPE::Int16:
            return GL_INT;
            break;
        case MXRender::ENUM_RENDER_DATA_TYPE::Bool:
            return GL_BOOL;
            break;
        default:
            return 0;
            break;
        }
        return 0;
    }


}