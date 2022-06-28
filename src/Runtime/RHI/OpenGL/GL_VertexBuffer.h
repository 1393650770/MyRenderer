#pragma once
#ifndef _GL_VERTEXBUFFER_
#define _GL_VERTEXBUFFER_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include"../RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include"../VertexBuffer.h"
namespace MXRender
{
    class GL_VertexBuffer:public VertexBuffer
    {
    private:

    public:
        GL_VertexBuffer(/* args */);
        virtual ~GL_VertexBuffer();

        void bind() const override;
        void unbind() const override;

        void set_data(const void* data, unsigned size) override;

        const Layout& get_layout() const override;
        void set_layout(const Layout& layout) override;

    };
    
}
#endif
