#include "RenderUtils.h"
namespace MXRender
{
    unsigned RenderUtils::Get_API_DataTypeEnum_To_OS_Size(ENUM_RENDER_DATA_TYPE data_type)
    {
        switch (data_type)
        {
        case MXRender::ENUM_RENDER_DATA_TYPE::Float:
        case MXRender::ENUM_RENDER_DATA_TYPE::Half:
        case MXRender::ENUM_RENDER_DATA_TYPE::Mat3:
        case MXRender::ENUM_RENDER_DATA_TYPE::Mat4:
            return sizeof(float);
            break;
        case MXRender::ENUM_RENDER_DATA_TYPE::Int:
        case MXRender::ENUM_RENDER_DATA_TYPE::Uint8:
        case MXRender::ENUM_RENDER_DATA_TYPE::Uint10:
        case MXRender::ENUM_RENDER_DATA_TYPE::Int16:
            return sizeof(int);
            break;
        case MXRender::ENUM_RENDER_DATA_TYPE::Bool:
            return sizeof(bool);
            break;
        default:
            return 0;
            break;
        }
        return 0;
    }
}