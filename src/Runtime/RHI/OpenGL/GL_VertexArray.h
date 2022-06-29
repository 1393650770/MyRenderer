#pragma once
#ifndef _GL_VERTEXARRAY_
#define _GL_VERTEXARRAY_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include"../RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include"../VertexArray.h"

namespace MXRender
{


    class GL_VertexArray:public VertexArray
    {
    private:
        unsigned id;
        std::shared_ptr<VertexBuffer> layout;
        std::shared_ptr<IndexBuffer> ebo;
    public:
        GL_VertexArray(/* args */);
        virtual ~GL_VertexArray();

        void bind() const override;
        void unbind() const override;

        void set_vertexbuffer(const std::shared_ptr < VertexBuffer>& _vertex_buffer) override;
        void set_indexbuffer(const std::shared_ptr < IndexBuffer>& _index_buffer) override;

    };
    
}
#endif
