#pragma once
#ifndef _RHIHANDLETYPES_
#define _RHIHANDLETYPES_

#include "Core/ResourceHandle.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

// RHI Handle Tags (compile-time type safety via empty structs)
struct TagTexture {};
struct TagBuffer {};
struct TagShader {};
struct TagPipelineState {};
struct TagSRB {};
struct TagSampler {};
struct TagRenderPass {};
struct TagFrameBuffer {};
struct TagComputePipelineState {};

using TextureHandle   = ResourceHandle<TagTexture>;
using BufferHandle    = ResourceHandle<TagBuffer>;
using ShaderHandle    = ResourceHandle<TagShader>;
using PSOHandle       = ResourceHandle<TagPipelineState>;
using SRBHandle       = ResourceHandle<TagSRB>;
using SamplerHandle   = ResourceHandle<TagSampler>;


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
