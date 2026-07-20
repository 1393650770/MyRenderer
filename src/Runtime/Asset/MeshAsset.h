#pragma once
#ifndef _MESHREASSET_
#define _MESHREASSET_
#include "Core/ConstDefine.h"
#include "RHI/RHIHandleTypes.h"
#include <atomic>
#include <future>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Buffer;
MYRENDERER_END_NAMESPACE
MYRENDERER_BEGIN_NAMESPACE(Tool)
struct MeshDataPayload;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(Asset)

// Mesh asset: async file load (Tool::MeshLoader) + lazy vertex/index buffer
// creation, mirroring TextureAsset's load/GetTexture pattern. Unlike
// TextureAsset it keeps the std::async future as a member so the load is
// genuinely asynchronous (a discarded future blocks in its destructor).
//
// Upload path: buffers are created as Vertex|Dynamic / Index|Dynamic ->
// host-visible persistently mapped -> Map/memcpy/Unmap is a direct write
// (the reliable path; plain device-local buffers would silently go through
// an unsynchronized staging+TRANSFER copy). Device-local static geometry
// upload is a future RHI iteration.
MYRENDERER_BEGIN_CLASS(MeshAsset)

#pragma region METHOD
public:
	MeshAsset() MYDEFAULT;
	MeshAsset(CONST String& in_path);     // starts LoadMesh immediately
	~MeshAsset();
	MeshAsset(CONST MeshAsset&) MYDELETE;
	MeshAsset& operator=(CONST MeshAsset&) MYDELETE;

	void METHOD(LoadMesh)(CONST String& in_path);
	Bool METHOD(IsLoaded)() CONST;
	// Block until the async load finishes. Use in OnInitScene when the mesh
	// must be ready before RDG Compile (SRB/PSO need final buffers at init).
	void METHOD(WaitUntilLoaded)();
	// Release GPU buffers + CPU payload. MUST be called before RHIShutdown
	// (e.g. in OnShutdownScene) when the asset outlives Window::Run - member
	// destruction at end of main() is too late and trips the RHI leak check.
	void METHOD(ReleaseBuffers)();

	// Lazily create + upload the RHI buffers on first call. Returns nullptr
	// while the async load is still running or if loading failed.
	RHI::Buffer* METHOD(GetVertexBuffer)();
	RHI::Buffer* METHOD(GetIndexBuffer)();
	UInt32 METHOD(GetVertexCount)() CONST { return vertex_count; }
	UInt32 METHOD(GetIndexCount)() CONST { return index_count; }
	// CPU payload access - only valid before the buffers are created
	// (payload memory is released after the GPU upload).
	CONST Tool::MeshDataPayload* METHOD(GetPayload)() CONST { return payload; }
protected:
	void METHOD(CreateBuffersIfReady)();
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	RHI::Buffer* vertex_buffer = nullptr;
	RHI::Buffer* index_buffer = nullptr;
	RHI::BufferHandle vertex_buffer_handle;
		RHI::BufferHandle index_buffer_handle;
	UInt32 vertex_count = 0;
	UInt32 index_count = 0;
	std::atomic_bool is_loaded = false;
	Tool::MeshDataPayload* payload = nullptr;
	std::future<void> load_future;
private:

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
