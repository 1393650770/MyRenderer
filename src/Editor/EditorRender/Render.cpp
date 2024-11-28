#include "Render.h"

#include "Render/Core/RenderGraphPass.h"
#include "RHI/RenderPipelineState.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderShader.h"
#include "Asset/TextureAsset.h"
#include <iostream>
#include <fstream>
#include "Application/Window.h"
#include "RHI/RenderViewport.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderRource.h"
#include "RHI/RenderBuffer.h"
#include "RHI/RenderUtils.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)


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

void EditorRenderPipeline::BeginRender()
{
	std::cout << "Hello Editor" << std::endl;
	editor_ui.Init(window);
	RHI::CommandList* cmd_list = RHIGetImmediateCommandList();

	struct ClearPassData :public  Render::RenderGraphPassDataBase
	{
		VIRTUAL ~ClearPassData()
		{
			Release();
		}
		void Release()
		{

		}
	};
	graph.AddRenderPass<ClearPassData>("ClearPass", &graph, cmd_list,
		[&](ClearPassData& data, Render::RenderGraphPassBuilder& builder, RHI::CommandList* in_cmd_list)
	{

	},
		[=](CONST ClearPassData& data, RHI::CommandList* in_cmd_list)
	{

		Vector<RHI::ClearValue> clear_values;
		Vector<RHI::Texture*> rtvs;
		RHI::Texture* dsv;
		rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
		dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();
		for (auto rtv : rtvs)
		{
			clear_values.push_back(rtv->GetTextureDesc().clear_value);
		}
		if (dsv)
			clear_values.push_back(dsv->GetTextureDesc().clear_value);
		in_cmd_list->SetRenderTarget(rtvs, dsv, clear_values, dsv != nullptr);
		for (auto rtv : rtvs)
		{
			in_cmd_list->ClearTexture(rtv);
		}
		if (dsv)
			in_cmd_list->ClearTexture(dsv);

	});


	struct TestData :public  Render::RenderGraphPassDataBase
	{

		RHI::RenderPipelineState* pipeline_state = nullptr;
		RHI::ShaderResourceBinding* srb = nullptr;
		Asset::TextureAsset* bind_texture = nullptr;
		RHI::Buffer* const_buffer = nullptr;
		VIRTUAL ~TestData()
		{
			Release();
		}
		void Release()
		{
			if (srb)
			{
				delete srb;
				srb = nullptr;
			}
			if (bind_texture)
			{
				delete bind_texture;
				bind_texture = nullptr;
			}
			if (const_buffer)
			{
				delete const_buffer;
				const_buffer = nullptr;
			}
		}
	};

	graph.AddRenderPass<TestData>("TestPass", &graph, cmd_list,
		[&](TestData& data, Render::RenderGraphPassBuilder& builder, RHI::CommandList* in_cmd_list)
	{
		RHI::Shader* vs_shader;
		RHI::Shader* ps_shader;
		RHI::ShaderDesc shader_desc;
		RHI::ShaderDataPayload shader_data;
		RHI::RenderGraphiPipelineStateDesc pipeline_state_desc;
		RHI::TextureDesc texture_desc;
		RHI::TextureDataPayload texture_data;
		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
		shader_desc.shader_name = "TestVS";
		shader_data.data = ReadShader("Shader/texture_test.vert.spv");
		vs_shader = RHICreateShader(shader_desc, shader_data);

		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
		shader_desc.shader_name = "TestPS";
		shader_data.data = ReadShader("Shader/stipple_transparency_test.frag.spv");
		ps_shader = RHICreateShader(shader_desc, shader_data);

		pipeline_state_desc.shaders[ENUM_SHADER_STAGE::Shader_Vertex] = vs_shader;
		pipeline_state_desc.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = ps_shader;
		pipeline_state_desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		Vector<RHI::Texture*> rtvs;
		RHI::Texture* dsv;
		rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
		dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();
		pipeline_state_desc.render_targets = rtvs;
		pipeline_state_desc.depth_stencil_view = dsv;
		pipeline_state_desc.raster_state.sample_count = 1;
		pipeline_state_desc.blend_state.render_targets.resize(rtvs.size());

		data.pipeline_state = RHICreateRenderPipelineState(pipeline_state_desc);
		data.pipeline_state->CreateShaderResourceBinding(data.srb, true);

		data.bind_texture = new Asset::TextureAsset("Texture/pbr_stone/pbr_stone_aorm.dds");
		delete vs_shader;
		delete ps_shader;
		RHI::BufferDesc bufferdesc;
		bufferdesc.size = sizeof(float);
		bufferdesc.stride = sizeof(float);
		bufferdesc.type = ENUM_BUFFER_TYPE::Uniform;
		data.const_buffer = RHICreateBuffer(bufferdesc);
		data.srb->SetResource("constants", data.const_buffer);
	},
		[=](CONST TestData& data, RHI::CommandList* in_cmd_list)
	{
		if (data.bind_texture->GetTexture())
		{
			Vector<RHI::ClearValue> clear_values;
			Vector<RHI::Texture*> rtvs;
			RHI::Texture* dsv;
			rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
			dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();
			for (auto rtv : rtvs)
			{
				clear_values.push_back(rtv->GetTextureDesc().clear_value);
			}
			if (dsv)
				clear_values.push_back(dsv->GetTextureDesc().clear_value);
			in_cmd_list->SetRenderTarget(rtvs, dsv, clear_values, dsv != nullptr);
			in_cmd_list->SetGraphicsPipeline(data.pipeline_state);
			data.srb->SetResource("basecolor_sampler", data.bind_texture->GetTexture());

			struct Constant
			{
				float z = 0.0f;
			};
			{
				RHI::MapHelper<Constant> map_helper(data.const_buffer, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::Discard);
				map_helper->z = 1.0f;
			}

			in_cmd_list->SetShaderResourceBinding(data.srb);
			DrawAttribute draw_attr;
			draw_attr.vertexCount = 6;
			draw_attr.instanceCount = 1;
			in_cmd_list->Draw(draw_attr);
		}
	});

	editor_ui.AddPass(&graph);
	graph.Compile();
}

void EditorRenderPipeline::EndRender()
{
	editor_ui.Release();
	graph.Release();
}

void EditorRenderPipeline::BeginFrame()
{

}

void EditorRenderPipeline::OnFrame()
{
	graph.Execute();
}

void EditorRenderPipeline::EndFrame()
{

}

EditorRenderPipeline::EditorRenderPipeline(MXRender::Application::Window* in_window) :window(in_window)
{

}

MXRender::Application::Window* EditorRenderPipeline::GetWindow()
{
	return window;

}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE