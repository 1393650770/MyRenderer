#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <limits>
#include "Application/Window.h"
#include "Render/RenderInterface.h"
#include "Render/Core/RenderGraph.h"
#include "RHI/RenderRource.h"
#include "Render/Core/RenderGraphPass.h"
#include "RHI/RenderPass.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderCommandList.h"
#include "RHI/Vulkan/VK_Viewport.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderShader.h"
#include "RHI/RenderPipelineState.h"
#include "RHI/RenderBuffer.h"
using namespace MXRender::RHI;
using namespace MXRender::Render;
using namespace MXRender::Application;
using namespace MXRender::RHI::Vulkan;
using namespace MXRender;

// -------- PBF simulation constants (must match the GLSL constants) --------
static CONST UInt32 MAX_PARTICLES = 32768;
static CONST UInt32 LOCAL_SIZE = 256;          // 1D particle passes
static CONST UInt32 EMIT_PER_FRAME = 16;       // keep the emitted slug near rest density
static CONST Int SOLVER_ITERS = 3;             // must be odd: chain a->b->a->b ends in xstar_b
// uniform grid over the whole domain (pool footprint x domain height)
static CONST UInt32 GRID_X = 30;
static CONST UInt32 GRID_Y = 20;
static CONST UInt32 GRID_Z = 22;
static CONST UInt32 GRID_CELLS = GRID_X * GRID_Y * GRID_Z; // 13200
static CONST UInt32 CELL_CAP = 32;
static CONST UInt32 NBR_MAX = 48;
// world: pool inner walls x[-1.4,1.4] z[-1.0,1.0], floor y=0, domain top y=1.9
static CONST Float32 POOL_X = 1.4f;
static CONST Float32 POOL_Z = 1.0f;
static CONST Float32 WATER_REF_Y = 0.3f;       // aim / interaction reference plane
static CONST Float32 GRAVITY = 9.8f;
static CONST Float32 JET_SPEED = 3.5f;
// screen-space ink buffers (fixed internal resolution, 4:3 like the window)
static CONST UInt32 IRES_W = 640;
static CONST UInt32 IRES_H = 480;

Vector<UInt32> ReadShader(CONST String& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	CHECK_WITH_LOG(!file.is_open(), " App Error: fail to open the shader file! ")

	size_t fileSize = (size_t)file.tellg();
	Vector<UInt32> buffer(fileSize / sizeof(UInt32));

	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);

	file.close();

	return std::move(buffer);
}

static Shader* LoadShaderFile(ENUM_SHADER_STAGE stage, CONST String& filename)
{
	ShaderDesc desc;
	desc.shader_type = stage;
	desc.entry_name = "main";
	desc.shader_name = filename;
	ShaderDataPayload payload;
	payload.data = ReadShader(filename);
	return RHICreateShader(desc, payload);
}

static RenderPipelineState* CreateComputePSO(Shader* cs)
{
	RenderGraphiPipelineStateDesc desc{};
	desc.shaders[ENUM_SHADER_STAGE::Shader_Compute] = cs;
	desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
	desc.raster_state.sample_count = 1;
	return RHICreateRenderPipelineState(desc);
}

