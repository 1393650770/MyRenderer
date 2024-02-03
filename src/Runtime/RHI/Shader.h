#pragma once
#ifndef _SHADER_
#define _SHADER_

#include "RenderEnum.h"
#include "Core/ConstDefine.h"
#include "RenderRource.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Shader, public RenderResource)

#pragma region METHOD
public:
	Shader() DEFAULT;
	Shader(CONST ShaderDesc& desc);
	Shader(CONST ShaderDesc& desc,CONST ShaderDataPayload& data);
	VIRTUAL ~Shader();

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
