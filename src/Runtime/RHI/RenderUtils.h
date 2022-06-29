#pragma once
#ifndef _RENDERUTILS_
#define _RENDERUTILS_

#include "RenderEnum.h"
namespace MXRender
{
    class RenderUtils
    {
    public:
        static unsigned Get_API_DataTypeEnum_To_OS_Size(ENUM_RENDER_DATA_TYPE data_type);
    };

}
#endif //_RENDERUTILS_