static Buffer* CreateStorageBuffer(UInt32 element_count)
{
	BufferDesc desc;
	desc.type = ENUM_BUFFER_TYPE::Storage;
	desc.size = (UInt32)(element_count * sizeof(Float32));
	desc.stride = (UInt32)(element_count * sizeof(Float32));
	Buffer* buffer = RHICreateBuffer(desc);
	void* ptr = RHIMapBuffer(buffer, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memset(ptr, 0, element_count * sizeof(Float32));
	RHIUnmapBuffer(buffer);
	return buffer;
}

static void UploadFloats(Buffer* buffer, CONST Float32* data, UInt32 count)
{
	void* ptr = RHIMapBuffer(buffer, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memcpy(ptr, data, count * sizeof(Float32));
	RHIUnmapBuffer(buffer);
}

static void UploadUInts(Buffer* buffer, CONST UInt32* data, UInt32 count)
{
	void* ptr = RHIMapBuffer(buffer, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memcpy(ptr, data, count * sizeof(UInt32));
	RHIUnmapBuffer(buffer);
}

struct Fluid3DData : public RenderGraphPassDataBase
{
	VIRTUAL ~Fluid3DData() MYDEFAULT;
	VIRTUAL void METHOD(Release)() OVERRIDE {}
};

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderTest, public MXRender::RenderInterface)
#pragma region METHOD
public:
	RenderTest(Window* in_window);
	RenderTest() MYDEFAULT;
	VIRTUAL ~RenderTest() MYDEFAULT;

	VIRTUAL void OnInit_Logic(Application::Window* in_window) OVERRIDE FINAL;
	VIRTUAL void OnShutdown_Logic() OVERRIDE FINAL;
	VIRTUAL void OnUpdate(float dt) OVERRIDE FINAL;
	VIRTUAL void OnRender() OVERRIDE FINAL;

	Window* GetWindow();
protected:
	void METHOD(CreateFluidBuffers)();
	void METHOD(CreateFluidPipelines)();
	void METHOD(CreateFluidBindings)();
	UInt32 METHOD(ParticleGroupCount)() CONST;
	void METHOD(RecordSim)(RHI::CommandList* in_cmd_list);
	void METHOD(RecordClear)(RHI::CommandList* in_cmd_list);
	void METHOD(RecordSplat)(RHI::CommandList* in_cmd_list);
	void METHOD(RecordBlurH)(RHI::CommandList* in_cmd_list);
	void METHOD(RecordBlurV)(RHI::CommandList* in_cmd_list);
	void METHOD(RecordDisplay)(RHI::CommandList* in_cmd_list);
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	Window* window = nullptr;

	// particle storage buffers (vec4-per-particle packed as float arrays)
	RHI::Buffer* pos_buf = nullptr;
	RHI::Buffer* vel_buf = nullptr;
	RHI::Buffer* xstar_a = nullptr;    // predicted positions ping
	RHI::Buffer* xstar_b = nullptr;    // predicted positions pong (solver end state)
	RHI::Buffer* lambda_buf = nullptr;
	RHI::Buffer* nbr_buf = nullptr;    // fixed-size neighbor lists (uint)
	RHI::Buffer* nbr_count = nullptr;  // (uint)
	RHI::Buffer* cell_count = nullptr; // (uint, atomic)
	RHI::Buffer* cell_particles = nullptr; // (uint)
	RHI::Buffer* fp_buf = nullptr;     // per-frame params (64 floats)
	RHI::Buffer* fill_grid = nullptr;  // fill params {value,count} (uint)

	// pipelines (owned by the pipeline state manager, not deleted here)
	RHI::RenderPipelineState* pso_emit = nullptr;
	RHI::RenderPipelineState* pso_predict = nullptr;
	RHI::RenderPipelineState* pso_fill_buf = nullptr;
	RHI::RenderPipelineState* pso_grid_bin = nullptr;
	RHI::RenderPipelineState* pso_neighbor = nullptr;
	RHI::RenderPipelineState* pso_lambda = nullptr;
	RHI::RenderPipelineState* pso_deltapos = nullptr;
	RHI::RenderPipelineState* pso_velocity = nullptr;
	RHI::RenderPipelineState* pso_finalize = nullptr;
	RHI::RenderPipelineState* pso_splat = nullptr;
	RHI::RenderPipelineState* pso_blur_h = nullptr;
	RHI::RenderPipelineState* pso_blur_v = nullptr;
	RHI::RenderPipelineState* pso_display = nullptr;

	// shader resource bindings. Buffer bindings are set once at init;
	// transient texture bindings are re-set every frame in the execute lambdas.
	RHI::ShaderResourceBinding* srb_emit = nullptr;
	RHI::ShaderResourceBinding* srb_predict = nullptr;
	RHI::ShaderResourceBinding* srb_fill_buf = nullptr;
	RHI::ShaderResourceBinding* srb_grid_bin = nullptr;
	RHI::ShaderResourceBinding* srb_neighbor = nullptr;
	RHI::ShaderResourceBinding* srb_lambda[2] = { nullptr, nullptr };   // [0] reads a, [1] reads b
	RHI::ShaderResourceBinding* srb_deltapos[2] = { nullptr, nullptr }; // [0] a->b, [1] b->a
	RHI::ShaderResourceBinding* srb_velocity = nullptr;
	RHI::ShaderResourceBinding* srb_finalize = nullptr;
	RHI::ShaderResourceBinding* srb_splat = nullptr;
	RHI::ShaderResourceBinding* srb_blur_h = nullptr;
	RHI::ShaderResourceBinding* srb_blur_v = nullptr;
	RHI::ShaderResourceBinding* srb_display = nullptr;

	// screen-space ink textures. Retained (created once at init) on purpose:
	// SRB descriptor sets are persistent and referenced by in-flight command
	// buffers (ThreeThread mode keeps two frames in flight), so per-frame
	// SetResource rebinds are unsafe. Bind once at init like Fluid2D.
	RHI::Texture* ink_depth_u = nullptr; // R32U, view-depth bits (imageAtomicMin)
	RHI::Texture* ink_thick_u = nullptr; // R32U, fixed-point thickness (imageAtomicAdd)
	RHI::Texture* ink_f_a = nullptr;     // RGBA16F, blur ping (r=depth g=thickness)
	RHI::Texture* ink_f_b = nullptr;     // RGBA16F, blur pong, sampled by display

	// per-frame CPU state
	UInt32 alive_count = 0;
	UInt32 emit_head = 0;
	UInt32 emit_start = 0;
	UInt32 emit_count = 0;
	UInt32 frame_index = 0;
	Bool firing = false;
	Bool interacting = false;
	Bool has_aim = false;
	glm::vec3 nozzle_pos = glm::vec3(0.0f, 1.7f, 0.9f);
	glm::vec3 emit_vel = glm::vec3(0.0f, 0.0f, -2.0f);
	glm::vec3 aim_target = glm::vec3(0.0f, WATER_REF_Y, 0.0f);
	glm::vec3 interact_pos = glm::vec3(0.0f);
	Float32 cur_dt = 1.0f / 60.0f;
	Float32 time_sec = 0.0f;
	glm::mat4 view_proj = glm::mat4(1.0f);
	glm::mat4 inv_view_proj = glm::mat4(1.0f);
private:

#pragma endregion

MYRENDERER_END_CLASS

void RenderTest::CreateFluidBuffers()
{
	pos_buf = CreateStorageBuffer(MAX_PARTICLES * 4);
	vel_buf = CreateStorageBuffer(MAX_PARTICLES * 4);
	xstar_a = CreateStorageBuffer(MAX_PARTICLES * 4);
	xstar_b = CreateStorageBuffer(MAX_PARTICLES * 4);
	lambda_buf = CreateStorageBuffer(MAX_PARTICLES);
	nbr_buf = CreateStorageBuffer(MAX_PARTICLES * NBR_MAX);
	nbr_count = CreateStorageBuffer(MAX_PARTICLES);
	cell_count = CreateStorageBuffer(GRID_CELLS);
	cell_particles = CreateStorageBuffer(GRID_CELLS * CELL_CAP);
	fp_buf = CreateStorageBuffer(64);
	fill_grid = CreateStorageBuffer(2);

	UInt32 grid_params[2] = { 0u, GRID_CELLS };
	UploadUInts(fill_grid, grid_params, 2);

	// screen-space ink textures (retained, see member comment)
	RHI::TextureDesc ink_uint_desc{};
	ink_uint_desc.width = IRES_W;
	ink_uint_desc.height = IRES_H;
	ink_uint_desc.format = ENUM_TEXTURE_FORMAT::R32U;
	ink_uint_desc.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
	ink_uint_desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_STORAGE;

	// RGBA16F instead of RG32F: linear filtering of 32-bit float formats is
	// not mandated by Vulkan, 16F is (display samples ink_f_b bilinearly)
	RHI::TextureDesc ink_float_desc{};
	ink_float_desc.width = IRES_W;
	ink_float_desc.height = IRES_H;
	ink_float_desc.format = ENUM_TEXTURE_FORMAT::RGBA16F;
	ink_float_desc.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
	ink_float_desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_STORAGE | ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_SHADERRESOURCE;

	ink_depth_u = RHICreateTexture(ink_uint_desc);
	ink_thick_u = RHICreateTexture(ink_uint_desc);
	ink_f_a = RHICreateTexture(ink_float_desc);
	ink_f_b = RHICreateTexture(ink_float_desc);
}

void RenderTest::CreateFluidPipelines()
{
	// Load every shader first, create every PSO, then delete the shaders.
	// PSO descs are hash-cached by shader pointer value: interleaving
	// create/delete could recycle a heap address and collide two pipelines.
	Shader* cs_emit = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_emit.comp.spv");
	Shader* cs_predict = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_predict.comp.spv");
	Shader* cs_fill_buf = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_fill_buf.comp.spv");
	Shader* cs_grid_bin = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_grid_bin.comp.spv");
	Shader* cs_neighbor = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_neighbor.comp.spv");
	Shader* cs_lambda = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_lambda.comp.spv");
	Shader* cs_deltapos = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_deltapos.comp.spv");
	Shader* cs_velocity = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_velocity.comp.spv");
	Shader* cs_finalize = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_finalize.comp.spv");
	Shader* cs_splat = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_splat.comp.spv");
	Shader* cs_blur_h = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_blur_h.comp.spv");
	Shader* cs_blur_v = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Compute, "Shader/fluid3d_blur_v.comp.spv");
	Shader* vs_fullscreen = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Vertex, "Shader/fullscreen.vert.spv");
	Shader* ps_display = LoadShaderFile(ENUM_SHADER_STAGE::Shader_Pixel, "Shader/fluid3d_display.frag.spv");

	pso_emit = CreateComputePSO(cs_emit);
	pso_predict = CreateComputePSO(cs_predict);
	pso_fill_buf = CreateComputePSO(cs_fill_buf);
	pso_grid_bin = CreateComputePSO(cs_grid_bin);
	pso_neighbor = CreateComputePSO(cs_neighbor);
	pso_lambda = CreateComputePSO(cs_lambda);
	pso_deltapos = CreateComputePSO(cs_deltapos);
	pso_velocity = CreateComputePSO(cs_velocity);
	pso_finalize = CreateComputePSO(cs_finalize);
	pso_splat = CreateComputePSO(cs_splat);
	pso_blur_h = CreateComputePSO(cs_blur_h);
	pso_blur_v = CreateComputePSO(cs_blur_v);

	RenderGraphiPipelineStateDesc display_desc;
	display_desc.shaders[ENUM_SHADER_STAGE::Shader_Vertex] = vs_fullscreen;
	display_desc.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = ps_display;
	display_desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
	Vector<Texture*> rtvs = { window->GetViewport()->GetCurrentBackBufferRTV() };
	display_desc.render_targets = rtvs;
	display_desc.depth_stencil_view = window->GetViewport()->GetCurrentBackBufferDSV();
	display_desc.raster_state.sample_count = 1;
	display_desc.blend_state.render_targets.resize(rtvs.size());
	pso_display = RHICreateRenderPipelineState(display_desc);

	delete cs_emit;
	delete cs_predict;
	delete cs_fill_buf;
	delete cs_grid_bin;
	delete cs_neighbor;
	delete cs_lambda;
	delete cs_deltapos;
	delete cs_velocity;
	delete cs_finalize;
	delete cs_splat;
	delete cs_blur_h;
	delete cs_blur_v;
	delete vs_fullscreen;
	delete ps_display;
}

