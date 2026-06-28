#pragma once
#ifndef _BINDLESS_MATERIAL_DATA_
#define _BINDLESS_MATERIAL_DATA_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// GPU-side MaterialData UBO (matches std140 layout in pbr_mesh_bindless.frag Set 1 binding 0)
// Must match the shader's MaterialData block exactly for correct memory layout.
struct alignas(16) BindlessMaterialDataGPU
{
	UInt32   basecolorIndex  = 0;
	UInt32   normalIndex     = 0;
	UInt32   aormIndex       = 0;
	UInt32   cubemapIndex    = 0;
	UInt32   irradianceIndex = 0;
	UInt32   iblLutIndex     = 0;
	Float32  metallicFactor  = 0.0f;
	Float32  roughnessFactor = 0.5f;

	static constexpr UInt32 STORAGE_SIZE = 32; // 8 x 4 bytes = 32 bytes
};

// Static assert to ensure correct size and alignment
static_assert(sizeof(BindlessMaterialDataGPU) == 32, "BindlessMaterialDataGPU must be exactly 32 bytes (std140)");
static_assert(alignof(BindlessMaterialDataGPU) == 16, "BindlessMaterialDataGPU must have 16-byte alignment");

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // _BINDLESS_MATERIAL_DATA_
