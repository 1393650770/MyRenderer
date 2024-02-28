#include<iostream>
#include <fstream>
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
using namespace MXRender::RHI;
using namespace MXRender::Render;
using namespace MXRender::Application;
using namespace MXRender::RHI::Vulkan;
using namespace MXRender;


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

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderTest,public MXRender::RenderInterface)
#pragma region METHOD
public:
	RenderTest(Window* in_window);
	RenderTest() DEFAULT;
	VIRTUAL ~RenderTest() DEFAULT;

	VIRTUAL void BeginRender() OVERRIDE FINAL;
	VIRTUAL void EndRender() OVERRIDE FINAL;
	VIRTUAL void BeginFrame() OVERRIDE FINAL;
	VIRTUAL void OnFrame() OVERRIDE FINAL;
	VIRTUAL void EndFrame() OVERRIDE FINAL;

	Window* GetWindow();
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	RenderGraph graph;
	Window* window;
private:

#pragma endregion

MYRENDERER_END_CLASS

void RenderTest::BeginRender()
{
	std::cout << "Hello Triangle" << std::endl;

	struct TestData
	{
	    Viewport* viewport;
		RenderPipelineState* pipeline_state;
		Texture* output;
	};
	RenderPassDesc renderpass_desc;
	CommandList* cmd_list = RHIGetImmediateCommandList();
	graph.AddRenderPass<TestData>("TestPass",&graph, cmd_list,
	[&](TestData& data, RenderGraphPassBuilder& builder, CommandList* in_cmd_list)
	{
		Shader* vs_shader;
		Shader* ps_shader;
		ShaderDesc shader_desc;
		ShaderDataPayload shader_data;
		RenderGraphiPipelineStateDesc pipeline_state_desc;
		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
		shader_desc.shader_name = "TestVS";
		shader_data.data = ReadShader("Shader/vert.spv");
		ps_shader = RHICreateShader(shader_desc,shader_data);
		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
		shader_desc.shader_name = "TestPS";
		shader_data.data = ReadShader("Shader/frag.spv");
		vs_shader = RHICreateShader(shader_desc, shader_data);
		pipeline_state_desc.shaders[ENUM_SHADER_STAGE::Shader_Vertex] =vs_shader;
		pipeline_state_desc.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = ps_shader;
		pipeline_state_desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		Vector<Texture*> rtvs;
		Texture* dsv;
		rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
		dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();
		pipeline_state_desc.render_targets = rtvs;
		pipeline_state_desc.depth_stencil_view = dsv;
		pipeline_state_desc.raster_state.sample_count = 1;
		pipeline_state_desc.blend_state.render_targets.resize(rtvs.size());
		data.pipeline_state = RHICreateRenderPipelineState(pipeline_state_desc);
	},
	[=](CONST TestData& data, CommandList* in_cmd_list)
	{

		Vector<ClearValue> clear_values;
		Vector<Texture*> rtvs;
		Texture* dsv;
		rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
		dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();
		for (auto rtv : rtvs)
		{
			clear_values.push_back(rtv->GetTextureDesc().clear_value);
		}
		if(dsv)
			clear_values.push_back(dsv->GetTextureDesc().clear_value);
		in_cmd_list->SetRenderTarget(rtvs, dsv, clear_values, dsv != nullptr);
		in_cmd_list->SetGraphicsPipeline(data.pipeline_state);
		DrawAttribute draw_attr;
		draw_attr.vertexCount = 3;
		draw_attr.instanceCount = 1;
		in_cmd_list->Draw(draw_attr);
	});

	graph.Compile();
}

void RenderTest::EndRender()
{
	graph.Release();
}

void RenderTest::BeginFrame()
{

}

void RenderTest::OnFrame()
{
	graph.Execute();
	//RHISubmitCommandList(RHIGetImmediateCommandList());
}

void RenderTest::EndFrame()
{

}

RenderTest::RenderTest(Window* in_window):window(in_window)
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
