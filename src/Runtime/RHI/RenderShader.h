#pragma once
#ifndef _SHADER_
#define _SHADER_

#include "RenderEnum.h"
#include "Core/ConstDefine.h"
#include "RenderRource.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_STRUCT( ReflectionOverrides) 
	CONST Char* name;
	ENUM_BINDING_RESOURCE_TYPE override_type;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS(ShaderResourceBinding)

#pragma region METHOD
public:
	ShaderResourceBinding() DEFAULT;

	VIRTUAL ~ShaderResourceBinding();

	VIRTUAL void METHOD(SetResource)(CONST String& name, CONST RenderResource* resource) PURE;
protected:
private:

#pragma endregion

#pragma region MEMBER
public:
protected:
	ENUM_SHADER_RESOURCE_BINDING_POINT_TYPE type = ENUM_SHADER_RESOURCE_BINDING_POINT_TYPE::Invalid;
private:

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Shader, public RenderResource)

#pragma region METHOD
public:
	Shader() DEFAULT;
	Shader(CONST ShaderDesc& desc);
	Shader(CONST ShaderDesc& desc,CONST ShaderDataPayload& data);
	VIRTUAL ~Shader();

	CONST ShaderDesc& GetDesc() CONST;
protected:
private:

#pragma endregion

#pragma region MEMBER
public:
protected:
	ShaderDesc desc;
private:

#pragma endregion
    
MYRENDERER_END_CLASS



MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif //_SHADER_
