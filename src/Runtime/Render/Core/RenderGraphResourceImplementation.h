#pragma once
#ifndef _RENDERGRAPHRESOURCEIMPLEMENTATION_
#define _RENDERGRAPHRESOURCEIMPLEMENTATION_
#include "Core/ConstDefine.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture; 
struct TextureDesc; 
class Buffer;
struct BufferDesc;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

template<typename description_type, typename actual_type>
struct missing_realize_implementation : std::false_type {};

template<typename description_type, typename actual_type>
std::unique_ptr<actual_type> Realize(CONST description_type& description)
{
	static_assert(missing_realize_implementation<description_type, actual_type>::value, "Missing realize implementation for description - type pair.");
	return nullptr;
}

template<>
extern std::unique_ptr<MXRender::RHI::Texture> Realize<MXRender::RHI::TextureDesc, MXRender::RHI::Texture>(CONST MXRender::RHI::TextureDesc& description);

template<>
extern std::unique_ptr<MXRender::RHI::Buffer> Realize<MXRender::RHI::BufferDesc, MXRender::RHI::Buffer>(CONST MXRender::RHI::BufferDesc& description);

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif

