#pragma once
#ifndef _GL_UTILS_
#define _GL_UTILS_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include"../RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace MXRender
{
    class GL_Utils
    {
    private:

    public:
        static GLenum Translate_API_UsageEnum_To_Opengl(ENUM_RENDER_DATA_USAGE_TYPE usage);
        static GLenum Translate_API_DataTypeEnum_To_Opengl(ENUM_RENDER_DATA_TYPE data_type);

    };
    
}
#endif
