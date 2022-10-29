#include "MeshPass.h"
#include "../../RHI/Vulkan/VK_GraphicsContext.h"
#include "../../RHI/Vulkan/VK_Device.h"
#include "../../RHI/Vulkan/VK_Shader.h"
#include "../../RHI/Vulkan/VK_VertexArray.h"
#include "../../RHI/Vulkan/VK_DescriptorSets.h"
#include "../../RHI/Vulkan/VK_Viewport.h"
#include "../../RHI/Vulkan/VK_SwapChain.h"
#include "../DefaultSetting.h"
#include "../../Utils/Singleton.h"
#include "vulkan/vulkan_core.h"
#include "../../RHI/RenderEnum.h"
#include "../../RHI/UniformBuffer.h"
#include "../../RHI/Vulkan/VK_Utils.h"
#include "../../Mesh/MeshBase.h"
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../RHI/Vulkan/VK_Texture.h"
#include <array>
#include "../../Logic/Component/StaticMeshComponent.h"
#include "../../Logic/GameObjectManager.h"
namespace MXRender
{



	void Mesh_RenderPass::setup_descriptorset_layout()
	{
		std::shared_ptr<VK_Device> device = cur_context.lock()->device;

		descriptorset_layout=std::make_shared<VK_DescriptorSetLayout>(device,5);
	
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding cubetextureLayoutBinding{};
		cubetextureLayoutBinding.binding = 1;
		cubetextureLayoutBinding.descriptorCount = 1;
		cubetextureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		cubetextureLayoutBinding.pImmutableSamplers = nullptr;
		cubetextureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		descriptorset_layout->add_bindingdescriptor(0, uboLayoutBinding);

		descriptorset_layout->add_bindingdescriptor(1, cubetextureLayoutBinding);
		descriptorset_layout->compile();
	}

