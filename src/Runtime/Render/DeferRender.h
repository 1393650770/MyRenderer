#pragma once
#ifndef _DEFERRENDER_
#define _DEFERRENDER_
#include "MyRender.h"
#include"../RHI/VertexArray.h"
#include"../RHI/VertexBuffer.h"
#include"../RHI/IndexBuffer.h"
#include"../RHI/Shader.h"

namespace MXRender { class MainCamera_RenderPass; }

namespace MXRender { class VK_Viewport; }
namespace MXRender
{
    class DeferRender :
        public MyRender
    {
    public:
        std::shared_ptr < VK_Viewport> main_viewport;
        std::shared_ptr < MainCamera_RenderPass> main_camera_pass ;
        DeferRender();
        virtual ~DeferRender();
        void run(std::weak_ptr <VK_GraphicsContext> context) override;
        void init(std::weak_ptr <VK_GraphicsContext> context, GLFWwindow* window) override;
    private:
        unsigned int VBO, VAO, EBO;
        std::shared_ptr<VertexArray> vertex_array;
        std::shared_ptr<VertexBuffer> vertex_buffer;
        std::shared_ptr<IndexBuffer> index_buffer;
        std::shared_ptr<Shader> shader;
    };
}

#endif // !_DEFERRENDER_


