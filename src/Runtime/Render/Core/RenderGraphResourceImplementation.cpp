#include "RenderGraphResourceImplementation.h"
#include "RenderGraphResource.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderBuffer.h"
#include "RHI/RenderPass.h"
#include "RHI/RenderFrameBuffer.h"
#include "RHI/RenderPipelineState.h"
#include "RHI/RenderShader.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// ---- Enum conversion helpers (shared by Serialize/Deserialize) ----

static CONST Char* TextureFormatToStr(Int format)
{
	static CONST Char* names[] = {
		"None","BC1","BC1A","BC2","BC3","BC4","BC5","BC6H","BC7",
		"ETC1","ETC2","ETC2A","ETC2A1","PTC12","PTC14","PTC12A","PTC14A","PTC22","PTC24",
		"ATC","ATCE","ATCI","ASTC4x4","ASTC5x5","ASTC6x6","ASTC8x5","ASTC8x6","ASTC10x5",
		"Unknown","R1","A8","R8","R8I","R8U","R8S","R16","R16I","R16U","R16F","R16S",
		"R32I","R32U","R32F","RG8","RG8I","RG8U","RG8S","RG16","RG16I","RG16U","RG16F","RG16S",
		"RG32I","RG32U","RG32F","RGB8","RGB8I","RGB8U","RGB8S","RGB9E5F",
		"RGB16I","RGB16U","RGB16F","RGB32I","RGB32U","RGB32F",
		"BGRA8","RGBA8","RGBA8I","RGBA8U","RGBA8S","RGBA16","RGBA16I","RGBA16U","RGBA16F","RGBA16S",
		"RGBA32I","RGBA32U","RGBA32F","R5G6B5","RGBA4","RGB5A1","RGB10A2","RG11B10F",
		"UnknownDepth","D16","D24","D24S8","D32","D32FS8","D16F","D24F","D32F","D0S8"
	};
	if (format < 0 || format >= (Int)(sizeof(names) / sizeof(names[0]))) return "RGBA8";
	return names[format];
}

static Int StrToTextureFormat(const String& str)
{
	static CONST Char* names[] = {
		"None","BC1","BC1A","BC2","BC3","BC4","BC5","BC6H","BC7",
		"ETC1","ETC2","ETC2A","ETC2A1","PTC12","PTC14","PTC12A","PTC14A","PTC22","PTC24",
		"ATC","ATCE","ATCI","ASTC4x4","ASTC5x5","ASTC6x6","ASTC8x5","ASTC8x6","ASTC10x5",
		"Unknown","R1","A8","R8","R8I","R8U","R8S","R16","R16I","R16U","R16F","R16S",
		"R32I","R32U","R32F","RG8","RG8I","RG8U","RG8S","RG16","RG16I","RG16U","RG16F","RG16S",
		"RG32I","RG32U","RG32F","RGB8","RGB8I","RGB8U","RGB8S","RGB9E5F",
		"RGB16I","RGB16U","RGB16F","RGB32I","RGB32U","RGB32F",
		"BGRA8","RGBA8","RGBA8I","RGBA8U","RGBA8S","RGBA16","RGBA16I","RGBA16U","RGBA16F","RGBA16S",
		"RGBA32I","RGBA32U","RGBA32F","R5G6B5","RGBA4","RGB5A1","RGB10A2","RG11B10F",
		"UnknownDepth","D16","D24","D24S8","D32","D32FS8","D16F","D24F","D32F","D0S8"
	};
	Int count = (Int)(sizeof(names) / sizeof(names[0]));
	for (Int i = 0; i < count; ++i) if (str == names[i]) return i;
	return 0;
}

template<>
std::unique_ptr<MXRender::RHI::Texture> RealizeResource<MXRender::RHI::TextureDesc, MXRender::RHI::Texture>(CONST MXRender::RHI::TextureDesc& description)
{
	// Try pooled resource first.
	auto pooled = AcquirePooledTexture(description);
	if (pooled)
		return pooled;

	std::unique_ptr<MXRender::RHI::Texture> texture(RHICreateTexture(description));
	return std::move(texture);
}

template<>
std::unique_ptr<MXRender::RHI::Buffer> RealizeResource<MXRender::RHI::BufferDesc, MXRender::RHI::Buffer>(CONST MXRender::RHI::BufferDesc& description)
{
	// Try pooled resource first.
	auto pooled = AcquirePooledBuffer(description);
	if (pooled)
		return pooled;

	std::unique_ptr<MXRender::RHI::Buffer> buffer(RHICreateBuffer(description));
	return std::move(buffer);
}
template<>
std::unique_ptr<MXRender::RHI::RenderPass> RealizeResource<MXRender::RHI::RenderPassDesc, MXRender::RHI::RenderPass>(CONST MXRender::RHI::RenderPassDesc& description)
{
	std::unique_ptr<MXRender::RHI::RenderPass> render_pass(RHICreateRenderPass(description));;
	return std::move(render_pass);
}

template<>
std::unique_ptr<MXRender::RHI::FrameBuffer> RealizeResource<MXRender::RHI::FrameBufferDesc, MXRender::RHI::FrameBuffer>(CONST MXRender::RHI::FrameBufferDesc& description)
{
	std::unique_ptr<MXRender::RHI::FrameBuffer> frame_buffer(RHICreateFrameBuffer(description));
	return std::move(frame_buffer);
}
template<>
std::unique_ptr<MXRender::RHI::RenderPipelineState> RealizeResource<MXRender::RHI::RenderGraphiPipelineStateDesc, MXRender::RHI::RenderPipelineState>(CONST MXRender::RHI::RenderGraphiPipelineStateDesc& description)
{
	std::unique_ptr<MXRender::RHI::RenderPipelineState> render_pipeline_state(RHICreateRenderPipelineState(description));
	return std::move(render_pipeline_state);
}

