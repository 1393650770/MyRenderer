#pragma once
#ifndef _GL_STATE_
#define _GL_STATE_

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
    class GL_State
    {
    private:

    public:
        GL_State(/* args */);
        virtual ~GL_State();
    };
    
}
#endif
