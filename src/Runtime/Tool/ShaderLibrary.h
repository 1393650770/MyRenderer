#pragma once
#ifndef _SHADERLIBRARY_
#define _SHADERLIBRARY_
#include "Core/ConstDefine.h"
#include "RHI/RenderEnum.h"

#if PLATFORM_ANDROID
struct AAssetManager;
#endif

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Shader;
class RenderPipelineState;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)

// Shader file IO + PSO creation helpers shared by all samples.
// Collects the ReadShader / LoadShaderFile / CreateComputePSO copies that used
// to be re-written in every sample (HelloTriangle, Fluid2D/3D, Ocean, 6-NN).
MYRENDERER_BEGIN_CLASS(ShaderLibrary)
#pragma region METHOD
public:
#if PLATFORM_ANDROID
	static void METHOD(SetAssetManager)(AAssetManager* in_mgr);
#endif
	// Read a compiled .spv binary (path relative to the working directory,
	// e.g. "Shader/xxx.vert.spv"). Keep filenames ASCII to avoid GBK/ACP issues.
	static Vector<UInt32> METHOD(ReadSpirv)(CONST String& in_filename);
#if PLATFORM_WGPU
	// Read a .wgsl text file (WebGPU only). Returns UTF-8 WGSL source.
	static String METHOD(ReadWgsl)(CONST String& in_filename);
#endif
	// Create a shader of any stage. shader_name is set to the file path so the
	// PSO cache hash stays unique per shader (the cache hashes shader_name).
	static RHI::Shader* METHOD(LoadShader)(ENUM_SHADER_STAGE in_stage, CONST String& in_filename);
	// Current engine convention: compute PSOs go through the graphics desc with
	// only shaders[Shader_Compute] filled (the separate ComputePipelineState
	// path has no CommandList bind entry point - do not switch to it).
	static RHI::RenderPipelineState* METHOD(CreateComputePSO)(RHI::Shader* in_compute_shader);
	// One-step helper: LoadShader + CreateComputePSO + delete shader.
	// Safe against the PSO-cache pointer-recycling pitfall because the cache
	// hash includes shader_name (= file path) which differs per shader. For
	// batch creation still prefer: load ALL shaders -> create ALL PSOs -> delete.
	static RHI::RenderPipelineState* METHOD(CreateComputePSOFromFile)(CONST String& in_filename);
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
#if PLATFORM_ANDROID
	static AAssetManager* s_asset_manager;
#endif
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
