#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include "Application/Window.h"
#include "Render/RenderInterface.h"
#include "RHI/RenderRource.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderBuffer.h"
#include "RHI/RenderShader.h"
#include "RHI/RenderPipelineState.h"
#include "RHI/Vulkan/VK_RenderRHI.h"
#include "RHI/Vulkan/VK_CommandBuffer.h"
#include "RHI/Vulkan/VK_Device.h"
#include "RHI/Vulkan/VK_Queue.h"
using namespace MXRender::RHI;
using namespace MXRender::Application;
using namespace MXRender;

// Read SPIR-V binary from file
Vector<UInt32> ReadShaderFile(CONST String& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	CHECK_WITH_LOG(!file.is_open(), " App Error: fail to open shader file! ")
	size_t fileSize = (size_t)file.tellg();
	Vector<UInt32> buffer(fileSize / sizeof(UInt32));
	file.seekg(0); file.read((char*)buffer.data(), fileSize); file.close();
	return std::move(buffer);
}

// XOR MLP weights: 2→4→1, trained to approximate XOR
// Layer 1 (input→hidden, 4 neurons): W0(4x2), B0(4)
// Layer 2 (hidden→output, 1 neuron): W1(1x4), B1(1)
struct XorWeights {
	float w0[8] = {   // 4x2: row-major
		 10.0f,  10.0f,   // neuron 0: responds to (1,1)
		 10.0f, -10.0f,   // neuron 1: responds to (1,0)
		-10.0f,  10.0f,   // neuron 2: responds to (0,1)
		  0.0f,   0.0f    // neuron 3: unused
	};
	float b0[4] = { -15.0f, -5.0f, -5.0f, 0.0f };
	float w1[4] = { -30.0f, 15.0f, 15.0f, 0.0f };
	float b1[1] = { -10.0f };
};

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(NeuralNetworkTest, public MXRender::RenderInterface)
public:
	NeuralNetworkTest(Window* w) : window(w) {}
	~NeuralNetworkTest() MYDEFAULT;

	void BeginRender() OVERRIDE FINAL;
	void EndRender() OVERRIDE FINAL {}
	void BeginFrame() OVERRIDE FINAL {}
	void OnFrame() OVERRIDE FINAL {}
	void EndFrame() OVERRIDE FINAL {}
	Window* GetWindow() { return window; }
protected:
	Window* window;
MYRENDERER_END_CLASS

