#pragma once
#ifndef _DEFERRENDER_
#define _DEFERRENDER_
#include "MyRender.h"

namespace MXRender { class Copy_RenderPass; }

namespace MXRender { class RenderScene; }


namespace MXRender { class PreComputeIBL_RenderPass; }

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
        std::shared_ptr < Copy_RenderPass> copy_pass;
        std::shared_ptr < UI_RenderPass> ui_pass;
        std::shared_ptr < PreComputeIBL_RenderPass> precomputeibl_pass;
        DeferRender();
        virtual ~DeferRender();
        void run(std::weak_ptr <VK_GraphicsContext> context, RenderScene* render_scene) override;
        void init(std::weak_ptr <VK_GraphicsContext> context, GLFWwindow* window,WindowUI* window_ui) override;
    private:

    };
}

#endif // !_DEFERRENDER_


