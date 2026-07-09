#pragma once
#ifndef _RENDERGRAPHRESOURCEIMPLEMENTATION_
#define _RENDERGRAPHRESOURCEIMPLEMENTATION_
#include "Core/ConstDefine.h"
#include <nlohmann/json.hpp>
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture; 
struct TextureDesc; 
class Buffer;
struct BufferDesc;
class RenderPass;
struct RenderPassDesc;
class FrameBuffer;
struct FrameBufferDesc;
class RenderPipelineState;
struct RenderGraphiPipelineStateDesc;
class ComputePipelineState;
struct ComputePipelineStateDesc;
class Shader;
struct ShaderDesc;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// ---- ResourceDescSerializer: canonical Desc↔JSON bridge ----
template<typename Desc>
struct ResourceDescSerializer
{
	static_assert(sizeof(Desc) == 0, "Missing ResourceDescSerializer specialization.");
};

template<> struct ResourceDescSerializer<MXRender::RHI::TextureDesc> {
	static void Serialize(nlohmann::json& j, CONST MXRender::RHI::TextureDesc& d);
	static MXRender::RHI::TextureDesc Deserialize(CONST nlohmann::json& j);
};
template<> struct ResourceDescSerializer<MXRender::RHI::BufferDesc> {
	static void Serialize(nlohmann::json& j, CONST MXRender::RHI::BufferDesc& d);
	static MXRender::RHI::BufferDesc Deserialize(CONST nlohmann::json& j);
};
template<> struct ResourceDescSerializer<MXRender::RHI::ShaderDesc> {
	static void Serialize(nlohmann::json& j, CONST MXRender::RHI::ShaderDesc& d);
	static MXRender::RHI::ShaderDesc Deserialize(CONST nlohmann::json& j);
};
template<> struct ResourceDescSerializer<MXRender::RHI::RenderPassDesc> {
	static void Serialize(nlohmann::json& j, CONST MXRender::RHI::RenderPassDesc& d);
	static MXRender::RHI::RenderPassDesc Deserialize(CONST nlohmann::json& j);
};
template<> struct ResourceDescSerializer<MXRender::RHI::FrameBufferDesc> {
	static void Serialize(nlohmann::json& j, CONST MXRender::RHI::FrameBufferDesc& d);
	static MXRender::RHI::FrameBufferDesc Deserialize(CONST nlohmann::json& j);
};
template<> struct ResourceDescSerializer<MXRender::RHI::RenderGraphiPipelineStateDesc> {
	static void Serialize(nlohmann::json& j, CONST MXRender::RHI::RenderGraphiPipelineStateDesc& d);
	static MXRender::RHI::RenderGraphiPipelineStateDesc Deserialize(CONST nlohmann::json& j);
};
template<> struct ResourceDescSerializer<MXRender::RHI::ComputePipelineStateDesc> {
	static void Serialize(nlohmann::json& j, CONST MXRender::RHI::ComputePipelineStateDesc& d);
	static MXRender::RHI::ComputePipelineStateDesc Deserialize(CONST nlohmann::json& j);
};

template<typename description_type, typename actual_type>
struct missing_realize_implementation : std::false_type {};

template<typename description_type, typename actual_type>
std::unique_ptr<actual_type> RealizeResource(CONST description_type& description)
{
	static_assert(missing_realize_implementation<description_type, actual_type>::value, "Missing realize implementation for description - type pair.");
	return nullptr;
}

template<>
extern std::unique_ptr<MXRender::RHI::Texture> RealizeResource<MXRender::RHI::TextureDesc, MXRender::RHI::Texture>(CONST MXRender::RHI::TextureDesc& description);

template<>
extern std::unique_ptr<MXRender::RHI::Buffer> RealizeResource<MXRender::RHI::BufferDesc, MXRender::RHI::Buffer>(CONST MXRender::RHI::BufferDesc& description);

template<>
extern std::unique_ptr<MXRender::RHI::RenderPass> RealizeResource<MXRender::RHI::RenderPassDesc, MXRender::RHI::RenderPass>(CONST MXRender::RHI::RenderPassDesc& description);

template<>
extern std::unique_ptr<MXRender::RHI::FrameBuffer> RealizeResource<MXRender::RHI::FrameBufferDesc, MXRender::RHI::FrameBuffer>(CONST MXRender::RHI::FrameBufferDesc& description);
template<>
extern std::unique_ptr<MXRender::RHI::RenderPipelineState> RealizeResource<MXRender::RHI::RenderGraphiPipelineStateDesc, MXRender::RHI::RenderPipelineState>(CONST MXRender::RHI::RenderGraphiPipelineStateDesc& description);

template<>
extern std::unique_ptr<MXRender::RHI::ComputePipelineState> RealizeResource<MXRender::RHI::ComputePipelineStateDesc, MXRender::RHI::ComputePipelineState>(CONST MXRender::RHI::ComputePipelineStateDesc& description);

template<>
extern std::unique_ptr<MXRender::RHI::Shader> RealizeResource<MXRender::RHI::ShaderDesc, MXRender::RHI::Shader>(CONST MXRender::RHI::ShaderDesc& description);


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif

