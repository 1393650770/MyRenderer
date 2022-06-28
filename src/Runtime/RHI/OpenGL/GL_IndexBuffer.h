#pragma once
#ifndef _GL_INDEXBUFFER_
#define _GL_INDEXBUFFER_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include"../RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include"../IndexBuffer.h"
namespace MXRender
{
    class GL_IndexBuffer :public IndexBuffer
    {
    private:

    public:
        GL_IndexBuffer(/* args */);
        virtual ~GL_IndexBuffer();

        void bind() const override;
        void unbind() const override;

        void set_data(const void* data, unsigned size) override;


    };
    
}
#endif
