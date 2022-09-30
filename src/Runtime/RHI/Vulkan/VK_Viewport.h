#pragma once
#ifndef _VK_VIEWPORT_
#define _VK_VIEWPORT_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../RenderEnum.h"

namespace MXRender
{
    class VK_SwapChain;
    class GraphicsContext;
    struct VK_SwapChainRecreateInfo;

    class VK_Viewport
    {
    private:
    protected:
        int size_x;
        int size_y;
        void * window_handle;
        VK_SwapChain* swapchain;
        bool b_isfullscreen=false;

    public:
        VK_Viewport(std::shared_ptr<GraphicsContext> Context, void* InWindowHandle, int InSizeX, int InSizeY, bool bInIsFullscreen);
        virtual ~VK_Viewport();

        //void create_swapchain();
        void destroy_swapchain(VK_SwapChainRecreateInfo* RecreateInfo);
    };

}
#endif
