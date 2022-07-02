#pragma once
#ifndef _VERTEXARRAY_
#define _VERTEXARRAY_

#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderEnum.h"

namespace MXRender
{
    class VertexArray
    {
    private:

    public:
        VertexArray() = default;
        virtual ~VertexArray();

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void set_vertexbuffer(const std::shared_ptr < VertexBuffer>& _vertex_buffer)=0;

        virtual void set_indexbuffer(const std::shared_ptr < IndexBuffer>& _index_buffer) = 0;

        static std::shared_ptr<VertexArray> CreateVertexArray();
    };

}
#endif


