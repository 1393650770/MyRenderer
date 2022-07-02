#pragma once
#ifndef _GL_STATE_
#define _GL_STATE_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../RenderState.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace MXRender
{
    class GL_State:public RenderState
    {
    private:

    public:
        GL_State(/* args */);
        virtual ~GL_State();

        //清楚所有
        void clear_all() override;

        //深度相关
        void set_depth_test_enable(bool enable) override;
        void set_depth_mask(bool mask_flag) override;
        void set_depth_func(ENUM_DEPTH_FUNCTION func) override;

        //混合相关
        void set_blend_enable(bool enable) override;
        void set_blend_equation(ENUM_BLEND_EQUATION equation) override;
        void set_blend_func(ENUM_BLEND_FACTOR left_factor, ENUM_BLEND_FACTOR right_factor) override;

        //模板相关
        void set_stencil_func(ENUM_STENCIL_FUNCTION stencil_func, int32_t ref, uint32_t mask) override;
        void set_frontOrback_stencil_operation(bool is_front, ENUM_STENCIL_OPERATIOON stencil_fail, ENUM_STENCIL_OPERATIOON depth_fail, ENUM_STENCIL_OPERATIOON depth_success) override;
        void set_stencil_test_enable(bool enable) override;
        void clear_stencil() override;
        void stencil_mask(uint32_t mask) override;

        //剔除相关
        void set_cull_enable(bool enable) override;
        void set_cull_frontOrback(bool b_isfront) override;

    };
    
}
#endif
