#pragma once
#ifndef _RENDERPASS_
#define _RENDERPASS_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "RenderEnum.h"
#include<memory>
#include<string>
namespace MXRender
{
    class FrameBuffer;

    struct PassInfo
    {
    public:
        PassInfo() =default;
        PassInfo(std::shared_ptr<FrameBuffer> framebuffer, std::string name) :pass_framebuffer(framebuffer), pass_name(name) {};
        std::shared_ptr<FrameBuffer>  pass_framebuffer;
        std::string pass_name = "";
    };
    class RenderPass
    {
    private:
        PassInfo pass_info;
    public:
        virtual void initialize(const PassInfo& init_info) = 0;
        virtual void post_initialize();
        virtual void set_commonInfo(const PassInfo& init_info);
        virtual void prepare_pass_data();
        virtual void initialize_ui_renderbackend();

        RenderPass(const PassInfo& init_info);
        virtual ~RenderPass() = default;

    };



}
#endif