void RenderTest::CreateFluidBindings()
{
	pso_emit->CreateShaderResourceBinding(srb_emit, false);
	srb_emit->SetResource("fp", fp_buf);
	srb_emit->SetResource("pos", pos_buf);
	srb_emit->SetResource("vel", vel_buf);
	srb_emit->FlushDescriptorWrites();

	pso_predict->CreateShaderResourceBinding(srb_predict, false);
	srb_predict->SetResource("fp", fp_buf);
	srb_predict->SetResource("pos", pos_buf);
	srb_predict->SetResource("vel", vel_buf);
	srb_predict->SetResource("xout", xstar_a);
	srb_predict->FlushDescriptorWrites();

	pso_fill_buf->CreateShaderResourceBinding(srb_fill_buf, false);
	srb_fill_buf->SetResource("fx", fill_grid);
	srb_fill_buf->SetResource("dst", cell_count);
	srb_fill_buf->FlushDescriptorWrites();

	pso_grid_bin->CreateShaderResourceBinding(srb_grid_bin, false);
	srb_grid_bin->SetResource("fp", fp_buf);
	srb_grid_bin->SetResource("xin", xstar_a);
	srb_grid_bin->SetResource("cnt", cell_count);
	srb_grid_bin->SetResource("cell", cell_particles);
	srb_grid_bin->FlushDescriptorWrites();

	pso_neighbor->CreateShaderResourceBinding(srb_neighbor, false);
	srb_neighbor->SetResource("fp", fp_buf);
	srb_neighbor->SetResource("xin", xstar_a);
	srb_neighbor->SetResource("cnt", cell_count);
	srb_neighbor->SetResource("cell", cell_particles);
	srb_neighbor->SetResource("nbr", nbr_buf);
	srb_neighbor->SetResource("nc", nbr_count);
	srb_neighbor->FlushDescriptorWrites();

	// lambda reads the current x* buffer: [0] = a (iters 0,2), [1] = b (iter 1)
	for (Int p = 0; p < 2; ++p)
	{
		pso_lambda->CreateShaderResourceBinding(srb_lambda[p], false);
		srb_lambda[p]->SetResource("fp", fp_buf);
		srb_lambda[p]->SetResource("xin", p == 0 ? xstar_a : xstar_b);
		srb_lambda[p]->SetResource("nbr", nbr_buf);
		srb_lambda[p]->SetResource("nc", nbr_count);
		srb_lambda[p]->SetResource("lam", lambda_buf);
		srb_lambda[p]->FlushDescriptorWrites();
	}

	// deltapos ping-pong: [0] a->b, [1] b->a; iters run a->b, b->a, a->b (ends in b)
	for (Int p = 0; p < 2; ++p)
	{
		pso_deltapos->CreateShaderResourceBinding(srb_deltapos[p], false);
		srb_deltapos[p]->SetResource("fp", fp_buf);
		srb_deltapos[p]->SetResource("xin", p == 0 ? xstar_a : xstar_b);
		srb_deltapos[p]->SetResource("lam", lambda_buf);
		srb_deltapos[p]->SetResource("nbr", nbr_buf);
		srb_deltapos[p]->SetResource("nc", nbr_count);
		srb_deltapos[p]->SetResource("xout", p == 0 ? xstar_b : xstar_a);
		srb_deltapos[p]->FlushDescriptorWrites();
	}

	pso_velocity->CreateShaderResourceBinding(srb_velocity, false);
	srb_velocity->SetResource("fp", fp_buf);
	srb_velocity->SetResource("pos", pos_buf);
	srb_velocity->SetResource("xin", xstar_b);
	srb_velocity->SetResource("nbr", nbr_buf);
	srb_velocity->SetResource("nc", nbr_count);
	srb_velocity->SetResource("vel", vel_buf);
	srb_velocity->FlushDescriptorWrites();

	pso_finalize->CreateShaderResourceBinding(srb_finalize, false);
	srb_finalize->SetResource("fp", fp_buf);
	srb_finalize->SetResource("xin", xstar_b);
	srb_finalize->SetResource("pos", pos_buf);
	srb_finalize->SetResource("vel", vel_buf);
	srb_finalize->FlushDescriptorWrites();

	// image bindings: bound ONCE here. Descriptor sets are persistent and the
	// previous frame's command buffer may still be executing (ThreeThread),
	// so runtime SetResource would race with the GPU.
	pso_splat->CreateShaderResourceBinding(srb_splat, false);
	srb_splat->SetResource("fp", fp_buf);
	srb_splat->SetResource("pos", pos_buf);
	srb_splat->SetResource("uimg_depth", ink_depth_u);
	srb_splat->SetResource("uimg_thick", ink_thick_u);
	srb_splat->FlushDescriptorWrites();

	pso_blur_h->CreateShaderResourceBinding(srb_blur_h, false);
	srb_blur_h->SetResource("uimg_depth", ink_depth_u);
	srb_blur_h->SetResource("uimg_thick", ink_thick_u);
	srb_blur_h->SetResource("img_out", ink_f_a);
	srb_blur_h->FlushDescriptorWrites();

	pso_blur_v->CreateShaderResourceBinding(srb_blur_v, false);
	srb_blur_v->SetResource("tex_in", ink_f_a);
	srb_blur_v->SetResource("img_out", ink_f_b);
	srb_blur_v->FlushDescriptorWrites();

	pso_display->CreateShaderResourceBinding(srb_display, false);
	srb_display->SetResource("fp", fp_buf);
	srb_display->SetResource("ink_tex", ink_f_b);
	srb_display->FlushDescriptorWrites();
}

