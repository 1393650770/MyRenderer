#pragma once
#ifndef _RENDERSTATE_
#define _RENDERSTATE_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "RenderEnum.h"

namespace MXRender
{
    class RenderState
    {
    private:

    public:
        static inline ENUM_RENDER_API_TYPE render_api_type = ENUM_RENDER_API_TYPE::OpenGL;

        RenderState();
        virtual ~RenderState();

    };


}
#endif //_RENDERSTATE_
