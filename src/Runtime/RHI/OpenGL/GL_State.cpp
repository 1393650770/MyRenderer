#include"GL_State.h"
#include"GL_Utils.h"
namespace MXRender
{
    GL_State::GL_State(/* args */)
    {

    }
    
    GL_State::~GL_State()
    {
    }

    void GL_State::clear_all()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }


    void GL_State::set_depth_test_enable(bool enable)
    {
        enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    }

    void GL_State::set_depth_mask(bool mask_flag)
    {
        mask_flag ? glDepthMask(GL_TRUE) : glDepthMask(GL_TRUE);
    }

    void GL_State::set_depth_func(ENUM_DEPTH_FUNCTION func)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_Utils::Translate_API_DepthFunctionEnum_To_Opengl(func));
    }

    void GL_State::set_blend_enable(bool enable)
    {
        enable? glEnable(GL_BLEND): glDisable(GL_BLEND);
    }

    void GL_State::set_blend_equation(ENUM_BLEND_EQUATION equation)
    {
        glEnable(GL_BLEND);
        glBlendEquation(GL_Utils::Translate_API_BlendEquationEnum_To_Opengl(equation));
    }

    void GL_State::set_blend_func(ENUM_BLEND_FACTOR left_factor, ENUM_BLEND_FACTOR right_factor)
    {
        glBlendFunc(GL_Utils::Translate_API_BlendFactorEnum_To_Opengl(left_factor), GL_Utils::Translate_API_BlendFactorEnum_To_Opengl(right_factor));
    }


    void GL_State::set_stencil_func(ENUM_STENCIL_FUNCTION stencil_func, int32_t ref, uint32_t mask)
    {
        glStencilFunc(GL_Utils::Translate_API_StencilFunctionEnum_To_Opengl(stencil_func), ref, mask);
    }

    void GL_State::set_frontOrback_stencil_operation(bool  is_front, ENUM_STENCIL_OPERATIOON stencil_fail, ENUM_STENCIL_OPERATIOON depth_fail, ENUM_STENCIL_OPERATIOON depth_success)
    {
        GLenum face = is_front ? GL_FRONT : GL_BACK;
        glStencilOpSeparate(face, GL_Utils::Translate_API_StencilOperationEnum_To_Opengl(stencil_fail), GL_Utils::Translate_API_StencilOperationEnum_To_Opengl(depth_fail), GL_Utils::Translate_API_StencilOperationEnum_To_Opengl(depth_success));
    }

    void GL_State::set_stencil_test_enable(bool enable)
    {
        enable? glEnable(GL_STENCIL_TEST): glDisable(GL_STENCIL_TEST);
    }



    void GL_State::clear_stencil()
    {
        glClear(GL_STENCIL_BUFFER_BIT);
    }

    void GL_State::stencil_mask(uint32_t mask)
    {
        glStencilMask(mask);
    }

    void GL_State::set_cull_enable(bool enable)
    {
        enable ? glEnable(GL_CULL_FACE): glDisable(GL_CULL_FACE);
    }

    void GL_State::set_cull_frontOrback(bool b_isfront)
    {
        b_isfront ? glCullFace(GL_FRONT): glCullFace(GL_BACK);
    }





}