UInt32 RenderTest::ParticleGroupCount() CONST
{
	UInt32 groups = (alive_count + LOCAL_SIZE - 1) / LOCAL_SIZE;
	return groups > 0 ? groups : 1;
}

void RenderTest::RecordSim(RHI::CommandList* in_cmd_list)
{
	// per-frame CPU bookkeeping: ring-buffer emission
	emit_count = firing ? EMIT_PER_FRAME : 0;
	emit_start = emit_head;
	emit_head = (emit_head + emit_count) % MAX_PARTICLES;
	UInt32 new_alive = alive_count + emit_count;
	alive_count = new_alive < MAX_PARTICLES ? new_alive : MAX_PARTICLES;

	// fp layout: [0]=dt [1]=alive [2]=emit_start [3]=emit_count [4-6]=nozzle
	//            [7]=fire [8-10]=emit_vel [11]=seed [12-14]=interact [15]=interact_flag
	//            [16-31]=view_proj (column major) [32-34]=aim [35]=time
	// The sim runs SUBSTEPS times per frame with dt/SUBSTEPS: halving the step
	// halves per-step compression, which PBF turns directly into velocity.
	CONST Int SUBSTEPS = 2;
	Float32 params[64] = {};
	params[0] = cur_dt / (Float32)SUBSTEPS;
	params[1] = (Float32)alive_count;
	params[2] = (Float32)emit_start;
	params[3] = (Float32)emit_count;
	params[4] = nozzle_pos.x;
	params[5] = nozzle_pos.y;
	params[6] = nozzle_pos.z;
	params[7] = firing ? 1.0f : 0.0f;
	params[8] = emit_vel.x;
	params[9] = emit_vel.y;
	params[10] = emit_vel.z;
	params[11] = (Float32)(frame_index & 0xFFFFu);
	params[12] = interact_pos.x;
	params[13] = interact_pos.y;
	params[14] = interact_pos.z;
	params[15] = interacting ? 1.0f : 0.0f;
	std::memcpy(&params[16], &view_proj[0][0], 16 * sizeof(Float32));
	params[32] = aim_target.x;
	params[33] = aim_target.y;
	params[34] = aim_target.z;
	params[35] = time_sec;
	UploadFloats(fp_buf, params, 64);

	auto run_compute = [&](RenderPipelineState* pso, ShaderResourceBinding* srb, UInt32 gx, UInt32 gy, UInt32 gz)
	{
		in_cmd_list->SetComputePipeline(pso);
		in_cmd_list->SetShaderResourceBinding(srb);
		in_cmd_list->Dispatch(gx, gy, gz);
		// write->read for the next consumer, write->write for ping-pong safety;
		// both accumulate into one pipeline barrier flushed by the next Dispatch
		in_cmd_list->ResourceBarrier(ENUM_RESOURCE_STATE::UnorderedAccess, ENUM_RESOURCE_STATE::ShaderResource);
		in_cmd_list->ResourceBarrier(ENUM_RESOURCE_STATE::UnorderedAccess, ENUM_RESOURCE_STATE::UnorderedAccess);
	};

	CONST UInt32 g = ParticleGroupCount();
	CONST UInt32 g_grid = (GRID_CELLS + LOCAL_SIZE - 1) / LOCAL_SIZE;

	if (emit_count > 0)
	{
		run_compute(pso_emit, srb_emit, 1, 1, 1);
	}
	if (alive_count == 0)
	{
		return; // nothing to simulate yet
	}
	for (Int step = 0; step < SUBSTEPS; ++step)
	{
		run_compute(pso_predict, srb_predict, g, 1, 1);
		run_compute(pso_fill_buf, srb_fill_buf, g_grid, 1, 1);
		run_compute(pso_grid_bin, srb_grid_bin, g, 1, 1);
		run_compute(pso_neighbor, srb_neighbor, g, 1, 1);
		for (Int i = 0; i < SOLVER_ITERS; ++i)
		{
			run_compute(pso_lambda, srb_lambda[i & 1], g, 1, 1);
			run_compute(pso_deltapos, srb_deltapos[i & 1], g, 1, 1);
		}
		run_compute(pso_velocity, srb_velocity, g, 1, 1);
		run_compute(pso_finalize, srb_finalize, g, 1, 1);
	}
}

