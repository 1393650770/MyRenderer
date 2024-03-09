#include "RenderShader.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)


Shader::~Shader()
{
}

Shader::Shader(CONST ShaderDesc& desc): desc(desc)
{

}

Shader::Shader(CONST ShaderDesc& desc, CONST ShaderDataPayload& data): desc(desc)
{

}

CONST ShaderDesc& Shader::GetDesc() CONST
{
	return desc;
}

ShaderResourceBinding::~ShaderResourceBinding()
{

}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
