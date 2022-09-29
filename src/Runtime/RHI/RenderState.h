#pragma once
#ifndef _RENDERSTATE_
#define _RENDERSTATE_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "RenderEnum.h"

namespace MXRender
{
    class RenderState
    {
    private:
    protected:
        bool b_depth_test_enable=false;
        bool b_depth_mask= false;
        bool b_depth_func=false;
        ENUM_DEPTH_FUNCTION  e_depth_func= ENUM_DEPTH_FUNCTION::ENUM_NONE;
        bool b_blend_enable =false;
        ENUM_BLEND_EQUATION e_blend_equation =ENUM_BLEND_EQUATION::ENUM_NONE;
        ENUM_BLEND_FACTOR e_blend_left_factor = ENUM_BLEND_FACTOR::ENUM_NONE, e_blend_right_factor = ENUM_BLEND_FACTOR::ENUM_NONE;
        ENUM_STENCIL_FUNCTION e_stencil_func= ENUM_STENCIL_FUNCTION::ENUM_NONE;
        bool b_stencil_test_enable=false;
        uint32_t stencil_mask_num=0;
        bool b_cull_enable=false;
        bool b_cull_frontOrback=false;
    public:
        static inline ENUM_RENDER_API_TYPE render_api_type = ENUM_RENDER_API_TYPE::OpenGL;


        RenderState() ;
        virtual ~RenderState() ;

        //清楚所有
        virtual void clear_all()=0;

        //深度相关
        virtual void set_depth_test_enable(bool enable) = 0;
        virtual void set_depth_mask(bool mask_flag) = 0;
        virtual void set_depth_func(ENUM_DEPTH_FUNCTION func) = 0;

        //混合相关
        virtual void set_blend_enable(bool enable) = 0;
        virtual void set_blend_equation(ENUM_BLEND_EQUATION equation) = 0;
        virtual void set_blend_func(ENUM_BLEND_FACTOR left_factor, ENUM_BLEND_FACTOR right_factor) =0;

        //模板相关
        virtual void set_stencil_func(ENUM_STENCIL_FUNCTION stencil_func, int32_t ref, uint32_t mask) = 0;
        virtual void set_frontOrback_stencil_operation(bool  is_front, ENUM_STENCIL_OPERATIOON stencil_fail, ENUM_STENCIL_OPERATIOON depth_fail, ENUM_STENCIL_OPERATIOON depth_success) = 0;
        virtual void set_stencil_test_enable(bool enable) = 0;
        virtual void clear_stencil() = 0;
        virtual void stencil_mask(uint32_t mask) = 0;

        //剔除相关
        virtual void set_cull_enable(bool enable) = 0;
        virtual void set_cull_frontOrback(bool b_isfront) = 0;


        static std::unique_ptr<RenderState> CreateRenderState();

    };


}
#endif //_RENDERSTATE_
