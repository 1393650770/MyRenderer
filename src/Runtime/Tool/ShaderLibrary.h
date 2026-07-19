#pragma once
#ifndef _SHADERLIBRARY_
#define _SHADERLIBRARY_
#include "Core/ConstDefine.h"
#include "RHI/RenderEnum.h"

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
	// Read a compiled .spv binary (path relative to the working directory,
	// e.g. "Shader/xxx.vert.spv"). Keep filenames ASCII to avoid GBK/ACP issues.
	static Vector<UInt32> METHOD(ReadSpirv)(CONST String& in_filename);
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

private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
