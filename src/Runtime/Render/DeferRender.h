#pragma once
#ifndef _DEFERRENDER_
#define _DEFERRENDER_
#include "MyRender.h"

namespace MXRender { class WindowUI; }


namespace MXRender { class UI_RenderPass; }


namespace MXRender { class MainCamera_RenderPass; }
namespace MXRender { class Mesh_RenderPass; }
namespace MXRender { class VK_Viewport; }
namespace MXRender
{
    class DeferRender :
        public MyRender
    {
    public:
        std::shared_ptr < VK_Viewport> main_viewport;
        std::shared_ptr < MainCamera_RenderPass> main_camera_pass ;
        std::shared_ptr < Mesh_RenderPass> mesh_pass; 
        std::shared_ptr < UI_RenderPass> ui_pass;

        DeferRender();
        virtual ~DeferRender();
        void run(std::weak_ptr <VK_GraphicsContext> context) override;
        void init(std::weak_ptr <VK_GraphicsContext> context, GLFWwindow* window,WindowUI* window_ui) override;
    private:

    };
}

#endif // !_DEFERRENDER_


