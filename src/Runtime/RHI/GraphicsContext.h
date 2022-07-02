#pragma once

#ifndef _GRAPHICSCONTEXT_
#define _GRAPHICSCONTEXT_
#include"RenderState.h"
#include"RenderEnum.h"
#include <string>
#include<vector>
#include<memory>

namespace MXRender
{
    class GraphicsContext
    {
    public:
        GraphicsContext();
        virtual ~GraphicsContext() = default;
        
        std::unique_ptr<RenderState> render_state;

    };
}
#endif

