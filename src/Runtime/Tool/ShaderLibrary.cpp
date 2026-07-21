#include "ShaderLibrary.h"
#include <fstream>
#include "RHI/RenderRHI.h"
#include "RHI/RenderRource.h"
#include "RHI/RenderShader.h"
#include "RHI/RenderPipelineState.h"
#if PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)

#if PLATFORM_ANDROID
AAssetManager* ShaderLibrary::s_asset_manager = nullptr;

void ShaderLibrary::SetAssetManager(AAssetManager* in_mgr)
{
	s_asset_manager = in_mgr;
}
#endif

Vector<UInt32> ShaderLibrary::ReadSpirv(CONST String& in_filename)
{
#if PLATFORM_ANDROID
	if (s_asset_manager)
	{
		AAsset* asset = AAssetManager_open(s_asset_manager, in_filename.c_str(), AASSET_MODE_BUFFER);
		if (asset)
		{
			size_t file_size = (size_t)AAsset_getLength(asset);
			Vector<UInt32> buffer(file_size / sizeof(UInt32));
			AAsset_read(asset, buffer.data(), file_size);
			AAsset_close(asset);
			return buffer;
		}
	}
#endif
	std::ifstream file(in_filename, std::ios::ate | std::ios::binary);
	CHECK_WITH_LOG(!file.is_open(), ("Tool Error: fail to open the shader file: " + in_filename).c_str())
	size_t file_size = (size_t)file.tellg();
	Vector<UInt32> buffer(file_size / sizeof(UInt32));
	file.seekg(0);
	file.read((char*)buffer.data(), file_size);
	file.close();
	return buffer;
}

RHI::Shader* ShaderLibrary::LoadShader(ENUM_SHADER_STAGE in_stage, CONST String& in_filename)
{
	RHI::ShaderDesc desc;
	desc.shader_type = in_stage;
	desc.shader_name = in_filename;
	desc.entry_name = "main";
	RHI::ShaderDataPayload payload;
	payload.data = ReadSpirv(in_filename);
	return RHICreateShader(desc, payload);
}

RHI::RenderPipelineState* ShaderLibrary::CreateComputePSO(RHI::Shader* in_compute_shader)
{
	RHI::RenderGraphiPipelineStateDesc desc{};
	desc.shaders[ENUM_SHADER_STAGE::Shader_Compute] = in_compute_shader;
	desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
	desc.raster_state.sample_count = 1;
	return g_render_rhi->CreateRenderPipelineState(desc);
}

RHI::RenderPipelineState* ShaderLibrary::CreateComputePSOFromFile(CONST String& in_filename)
{
	RHI::Shader* compute_shader = LoadShader(ENUM_SHADER_STAGE::Shader_Compute, in_filename);
	RHI::RenderPipelineState* pso = CreateComputePSO(compute_shader);
	delete compute_shader;
	return pso;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
