#pragma once
#ifndef _VK_RENDERPASS_
#define _VK_RENDERPASS_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../RenderEnum.h"
#include<memory>
#include<string>
#include "../RenderPass.h"

namespace MXRender { class VK_Shader; }

namespace MXRender { class VK_Viewport; }

namespace MXRender { class VK_GraphicsContext; }

namespace MXRender
{
    struct VKPassCommonInfo:PassOtherInfo
    {
    public:
        std::weak_ptr<VK_GraphicsContext> context;
        VkRenderPass render_pass;
    };

	struct VertexInputDescription {
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

	class PipelineBuilder
	{
	public:
		std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
		VertexInputDescription vertexDescription;
		VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
		VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
		VkViewport _viewport;
		VkRect2D _scissor;
		VkPipelineRasterizationStateCreateInfo _rasterizer;
		VkPipelineColorBlendAttachmentState _colorBlendAttachment;
		VkPipelineMultisampleStateCreateInfo _multisampling;
		VkPipelineLayout _pipelineLayout;
		VkPipelineDepthStencilStateCreateInfo _depthStencil;
		VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
		void clear_vertex_input();

		void set_shaders(VK_Shader* effect);
	};

    class VK_RenderPass:public RenderPass
    {
    protected:
        std::vector<RenderPipelineBase> render_pipeline_array;
        std::weak_ptr<VK_GraphicsContext> cur_context;
        VkRenderPass  render_pass;
        VkPipeline pipeline;
        VkPipelineLayout pipeline_layout;
    public:
        virtual void initialize(const PassInfo& init_info, PassOtherInfo* other_info);
        virtual void post_initialize();
        virtual void set_commonInfo(const PassInfo& init_info);
        virtual void prepare_pass_data();
        virtual void initialize_ui_renderbackend(WindowUI* window_ui);
        VK_RenderPass();
        VK_RenderPass(const PassInfo& init_info);
        virtual ~VK_RenderPass() ;
        virtual VkRenderPass& get_render_pass() ;
        virtual VkPipeline& get_pipeline() ;
    };



}
#endif