void RenderTest::RecordClear(RHI::CommandList* in_cmd_list)
{
	// frame-head clear via vkCmdClearColorImage (ClearTexture transitions to
	// CopyDest internally). VkClearColorValue is a union: the engine fills
	// float32[], and R32_UINT images read the same bits as uint32 — so +inf
	// (0x7F800000) is exactly the FAR sentinel imageAtomicMin expects.
	CONST Float32 far_sentinel = std::numeric_limits<Float32>::infinity();
	in_cmd_list->ClearTexture(ink_depth_u, { far_sentinel, 0.0f, 0.0f, 0.0f });
	in_cmd_list->ClearTexture(ink_thick_u, { 0.0f, 0.0f, 0.0f, 0.0f });
}

void RenderTest::RecordSplat(RHI::CommandList* in_cmd_list)
{
	// CopyDest (after the clear pass) -> GENERAL for imageAtomic writes
	in_cmd_list->TransitionTextureState(ink_depth_u, ENUM_RESOURCE_STATE::UnorderedAccess);
	in_cmd_list->TransitionTextureState(ink_thick_u, ENUM_RESOURCE_STATE::UnorderedAccess);

	in_cmd_list->SetComputePipeline(pso_splat);
	in_cmd_list->SetShaderResourceBinding(srb_splat);
	in_cmd_list->Dispatch(ParticleGroupCount(), 1, 1);
	in_cmd_list->ResourceBarrier(ENUM_RESOURCE_STATE::UnorderedAccess, ENUM_RESOURCE_STATE::ShaderResource);
	in_cmd_list->ResourceBarrier(ENUM_RESOURCE_STATE::UnorderedAccess, ENUM_RESOURCE_STATE::UnorderedAccess);
}

