#pragma once
#ifndef _DEFERRENDER_
#define _DEFERRENDER_
#include "MyRender.h"
#include"../RHI/VertexArray.h"
#include"../RHI/VertexBuffer.h"
#include"../RHI/IndexBuffer.h"
#include"../RHI/Shader.h"
namespace MXRender
{
    class DeferRender :
        public MyRender
    {
    public:
        DeferRender();
        virtual ~DeferRender();
        void run() override;
        void init() override;
    private:
        unsigned int VBO, VAO, EBO;
        std::shared_ptr<VertexArray> vertex_array;
        std::shared_ptr<VertexBuffer> vertex_buffer;
        std::shared_ptr<IndexBuffer> index_buffer;
        std::shared_ptr<Shader> shader;
    };
}

#endif // !_DEFERRENDER_


