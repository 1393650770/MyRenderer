#pragma once

#ifndef _GRAPHICSCONTEXT_
#define _GRAPHICSCONTEXT_
#include"RenderState.h"
#include"RenderEnum.h"
#include <string>
#include<vector>
#include<memory>

struct GLFWwindow;
namespace MXRender { class Window; }

namespace MXRender
{
    class GraphicsContext
    {
    private:
    protected:
    public:
#define NDEBUG
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

        GraphicsContext();
        virtual ~GraphicsContext() = default;
        virtual void init(Window* new_window)=0;
        virtual void pre_init();

        std::unique_ptr<RenderState> render_state;


    };
}
#endif

