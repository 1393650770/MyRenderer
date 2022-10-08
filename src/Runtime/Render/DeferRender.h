#pragma once
#ifndef _DEFERRENDER_
#define _DEFERRENDER_
#include "MyRender.h"

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

    };
}

#endif // !_DEFERRENDER_