void RenderTest::RecordBlurH(RHI::CommandList* in_cmd_list)
{
	// frame 0: Undefined -> GENERAL, afterwards SRV -> GENERAL (BlurV read it)
	in_cmd_list->TransitionTextureState(ink_f_a, ENUM_RESOURCE_STATE::UnorderedAccess);

	in_cmd_list->SetComputePipeline(pso_blur_h);
	in_cmd_list->SetShaderResourceBinding(srb_blur_h);
	in_cmd_list->Dispatch((IRES_W + 15) / 16, (IRES_H + 15) / 16, 1);
	in_cmd_list->ResourceBarrier(ENUM_RESOURCE_STATE::UnorderedAccess, ENUM_RESOURCE_STATE::ShaderResource);
	in_cmd_list->ResourceBarrier(ENUM_RESOURCE_STATE::UnorderedAccess, ENUM_RESOURCE_STATE::UnorderedAccess);
}

void RenderTest::RecordBlurV(RHI::CommandList* in_cmd_list)
{
	// f_a: GENERAL -> SHADER_READ_ONLY (sampled via texelFetch)
	in_cmd_list->TransitionTextureState(ink_f_a, ENUM_RESOURCE_STATE::ShaderResource);
	// f_b: frame 0 Undefined -> GENERAL, afterwards SRV -> GENERAL
	in_cmd_list->TransitionTextureState(ink_f_b, ENUM_RESOURCE_STATE::UnorderedAccess);

	in_cmd_list->SetComputePipeline(pso_blur_v);
	in_cmd_list->SetShaderResourceBinding(srb_blur_v);
	in_cmd_list->Dispatch((IRES_W + 15) / 16, (IRES_H + 15) / 16, 1);
	in_cmd_list->ResourceBarrier(ENUM_RESOURCE_STATE::UnorderedAccess, ENUM_RESOURCE_STATE::ShaderResource);
}

void RenderTest::RecordDisplay(RHI::CommandList* in_cmd_list)
{
	// f_b: GENERAL -> SHADER_READ_ONLY for hardware bilinear sampling
	// (SetRenderTarget flushes the pending barrier)
	in_cmd_list->TransitionTextureState(ink_f_b, ENUM_RESOURCE_STATE::ShaderResource);

	Vector<ClearValue> clear_values;
	Vector<Texture*> rtvs = { window->GetViewport()->GetCurrentBackBufferRTV() };
	Texture* dsv = window->GetViewport()->GetCurrentBackBufferDSV();
	for (auto rtv : rtvs)
		clear_values.push_back(rtv->GetTextureDesc().clear_value);
	if (dsv)
		clear_values.push_back(dsv->GetTextureDesc().clear_value);
	in_cmd_list->SetRenderTarget(rtvs, dsv, clear_values, dsv != nullptr);
	in_cmd_list->SetGraphicsPipeline(pso_display);
	in_cmd_list->SetShaderResourceBinding(srb_display);
	DrawAttribute draw_attr;
	draw_attr.vertexCount = 3;
	draw_attr.instanceCount = 1;
	in_cmd_list->Draw(draw_attr);
}