template<>
std::unique_ptr<MXRender::RHI::ComputePipelineState> RealizeResource<MXRender::RHI::ComputePipelineStateDesc, MXRender::RHI::ComputePipelineState>(CONST MXRender::RHI::ComputePipelineStateDesc& description)
{
	std::unique_ptr<MXRender::RHI::ComputePipelineState> compute_pipeline_state(RHICreateComputePipelineState(description));
	return std::move(compute_pipeline_state);
}

template<>
std::unique_ptr<MXRender::RHI::Shader> RealizeResource<MXRender::RHI::ShaderDesc, MXRender::RHI::Shader>(CONST MXRender::RHI::ShaderDesc& description)
{
	MXRender::RHI::ShaderDataPayload payload;
	payload.data = description.spirv_data;
	std::unique_ptr<MXRender::RHI::Shader> shader(RHICreateShader(description, payload));
	return std::move(shader);
}

// ======== ResourceDescSerializer out-of-line implementations ========

void ResourceDescSerializer<MXRender::RHI::TextureDesc>::Serialize(nlohmann::json& j, CONST MXRender::RHI::TextureDesc& d)
{
	j["texture_format"] = TextureFormatToStr((Int)d.format);
	j["width"] = d.width;
	j["height"] = d.height;
	j["mip_level"] = d.mip_level;
	j["samples"] = d.samples;
}
MXRender::RHI::TextureDesc ResourceDescSerializer<MXRender::RHI::TextureDesc>::Deserialize(CONST nlohmann::json& j)
{
	MXRender::RHI::TextureDesc d;
	d.format = static_cast<ENUM_TEXTURE_FORMAT>(StrToTextureFormat(j.value("texture_format", "RGBA8")));
	d.width = j.value("width", (UInt32)1920);
	d.height = j.value("height", (UInt32)1080);
	d.mip_level = j.value("mip_level", (UInt8)1);
	d.samples = j.value("samples", (UInt8)1);
	return d;
}

void ResourceDescSerializer<MXRender::RHI::BufferDesc>::Serialize(nlohmann::json& j, CONST MXRender::RHI::BufferDesc& d)
{
	j["buffer_size"] = d.size;
	j["buffer_stride"] = d.stride;
}
MXRender::RHI::BufferDesc ResourceDescSerializer<MXRender::RHI::BufferDesc>::Deserialize(CONST nlohmann::json& j)
{
	MXRender::RHI::BufferDesc d;
	d.size = j.value("buffer_size", (UInt32)256);
	d.stride = j.value("buffer_stride", (UInt32)16);
	return d;
}

void ResourceDescSerializer<MXRender::RHI::ShaderDesc>::Serialize(nlohmann::json& j, CONST MXRender::RHI::ShaderDesc& d)
{
	j["shader_type"] = (Int)d.shader_type;
	j["shader_name"] = d.shader_name;
	j["entry_name"] = d.entry_name;
	j["spirv_data"] = d.spirv_data;
}
MXRender::RHI::ShaderDesc ResourceDescSerializer<MXRender::RHI::ShaderDesc>::Deserialize(CONST nlohmann::json& j)
{
	MXRender::RHI::ShaderDesc d;
	d.shader_type = static_cast<ENUM_SHADER_STAGE>(j.value("shader_type", (Int)0));
	d.shader_name = j.value("shader_name", "");
	d.entry_name = j.value("entry_name", "main");
	if (j.contains("spirv_data") && j["spirv_data"].is_array())
		for (auto& v : j["spirv_data"]) d.spirv_data.push_back(v.get<UInt32>());
	return d;
}

void ResourceDescSerializer<MXRender::RHI::RenderPassDesc>::Serialize(nlohmann::json& j, CONST MXRender::RHI::RenderPassDesc& d) {}
MXRender::RHI::RenderPassDesc ResourceDescSerializer<MXRender::RHI::RenderPassDesc>::Deserialize(CONST nlohmann::json& j) { return {}; }

void ResourceDescSerializer<MXRender::RHI::FrameBufferDesc>::Serialize(nlohmann::json& j, CONST MXRender::RHI::FrameBufferDesc& d) {}
MXRender::RHI::FrameBufferDesc ResourceDescSerializer<MXRender::RHI::FrameBufferDesc>::Deserialize(CONST nlohmann::json& j) { return {}; }

void ResourceDescSerializer<MXRender::RHI::RenderGraphiPipelineStateDesc>::Serialize(nlohmann::json& j, CONST MXRender::RHI::RenderGraphiPipelineStateDesc& d) {}
MXRender::RHI::RenderGraphiPipelineStateDesc ResourceDescSerializer<MXRender::RHI::RenderGraphiPipelineStateDesc>::Deserialize(CONST nlohmann::json& j) { return {}; }

void ResourceDescSerializer<MXRender::RHI::ComputePipelineStateDesc>::Serialize(nlohmann::json& j, CONST MXRender::RHI::ComputePipelineStateDesc& d) {}
MXRender::RHI::ComputePipelineStateDesc ResourceDescSerializer<MXRender::RHI::ComputePipelineStateDesc>::Deserialize(CONST nlohmann::json& j) { return {}; }

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
