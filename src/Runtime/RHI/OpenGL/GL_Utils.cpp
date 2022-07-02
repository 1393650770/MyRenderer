
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

    GLenum GL_Utils::Translate_API_DepthFunctionEnum_To_Opengl(ENUM_DEPTH_FUNCTION depth_func)
    {
        switch (depth_func)
        {
        case ENUM_DEPTH_FUNCTION::ENUM_EQUAL:
        {
            return GL_EQUAL;
            break;
        }
        case ENUM_DEPTH_FUNCTION::ENUM_LEAQUAL:
        {
            return GL_LEQUAL;
            break;
        }
        case ENUM_DEPTH_FUNCTION::ENUM_LESS:
        {
            return GL_LESS;
            break;
        }
        case ENUM_DEPTH_FUNCTION::ENUM_GREATER:
        {
            return GL_GREATER;
            break;
        }
        case ENUM_DEPTH_FUNCTION::ENUM_GEQUAL:
        {
            return GL_GEQUAL;
            break;
        }
        case ENUM_DEPTH_FUNCTION::ENUM_NOTEQUAL:
        {
            return GL_NOTEQUAL;
            break;
        }
        case ENUM_DEPTH_FUNCTION::ENUM_ALWAYS:
        {
            return GL_ALWAYS;
            break;
        }
        default:
        {
            return -1;
            break;
        }
        }
        return -1;
    }

    GLenum GL_Utils::Translate_API_StencilFunctionEnum_To_Opengl(ENUM_STENCIL_FUNCTION stencil_func)
    {
        switch (stencil_func)
        {

        case MXRender::ENUM_STENCIL_FUNCTION::ENUM_ALWAYS:
            return GL_ALWAYS;
            break;
        case MXRender::ENUM_STENCIL_FUNCTION::ENUM_NOTEQUAL:
            return GL_NOTEQUAL;
            break;
        case MXRender::ENUM_STENCIL_FUNCTION::ENUM_NEVER:
            return GL_NEVER;
            break;
        case MXRender::ENUM_STENCIL_FUNCTION::ENUM_LESS:
            return GL_LESS;
            break;
        case MXRender::ENUM_STENCIL_FUNCTION::ENUM_LEQUAL:
            return GL_LEQUAL;
            break;
        case MXRender::ENUM_STENCIL_FUNCTION::ENUM_GREATER:
            return GL_GREATER;
            break;
        case MXRender::ENUM_STENCIL_FUNCTION::ENUM_EQUAL:
            return GL_EQUAL;
            break;
        default:
            return GL_ALWAYS;
            break;
        }
        return -1;
    }

    GLenum GL_Utils::Translate_API_StencilOperationEnum_To_Opengl(ENUM_STENCIL_OPERATIOON stencil_operation)
    {
        switch (stencil_operation)
        {
        case MXRender::ENUM_STENCIL_OPERATIOON::ENUM_KEEP:
            return GL_KEEP;
            break;
        case MXRender::ENUM_STENCIL_OPERATIOON::ENUM_ADD:
            return GL_INCR;
            break;
        case MXRender::ENUM_STENCIL_OPERATIOON::ENUM_SUB:
            return GL_DECR;
            break;
        default:
            return GL_KEEP;
            break;
        }
        return -1;
    }

    GLenum GL_Utils::Translate_API_BlendEquationEnum_To_Opengl(ENUM_BLEND_EQUATION blend_quation)
    {
        switch (blend_quation)
        {
        case MXRender::ENUM_BLEND_EQUATION::ENUM_ADD:
            return GL_FUNC_ADD;
            break;
        case MXRender::ENUM_BLEND_EQUATION::ENUM_SUB:
            return GL_FUNC_SUBTRACT;
            break;
        case MXRender::ENUM_BLEND_EQUATION::ENUM_REVERSE_SUB:
            return GL_FUNC_REVERSE_SUBTRACT;
            break;

        default:
            return -1;
            break;
        }
        return -1;
    }

    GLenum GL_Utils::Translate_API_BlendFactorEnum_To_Opengl(ENUM_BLEND_FACTOR blend_factor)
    {
        switch (blend_factor)
        {
        case MXRender::ENUM_BLEND_FACTOR::ENUM_ZERO:
            return GL_ZERO;
            break;
        case MXRender::ENUM_BLEND_FACTOR::ENUM_ONE:
            return GL_ONE;
            break;
        case MXRender::ENUM_BLEND_FACTOR::ENUM_SRC_COLOR:
            return GL_SRC_COLOR;
            break;
        case MXRender::ENUM_BLEND_FACTOR::ENUM_DST_COLOR:
            return GL_DST_COLOR;
            break;
        case MXRender::ENUM_BLEND_FACTOR::ENUM_SRC_ALPHA:
            return GL_SRC_ALPHA;
            break;
        case MXRender::ENUM_BLEND_FACTOR::ENUM_DST_ALPHA:
            return GL_DST_ALPHA;
            break;
        case MXRender::ENUM_BLEND_FACTOR::ENUM_ONE_MINUS_SRC_COLOR:
            return GL_ONE_MINUS_SRC_COLOR;
            break;
        case MXRender::ENUM_BLEND_FACTOR::ENUM_ONE_MINUS_DST_COLOR:
            return GL_ONE_MINUS_SRC_COLOR;
            break;
        case MXRender::ENUM_BLEND_FACTOR::ENUM_ONE_MINUS_SRC_ALPHA:
            return GL_ONE_MINUS_SRC_ALPHA;
            break;
        case MXRender::ENUM_BLEND_FACTOR::ENUM_ONE_MINUS_DST_ALPHA:
            return GL_ONE_MINUS_DST_ALPHA;
            break;
        default:
            return GL_ONE;
            break;
        }
        return -1;
    }




}