void RenderTest::OnInit_Logic(Application::Window* in_window)
{
	window = in_window;
	std::cout << "Hello Fluid3D" << std::endl;

	RHI::CommandList* cmd_list = RHIGetImmediateCommandList();

	// Step 1: Register external resources (BackBuffer + DepthStencil)
	RHI::Texture* backbuffer_rtv = window->GetViewport()->GetCurrentBackBufferRTV();
	RHI::Texture* backbuffer_dsv = window->GetViewport()->GetCurrentBackBufferDSV();

	RHI::TextureDesc rt_desc = backbuffer_rtv->GetTextureDesc();
	auto* rt_resource = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>(
		"BackBuffer", rt_desc, backbuffer_rtv);

	RenderGraphResource<RHI::TextureDesc, RHI::Texture>* ds_resource = nullptr;
	if (backbuffer_dsv)
	{
		RHI::TextureDesc ds_desc = backbuffer_dsv->GetTextureDesc();
		ds_resource = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>(
			"DepthStencil", ds_desc, backbuffer_dsv);
	}

	// Step 2: fixed oblique top-down camera looking at the pool
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 3.0f, 3.2f), glm::vec3(0.0f, 0.25f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (Float32)IRES_W / (Float32)IRES_H, 0.1f, 100.0f);
	view_proj = proj * view;
	inv_view_proj = glm::inverse(view_proj);

	// Step 3: Create simulation resources
	CreateFluidBuffers();
	CreateFluidPipelines();
	CreateFluidBindings();

	// Step 4: build the 5-pass graph. The screen-space ink textures are
	// retained (created in CreateFluidBuffers); layout transitions are issued
	// manually inside the execute lambdas (RDG only auto-transitions transient
	// resources, and per-frame SRB rebinding of transients is unsafe here).
	auto* ink_depth_res = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>(
		"InkDepthU", ink_depth_u->GetTextureDesc(), ink_depth_u);
	auto* ink_thick_res = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>(
		"InkThickU", ink_thick_u->GetTextureDesc(), ink_thick_u);
	auto* ink_f_a_res = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>(
		"InkBlurA", ink_f_a->GetTextureDesc(), ink_f_a);
	auto* ink_f_b_res = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>(
		"InkBlurB", ink_f_b->GetTextureDesc(), ink_f_b);

	// frame-head clear pass: wipe the splat targets before anything else
	auto* clear_pass = graph.AddRenderPass<Fluid3DData>("Fluid3DClearPass", &graph, cmd_list,
	[&](Fluid3DData& data, RenderGraphPassBuilder& builder, CommandList* in_cmd_list)
	{
		builder.Write(ink_depth_res, ENUM_RESOURCE_STATE::CopyDest);
		builder.Write(ink_thick_res, ENUM_RESOURCE_STATE::CopyDest);
	},
	[=](CONST Fluid3DData& data, CommandList* in_cmd_list)
	{
		this->RecordClear(in_cmd_list);
	});
	clear_pass->SetIsCullable(false);

	auto* sim_pass = graph.AddRenderPass<Fluid3DData>("Fluid3DSimPass", &graph, cmd_list,
	[&](Fluid3DData& data, RenderGraphPassBuilder& builder, CommandList* in_cmd_list)
	{
		// all simulation buffers live outside the graph (manual barriers)
	},
	[=](CONST Fluid3DData& data, CommandList* in_cmd_list)
	{
		this->RecordSim(in_cmd_list);
	});
	sim_pass->SetIsCullable(false);

	auto* splat_pass = graph.AddRenderPass<Fluid3DData>("Fluid3DSplatPass", &graph, cmd_list,
	[&](Fluid3DData& data, RenderGraphPassBuilder& builder, CommandList* in_cmd_list)
	{
		builder.Write(ink_depth_res, ENUM_RESOURCE_STATE::UnorderedAccess);
		builder.Write(ink_thick_res, ENUM_RESOURCE_STATE::UnorderedAccess);
	},
	[=](CONST Fluid3DData& data, CommandList* in_cmd_list)
	{
		this->RecordSplat(in_cmd_list);
	});
	splat_pass->SetIsCullable(false);

	auto* blur_h_pass = graph.AddRenderPass<Fluid3DData>("Fluid3DBlurHPass", &graph, cmd_list,
	[&](Fluid3DData& data, RenderGraphPassBuilder& builder, CommandList* in_cmd_list)
	{
		builder.Read(ink_depth_res, ENUM_RESOURCE_STATE::UnorderedAccess); // imageLoad, stays GENERAL
		builder.Read(ink_thick_res, ENUM_RESOURCE_STATE::UnorderedAccess);
		builder.Write(ink_f_a_res, ENUM_RESOURCE_STATE::UnorderedAccess);
	},
	[=](CONST Fluid3DData& data, CommandList* in_cmd_list)
	{
		this->RecordBlurH(in_cmd_list);
	});
	blur_h_pass->SetIsCullable(false);

	auto* blur_v_pass = graph.AddRenderPass<Fluid3DData>("Fluid3DBlurVPass", &graph, cmd_list,
	[&](Fluid3DData& data, RenderGraphPassBuilder& builder, CommandList* in_cmd_list)
	{
		builder.Read(ink_f_a_res, ENUM_RESOURCE_STATE::ShaderResource); // sampled via texelFetch
		builder.Write(ink_f_b_res, ENUM_RESOURCE_STATE::UnorderedAccess);
	},
	[=](CONST Fluid3DData& data, CommandList* in_cmd_list)
	{
		this->RecordBlurV(in_cmd_list);
	});
	blur_v_pass->SetIsCullable(false);

	auto* display_pass = graph.AddRenderPass<Fluid3DData>("Fluid3DDisplayPass", &graph, cmd_list,
	[&](Fluid3DData& data, RenderGraphPassBuilder& builder, CommandList* in_cmd_list)
	{
		builder.Read(ink_f_b_res, ENUM_RESOURCE_STATE::ShaderResource); // hardware bilinear in frag
		builder.Write(rt_resource);
		if (ds_resource) builder.Write(ds_resource);
	},
	[=](CONST Fluid3DData& data, CommandList* in_cmd_list)
	{
		this->RecordDisplay(in_cmd_list);
	});
	display_pass->SetIsCullable(false);

	graph.Compile();
}

