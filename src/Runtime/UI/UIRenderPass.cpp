#include "UIRenderPass.h"
#include "UIRenderer.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderRource.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderViewport.h"
#include "Render/Core/RenderGraphPass.h"
#include "Render/Core/RenderGraphResource.h"
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

void RegisterUIPass(
	Render::RenderGraph* graph,
	Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* bb_resource,
	UIRenderer* renderer,
	RHI::CommandList* cmd_list,
	UInt32 viewport_w, UInt32 viewport_h)
{
	if (!graph || !bb_resource || !renderer) return;

	bool needs_offscreen = renderer->NeedsOffscreen();

	if (!needs_offscreen)
	{
		// === Mode A: direct backbuffer rendering ===
		graph->AddRenderPass<UIPassData>("RmlUIPass", graph, cmd_list,
			[bb_resource](UIPassData& data, Render::RenderGraphPassBuilder& builder, RHI::CommandList* in_cmd_list)
		{
			builder.Write(bb_resource, ENUM_RESOURCE_STATE::RenderTarget);
		},
			[renderer, viewport_h](CONST UIPassData& data, RHI::CommandList* in_cmd_list)
		{
			// render target is set by the pass system (backbuffer)
			renderer->BeginFrame(in_cmd_list);
		});
	}
	else
	{
		// === Mode B: offscreen + composite ===
		// Offscreen color target: RGBA8 at viewport res
		RHI::TextureDesc color_desc;
		color_desc.width = viewport_w;
		color_desc.height = viewport_h;
		color_desc.mip_level = 1;
		color_desc.layer_count = 1;
		color_desc.format = ENUM_TEXTURE_FORMAT::RGBA8;
		color_desc.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
		color_desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_COLOR_ATTACHMENT
			| ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_SHADERRESOURCE;
		color_desc.resource_state = ENUM_RESOURCE_STATE::Undefined;
		color_desc.clear_value = { {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} };

		// Offscreen depth-stencil target: D24S8 at viewport res
		RHI::TextureDesc ds_desc;
		ds_desc.width = viewport_w;
		ds_desc.height = viewport_h;
		ds_desc.mip_level = 1;
		ds_desc.layer_count = 1;
		ds_desc.format = ENUM_TEXTURE_FORMAT::D24S8;
		ds_desc.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
		ds_desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_DEPTH_ATTACHMENT;
		ds_desc.resource_state = ENUM_RESOURCE_STATE::Undefined;
		ds_desc.clear_value = { {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} };

		// Offscreen pass: render UI to offscreen color + DS
		Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* ui_color = nullptr;
		Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* ui_ds = nullptr;

		graph->AddRenderPass<UIPassData>("RmlUIOffscreenPass", graph, cmd_list,
			[&ui_color, &ui_ds, color_desc, ds_desc](UIPassData& data, Render::RenderGraphPassBuilder& builder, RHI::CommandList* in_cmd_list)
		{
			ui_color = builder.Create<Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>>("RmlUI_Color", color_desc);
			ui_ds = builder.Create<Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>>("RmlUI_DS", ds_desc);
			builder.Write(ui_color, ENUM_RESOURCE_STATE::RenderTarget);
			builder.Write(ui_ds, ENUM_RESOURCE_STATE::DepthWrite);
		},
			[renderer, &ui_color, &ui_ds](CONST UIPassData& data, RHI::CommandList* in_cmd_list)
		{
			// Set offscreen render target
			Vector<RHI::Texture*> rtvs = { ui_color->GetActual() };
			Vector<RHI::ClearValue> cvs = {
				{ {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, // color: transparent black
				{ {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} }, // DS: depth=1, stencil=0
			};
			in_cmd_list->SetRenderTarget(rtvs, ui_ds->GetActual(), cvs, true);
			renderer->BeginFrame(in_cmd_list);
		});

		// Composite pass: read offscreen color, write backbuffer
		graph->AddRenderPass<UIPassData>("RmlUICompositePass", graph, cmd_list,
			[bb_resource, &ui_color](UIPassData& data, Render::RenderGraphPassBuilder& builder, RHI::CommandList* in_cmd_list)
		{
			builder.Read(ui_color, ENUM_RESOURCE_STATE::ShaderResource);
			builder.Write(bb_resource, ENUM_RESOURCE_STATE::RenderTarget);
		},
			[bb_resource](CONST UIPassData& data, RHI::CommandList* in_cmd_list)
		{
			// Composite: render fullscreen quad sampling ui_color onto backbuffer
			// v1: use the existing backbuffer render target setup + composite draw
			// The backbuffer is already set as render target by RDG system
		});
	}
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
