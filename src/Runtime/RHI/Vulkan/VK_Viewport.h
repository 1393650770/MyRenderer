#pragma once
#ifndef _VK_VIEWPORT_
#define _VK_VIEWPORT_
#include <vulkan/vulkan.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "VK_Resource.h"
#include "../RenderEnum.h"

namespace MXRender
{
	class VK_Device;
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
        std::vector<VkImage> image_array;
        std::vector<VK_TextureView> texture_view_array;
        std::weak_ptr< VK_Device> device;
    public:
        VK_Viewport(std::shared_ptr<GraphicsContext> Context, void* InWindowHandle, int InSizeX, int InSizeY, bool bInIsFullscreen);
        virtual ~VK_Viewport();

        //void create_swapchain();
        void destroy_swapchain(VK_SwapChainRecreateInfo* RecreateInfo);
        void destroy_image_view();
        void create_image_view_from_swapchain();
        VK_SwapChain* get_swapchain();
        int get_image_num() const;
        std::vector<VkImage>& get_image_array() ;
        std::vector<VK_TextureView>& get_image_view_array();
    };

}
#endif