void RenderTest::OnShutdown_Logic()
{
	graph.Release();

	RHI::ShaderResourceBinding* srbs[] = {
		srb_emit, srb_predict, srb_fill_buf, srb_grid_bin, srb_neighbor,
		srb_lambda[0], srb_lambda[1], srb_deltapos[0], srb_deltapos[1],
		srb_velocity, srb_finalize,
		srb_splat, srb_blur_h, srb_blur_v, srb_display };
	for (auto* srb : srbs)
	{
		delete srb;
	}

	RHI::Buffer* buffers[] = {
		pos_buf, vel_buf, xstar_a, xstar_b, lambda_buf, nbr_buf, nbr_count,
		cell_count, cell_particles, fp_buf, fill_grid };
	for (auto* buf : buffers)
	{
		delete buf;
	}

	RHI::Texture* textures[] = { ink_depth_u, ink_thick_u, ink_f_a, ink_f_b };
	for (auto* tex : textures)
	{
		delete tex;
	}
}

void RenderTest::OnUpdate(float dt)
{
	GLFWwindow* glfw_win = window->GetWindow();
	int win_w = 0, win_h = 0;
	glfwGetWindowSize(glfw_win, &win_w, &win_h);
	Float64 cx = 0.0, cy = 0.0;
	glfwGetCursorPos(glfw_win, &cx, &cy);
	firing = glfwGetMouseButton(glfw_win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	interacting = glfwGetMouseButton(glfw_win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

	if (win_w <= 0 || win_h <= 0)
	{
		firing = false;
		interacting = false;
		return;
	}

	// cursor -> NDC -> world ray -> hit on the aim plane y = WATER_REF_Y.
	// ny flips because cursor y is top-down while NDC y is bottom-up.
	Float32 nx = (Float32)(2.0 * cx / (Float64)win_w - 1.0);
	Float32 ny = (Float32)(1.0 - 2.0 * cy / (Float64)win_h);
	glm::vec4 p0h = inv_view_proj * glm::vec4(nx, ny, -1.0f, 1.0f);
	glm::vec4 p1h = inv_view_proj * glm::vec4(nx, ny, 1.0f, 1.0f);
	glm::vec3 p0 = glm::vec3(p0h) / p0h.w;
	glm::vec3 p1 = glm::vec3(p1h) / p1h.w;
	glm::vec3 ray_dir = glm::normalize(p1 - p0);
	if (ray_dir.y < -1e-4f)
	{
		Float32 t = (WATER_REF_Y - p0.y) / ray_dir.y;
		glm::vec3 hit = p0 + ray_dir * t;
		hit.x = glm::clamp(hit.x, -(POOL_X - 0.1f), POOL_X - 0.1f);
		hit.z = glm::clamp(hit.z, -(POOL_Z - 0.1f), POOL_Z - 0.1f);
		hit.y = WATER_REF_Y;
		aim_target = hit;
		has_aim = true;
	}

	// nozzle hovers over the near side, x slides with the aim point
	nozzle_pos = glm::vec3(glm::clamp(aim_target.x * 0.5f, -1.0f, 1.0f), 1.7f, 0.9f);

	// ballistic solve so the jet lands on the aim point:
	// v0 = delta/T + 0.5*g*T*up, T clamped by distance
	glm::vec3 delta = aim_target - nozzle_pos;
	Float32 flight_t = glm::clamp(glm::length(delta) / JET_SPEED, 0.25f, 0.7f);
	emit_vel = delta / flight_t;
	emit_vel.y += 0.5f * GRAVITY * flight_t;

	interact_pos = aim_target;

	cur_dt = glm::clamp(dt, 1.0f / 240.0f, 1.0f / 60.0f);
	time_sec += dt;
	frame_index++;
}

void RenderTest::OnRender()
{
	graph.Execute();
}

RenderTest::RenderTest(Window* in_window) :window(in_window)
{

}

MXRender::Application::Window* RenderTest::GetWindow()
{
	return window;
}

int main()
{
	Window window;
	RenderTest render(&window);
	window.InitWindow();
	window.Run(&render);
	system("pause");

	return 0;
}
