#pragma once
#ifndef _VK_RENDERPASS_
#define _VK_RENDERPASS_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../RenderEnum.h"
#include<memory>
#include<string>
#include "../RenderPass.h"

namespace MXRender
{

    class VK_RenderPass:public RenderPass
    {
    protected:
        std::vector<RenderPipelineBase> render_pipeline_array;
    public:
        virtual void initialize(const PassInfo& init_info);
        virtual void post_initialize();
        virtual void set_commonInfo(const PassInfo& init_info);
        virtual void prepare_pass_data();
        virtual void initialize_ui_renderbackend();
        VK_RenderPass();
        VK_RenderPass(const PassInfo& init_info);
        virtual ~VK_RenderPass() ;

    };



}
#endif
