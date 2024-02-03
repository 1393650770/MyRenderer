#include "Shader.h"

#include"RenderState.h"
#include "Vulkan/VK_Shader.h"


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

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
