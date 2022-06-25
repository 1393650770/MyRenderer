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
        static ENUM_RENDER_API_TYPE render_api_type;

        RenderState();
        virtual ~RenderState();




    };


}
#endif //_RENDERSTATE_
