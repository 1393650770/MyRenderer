#pragma once
#ifndef _SHADER_
#define _SHADER_

#include "RenderEnum.h"
#include "../Core/ConstDefine.h"
#include "RenderRource.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_STRUCT(ShaderDesc)
ShaderDesc() DEFAULT;
ShaderDesc(ENUM_SHADER_STAGE in_shader_type) : shader_type(in_shader_type) {}
ENUM_SHADER_STAGE shader_type = ENUM_SHADER_STAGE::Invalid;
String debug_name;
String entry_name = "main";

MYRENDERER_END_STRUCT



MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Shader, public RenderResource)

private:
protected:
    // ≥Ã–ÚID
    ShaderDesc desc;
public:
    Shader() DEFAULT;
    virtual ~Shader();
    //New api
    virtual void addUniformName(const std::string& name, uint64_t uniformSize) = 0;
    
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif //_SHADER_
