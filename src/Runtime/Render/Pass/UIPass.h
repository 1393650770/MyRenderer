#pragma once
#ifndef _UI_RENDERPASS_
#define _UI_RENDERPASS_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../../RHI/RenderEnum.h"
#include<memory>
#include<string>


#include "../../RHI/Vulkan/VK_RenderPass.h"


namespace MXRender { class WindowUI; }

namespace MXRender { class VK_Texture; }

namespace MXRender { class VK_DescriptorSetLayout; }


namespace MXRender
{

    class UI_RenderPass :public VK_RenderPass
    {
    private:
    protected:
        WindowUI* window_ui;
        
        VkDescriptorPool descriptor_pool;

        void setup_descriptorpool();

        void upload_fonts();
    public:
        virtual ~UI_RenderPass();
        virtual void initialize(const PassInfo& init_info, PassOtherInfo* other_info) override;


        virtual void initialize_ui_renderbackend(WindowUI* window_ui) override;



        virtual void draw(GraphicsContext* context, RenderScene* render_scene=nullptr) override;

    };



}
#endif