	void Mesh_RenderPass::setup_pipelines()
	{
		std::shared_ptr<VK_Device> device= cur_context.lock()->device;

		VK_Shader cur_shader(device, VK_SHADER_STAGE_VERTEX_BIT,"Shader/mesh_rock_vert.spv","Shader/mesh_rock_frag.spv");

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = cur_shader.shader_modules[ENUM_SHADER_STAGE::Shader_Vertex];
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = cur_shader.shader_modules[ENUM_SHADER_STAGE::Shader_Pixel];
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDescription = SimpleVertex::getBindingDescription();
		auto attributeDescriptions = SimpleVertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;	
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;


		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		//pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pSetLayouts = &descriptorset_layout->get_descriptorset_layout();

		if (vkCreatePipelineLayout(cur_context.lock()->device->device, &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipeline_layout;
		pipelineInfo.renderPass = render_pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(cur_context.lock()->device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}



	void Mesh_RenderPass::setup_uniformbuffer()
	{
		VkDeviceSize bufferSize = sizeof(MVP_Struct);

		uniform_buffers.resize(1);
		uniform_buffers_memory.resize(1);

		for (size_t i = 0; i < 1; i++) {
			VK_Utils::Create_VKBuffer( cur_context.lock()->device,bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers[i], uniform_buffers_memory[i]);
		}
	}

	void Mesh_RenderPass::setup_descriptorpool()
	{
		std::vector<VkDescriptorPoolSize> poolSize(2);
		poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize[0].descriptorCount = static_cast<uint32_t>(cur_context.lock()->get_max_frame_num());
		poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize[1].descriptorCount = static_cast<uint32_t>(cur_context.lock()->get_max_frame_num());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSize.size();
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(3);

		if (vkCreateDescriptorPool(cur_context.lock()->device->device, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void Mesh_RenderPass::setup_descriptorsets()
	{
		
		std::string texture_path="Resource/Texture/viking_room.png";



		cubemap_texture=std::make_shared<VK_Texture>(texture_path);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptor_pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorset_layout->get_descriptorset_layout();
		descriptor_sets.resize(1);
		if (vkAllocateDescriptorSets(cur_context.lock()->device->device, &allocInfo, descriptor_sets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < 1; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniform_buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(MVP_Struct);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = cubemap_texture->textureImageView;
			imageInfo.sampler = cubemap_texture->textureSampler;

			std::vector< VkWriteDescriptorSet> descriptorWrite(2);
			descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[0].dstSet = descriptor_sets[i];
			descriptorWrite[0].dstBinding = 0;
			descriptorWrite[0].dstArrayElement = 0;
			descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite[0].descriptorCount = 1;
			descriptorWrite[0].pBufferInfo = &bufferInfo;

			descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[1].dstSet = descriptor_sets[i];
			descriptorWrite[1].dstBinding = 1;
			descriptorWrite[1].dstArrayElement = 0;
			descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite[1].descriptorCount = 1;
			descriptorWrite[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(cur_context.lock()->device->device, descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
		}
		
	}

	

	void Mesh_RenderPass::update_uniformbuffer()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		MVP_Struct ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), (float)cur_context.lock()->get_swapchain_extent().width / (float)cur_context.lock()->get_swapchain_extent().height, 0.1f, 100.0f);
		
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(cur_context.lock()->device->device, uniform_buffers_memory[0], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(cur_context.lock()->device->device, uniform_buffers_memory[0]);
	}	




	void Mesh_RenderPass::render_mesh(ComponentBase* mesh_component)
	{
		StaticMeshComponent* staticmesh=dynamic_cast<StaticMeshComponent*>(mesh_component);
		if (staticmesh)
		{
			VK_BindMeshInfo bind_mesh_info(vertexBuffer, indexBuffer, vertexBufferMemory, indexBufferMemory);//=new VK_BindMeshInfo(vertexBuffer,indexBuffer,vertexBufferMemory,indexBufferMemory);
			
			bind_mesh_info.context=cur_context.lock().get();
			//bind_mesh_info.index_buffer=indexBuffer;
			//bind_mesh_info.index_buffer_memory=indexBufferMemory;
			//bind_mesh_info.vertex_buffer=vertexBuffer;
			//bind_mesh_info.vertex_buffer_memory=vertexBufferMemory;

			staticmesh->bind_mesh(&bind_mesh_info);

			VK_RenderMeshInfo render_mesh_info(vertexBuffer,indexBuffer);//=new VK_RenderMeshInfo();
			render_mesh_info.context= cur_context.lock().get();

			//render_mesh_info.index_buffer= indexBuffer;
			//render_mesh_info.vertex_buffer= vertexBuffer;
			staticmesh->render_mesh(&render_mesh_info);

			//delete bind_mesh_info;
			//delete render_mesh_info;
		}
	}

	void Mesh_RenderPass::initialize(const PassInfo& init_info, PassOtherInfo* other_info)
	{
		VKPassCommonInfo* vk_info = static_cast<VKPassCommonInfo*>(other_info);
		cur_context = vk_info->context;
		pass_info = init_info;
		render_pass=vk_info->render_pass;

		setup_descriptorset_layout();
		setup_pipelines();

		setup_uniformbuffer();
		setup_descriptorpool();
		setup_descriptorsets();
	}

	void Mesh_RenderPass::post_initialize()
    {
    }

    void Mesh_RenderPass::set_commonInfo(const PassInfo& init_info)
    {
    }

    void Mesh_RenderPass::prepare_pass_data(const GraphicsContext& context)
    {
    }

    void Mesh_RenderPass::initialize_ui_renderbackend()
    {
    }


	void Mesh_RenderPass::draw(GraphicsContext* context)
	{
		update_uniformbuffer();

		VK_GraphicsContext* vk_context=nullptr;
		vk_context= dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context==nullptr)
		{	
			return;
		}

		vkCmdBindPipeline(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport vk_viewport{};
		
		vk_viewport.x = 0.0f;
		vk_viewport.y = 0.0f;
		vk_viewport.width = (float)vk_context->get_swapchain_extent().width;
		vk_viewport.height = (float)vk_context->get_swapchain_extent().height;
		vk_viewport.minDepth = 0.0f;
		vk_viewport.maxDepth = 1.0f;

		vkCmdSetViewport(vk_context->get_cur_command_buffer(), 0, 1, &vk_viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = vk_context->get_swapchain_extent();

		vkCmdSetScissor(vk_context->get_cur_command_buffer(), 0, 1, &scissor);

		vkCmdBindDescriptorSets(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[0], 0, nullptr);

		//vkCmdDrawIndexed(vk_context->get_cur_command_buffer(), static_cast<uint32_t>(mesh_data->indices.size()), 1, 0, 0, 0);
		for(int i=0;i< Singleton<DefaultSetting>::get_instance().gameobject_manager->object_list.size();i++)
			render_mesh(Singleton<DefaultSetting>::get_instance().gameobject_manager->object_list[i].get_staticmesh());
	}



    Mesh_RenderPass::Mesh_RenderPass()
	{

	}

	Mesh_RenderPass::~Mesh_RenderPass()
	{
		vkDestroyPipeline(cur_context.lock()->device->device, pipeline, nullptr);
		vkDestroyPipelineLayout(cur_context.lock()->device->device, pipeline_layout, nullptr);
		//svkDestroyRenderPass(cur_context.lock()->device->device, render_pass, nullptr);



		for (size_t i = 0; i < uniform_buffers.size(); i++) {
			vkDestroyBuffer(cur_context.lock()->device->device, uniform_buffers[i], nullptr);
			vkFreeMemory(cur_context.lock()->device->device, uniform_buffers_memory[i], nullptr);
		}

		vkDestroyDescriptorPool(cur_context.lock()->device->device, descriptor_pool, nullptr);


	}

	VkRenderPass& Mesh_RenderPass::get_render_pass() 
	{
		return render_pass;
	}


	MXRender::Mesh_RenderPass::Mesh_RenderPass(const PassInfo& init_info)
    {
        pass_info = init_info;
    }
}