void NeuralNetworkTest::BeginRender()
{
	std::cout << "=== Vulkan Compute Shader Neural Network ===" << std::endl;

	auto* vk_rhi = static_cast<Vulkan::VulkanRHI*>(g_render_rhi);
	auto* device = vk_rhi->GetDevice();

	// ===== 1. Create compute shader =====
	ShaderDesc cs_desc;
	cs_desc.shader_type = ENUM_SHADER_STAGE::Shader_Compute;
	cs_desc.entry_name = "main";
	ShaderDataPayload cs_payload;
	cs_payload.data = ReadShaderFile("Shader/nn_mlp.comp.spv");
	Shader* cs = RHICreateShader(cs_desc, cs_payload);

	// ===== 2. Create storage buffers =====
	BufferDesc storage_desc;
	storage_desc.type = ENUM_BUFFER_TYPE::Storage;
	storage_desc.stride = 4;

	XorWeights weights;
	void* ptr = nullptr;

	// W0: 8 floats (4x2 matrix)
	storage_desc.size = sizeof(weights.w0);
	Buffer* w0_buf = RHICreateBuffer(storage_desc);
	ptr = RHIMapBuffer(w0_buf, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	memcpy(ptr, weights.w0, sizeof(weights.w0));
	RHIUnmapBuffer(w0_buf);

	// B0: 4 floats
	storage_desc.size = sizeof(weights.b0);
	Buffer* b0_buf = RHICreateBuffer(storage_desc);
	ptr = RHIMapBuffer(b0_buf, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	memcpy(ptr, weights.b0, sizeof(weights.b0));
	RHIUnmapBuffer(b0_buf);

	// W1: 4 floats (1x4 matrix)
	storage_desc.size = sizeof(weights.w1);
	Buffer* w1_buf = RHICreateBuffer(storage_desc);
	ptr = RHIMapBuffer(w1_buf, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	memcpy(ptr, weights.w1, sizeof(weights.w1));
	RHIUnmapBuffer(w1_buf);

	// B1: 1 float
	storage_desc.size = sizeof(weights.b1);
	Buffer* b1_buf = RHICreateBuffer(storage_desc);
	ptr = RHIMapBuffer(b1_buf, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	memcpy(ptr, weights.b1, sizeof(weights.b1));
	RHIUnmapBuffer(b1_buf);

	// Input buffer: 2 floats
	storage_desc.size = 2 * sizeof(float);
	Buffer* input_buf = RHICreateBuffer(storage_desc);

	// Output buffer: 1 float
	storage_desc.size = 1 * sizeof(float);
	Buffer* output_buf = RHICreateBuffer(storage_desc);

	// ===== 3. Create compute pipeline =====
	RenderGraphiPipelineStateDesc pipe_desc{};
	pipe_desc.shaders[ENUM_SHADER_STAGE::Shader_Compute] = cs;
	pipe_desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
	pipe_desc.raster_state.sample_count = 1;
	RenderPipelineState* pipeline = RHICreateRenderPipelineState(pipe_desc);
	delete cs;

	// ===== 4. Create SRB and bind resources =====
	ShaderResourceBinding* srb = nullptr;
	pipeline->CreateShaderResourceBinding(srb, false);
	srb->SetResource("w0", w0_buf);
	srb->SetResource("b0", b0_buf);
	srb->SetResource("w1", w1_buf);
	srb->SetResource("b1", b1_buf);
	srb->SetResource("in_buf", input_buf);
	srb->SetResource("out_buf", output_buf);

	// ===== 5. Run inference for XOR test cases =====
	float test_inputs[4][2] = { {0,0}, {0,1}, {1,0}, {1,1} };

	for (int test = 0; test < 4; ++test)
	{
		// Fill input buffer
		ptr = RHIMapBuffer(input_buf, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
		memcpy(ptr, test_inputs[test], 2 * sizeof(float));
		RHIUnmapBuffer(input_buf);

		// Get compute command buffer
		auto* cmd = device->GetCommandBufferManager()->GetOrCreateCommandBuffer(
			ENUM_QUEUE_TYPE::COMPUTE, false);

		// Record compute dispatch
		cmd->Begin();
		cmd->SetComputePipeline(pipeline);
		cmd->SetShaderResourceBinding(srb);
		cmd->Dispatch(1, 1, 1);
		cmd->End();

		// Submit and wait for completion
		device->GetQueue(ENUM_QUEUE_TYPE::COMPUTE)->Submit(cmd);
		vkDeviceWaitIdle(device->GetDevice());

		// Read back output
		float* output = (float*)RHIMapBuffer(output_buf, ENUM_MAP_TYPE::Read, ENUM_MAP_FLAG::None);
		float result = output[0];
		RHIUnmapBuffer(output_buf);

		std::cout << "  XOR(" << test_inputs[test][0] << ", " << test_inputs[test][1]
				  << ") = " << result << " (expected: " << (int)(test_inputs[test][0] != test_inputs[test][1])
				  << ")" << std::endl;
	}

	// ===== 6. Cleanup =====
	delete srb;
	delete pipeline;
	delete w0_buf; delete b0_buf; delete w1_buf; delete b1_buf;
	delete input_buf; delete output_buf;

	std::cout << "=== Done ===" << std::endl;
}

int main() {
	Window window;
	NeuralNetworkTest render(&window);
	window.InitWindow();
	// Run the neural network test directly (don't enter render loop)
	render.BeginRender();
	system("pause");
	return 0;
}
