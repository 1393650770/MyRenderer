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
#include "../../Logic/GameObject.h"
#include "../RenderScene.h"
#include "../../Mesh/VK_Mesh.h"
#include "PipelineShaderObject.h"
#include "../../Logic/TaskScheduler.h"
#include "../TextureManager.h"
#include "optick.h"
#include "../GPUDriven.h"
namespace MXRender
{



	void Mesh_RenderPass::setup_descriptorset_layout()
	{
		std::shared_ptr<VK_Device> device = cur_context.lock()->device;
		descriptorset_layout2 = std::make_shared<VK_DescriptorSetLayout>(device, 5);
		descriptorset_layout=std::make_shared<VK_DescriptorSetLayout>(device,5);
	
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		VkDescriptorSetLayoutBinding cameradata_LayoutBinding{};
		cameradata_LayoutBinding.binding = 1;
		cameradata_LayoutBinding.descriptorCount = 1;
		cameradata_LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameradata_LayoutBinding.pImmutableSamplers = nullptr;
		cameradata_LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding cubetextureLayoutBinding{};
		cubetextureLayoutBinding.binding = 0;
		cubetextureLayoutBinding.descriptorCount = 1;
		cubetextureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		cubetextureLayoutBinding.pImmutableSamplers = nullptr;
		cubetextureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		descriptorset_layout->add_bindingdescriptor(0, uboLayoutBinding);
		descriptorset_layout->add_bindingdescriptor(1, cameradata_LayoutBinding);
		descriptorset_layout2->add_bindingdescriptor(0, cubetextureLayoutBinding);
		descriptorset_layout->compile();
		descriptorset_layout2->compile();
	}

	void Mesh_RenderPass::setup_pipelines()
	{
		std::shared_ptr<VK_Device> device= cur_context.lock()->device;

		VK_Shader cur_shader(device, "Shader/mesh_rock_vert.spv","Shader/mesh_rock_frag.spv");

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

		VertexInputDescription input_description = SimpleVertex::get_vertex_description();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(input_description.bindings.size());
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(input_description.attributes.size());
		vertexInputInfo.pVertexBindingDescriptions = input_description.bindings.data();
		vertexInputInfo.pVertexAttributeDescriptions = input_description.attributes.data();


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
		rasterizer.cullMode = VK_CULL_MODE_NONE;
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
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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
		std::vector< VkDescriptorSetLayout> descriptors{
			descriptorset_layout->get_descriptorset_layout(), descriptorset_layout2->get_descriptorset_layout()
		};
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(float);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = descriptors.size();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		pipelineLayoutInfo.pSetLayouts = descriptors.data();

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
		VkDeviceSize bufferSize = sizeof(MVP_Struct)* 1000000;

		uniform_buffers.resize(2);
		uniform_buffers_memory.resize(2);

		for (size_t i = 0; i < uniform_buffers.size(); i++) {
			VK_Utils::Create_VKBuffer( cur_context.lock()->device,bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers[i], uniform_buffers_memory[i]);
		}
	}

	void Mesh_RenderPass::setup_descriptorpool()
	{
		std::vector<VkDescriptorPoolSize> poolSize(2);
		poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
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

		VK_Texture* texture = Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("viking_room", ENUM_TEXTURE_TYPE::ENUM_TYPE_2D, texture_path);

		if (texture==nullptr)
		{
			return;
		}

		std::vector< VkDescriptorSetLayout> descriptors{
	descriptorset_layout->get_descriptorset_layout(), descriptorset_layout2->get_descriptorset_layout()
		};
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptor_pool;
		allocInfo.descriptorSetCount = descriptors.size();
		allocInfo.pSetLayouts = descriptors.data();
		descriptor_sets.resize(descriptors.size());
		if (vkAllocateDescriptorSets(cur_context.lock()->device->device, &allocInfo, descriptor_sets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}


		VkDescriptorBufferInfo mvp_bufferInfo{};
		mvp_bufferInfo.buffer = uniform_buffers[0];
		mvp_bufferInfo.offset = 0;
		mvp_bufferInfo.range = sizeof(MVP_Struct);

		VkDescriptorBufferInfo camera_data_bufferInfo{};
		camera_data_bufferInfo.buffer = uniform_buffers[1];
		camera_data_bufferInfo.offset = 0;
		camera_data_bufferInfo.range = sizeof(CameraData);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture->textureImageView;
		imageInfo.sampler = texture->textureSampler;

		std::vector< VkWriteDescriptorSet> descriptorWrite(3);
		descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[0].dstSet = descriptor_sets[0];
		descriptorWrite[0].dstBinding = 0;
		descriptorWrite[0].dstArrayElement = 0;
		descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorWrite[0].descriptorCount = 1;
		descriptorWrite[0].pBufferInfo = &mvp_bufferInfo;

		descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[1].dstSet = descriptor_sets[1];
		descriptorWrite[1].dstBinding = 0;
		descriptorWrite[1].dstArrayElement = 0;
		descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite[1].descriptorCount = 1;
		descriptorWrite[1].pImageInfo = &imageInfo;

		descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[2].dstSet = descriptor_sets[0];
		descriptorWrite[2].dstBinding = 1;
		descriptorWrite[2].dstArrayElement = 0;
		descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[2].descriptorCount = 1;
		descriptorWrite[2].pBufferInfo = &camera_data_bufferInfo;

		vkUpdateDescriptorSets(cur_context.lock()->device->device, descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
		


		//DescriptorBuilder::begin(cur_context.lock()->material_system.get_descript_pool())
		//.bind_buffer(0, &bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		//	.build(GlobalSet);

		
	}

	






	void Mesh_RenderPass::update_object_uniform(GameObject* game_object, VkPipelineLayout pipeline_layout)
	{
		MVP_Struct ubo{};
		ubo.model = game_object->get_transform()->get_model_matrix();
		ubo.view = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_view_mat();
		ubo.proj = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_projection_mat(); 
		
		ubo.proj[1][1] *= -1;
		

		uint32_t offset = cpu_ubo_buffer.push(ubo);

		vkCmdBindDescriptorSets(cur_context.lock()->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[0], 1, &offset);
	}

	void Mesh_RenderPass::update_camera_uniform()
	{
		CameraData camera_data;
		camera_data.viewPos= Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_position();
		camera_data.viewDir= Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_direction();

		void* data;
		vkMapMemory(cur_context.lock()->device->device, uniform_buffers_memory[1], 0, sizeof(camera_data), 0, &data);
		memcpy(data, &camera_data, sizeof(camera_data));
		vkUnmapMemory(cur_context.lock()->device->device, uniform_buffers_memory[1]);
	}

	void Mesh_RenderPass::render_mesh(ComponentBase* mesh_component)
	{
		StaticMeshComponent* staticmesh=dynamic_cast<StaticMeshComponent*>(mesh_component);
		if (staticmesh&&staticmesh->get_already_load_mesh())
		{

			BindMeshInfo bind_mesh_info;

			bind_mesh_info.context=cur_context.lock().get();


			staticmesh->bind_mesh(&bind_mesh_info);

			RenderMeshInfo render_mesh_info;
			render_mesh_info.context= cur_context.lock().get();


			staticmesh->render_mesh(&render_mesh_info);


		}
	}

	void Mesh_RenderPass::render_mesh(MeshObject* mesh_component, VkDescriptorSet GlobalSet, VkCommandBuffer command_buffer)
	{
		OPTICK_PUSH("Bind")
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_component->material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline);

		viewport.width = cur_context.lock()->get_swapchain_extent().width;
		viewport.height = cur_context.lock()->get_swapchain_extent().height;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		MVP_Struct ubo{};
		ubo.model = mesh_component->transformMatrix;
		ubo.view = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_view_mat();
		ubo.proj = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_projection_mat();
		ubo.proj[1][1] *= -1;

		uint32_t offset= cpu_ubo_buffer.push(ubo);

		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_component->material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout, 0, 1, &GlobalSet, 1, &offset);


		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_component->material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout, 1, 1, &mesh_component->material->pass_sets[MeshpassType::Forward], 0, nullptr);

		VK_Mesh* vk_mesh = dynamic_cast<VK_Mesh*>(mesh_component->mesh);
		if (!vk_mesh) return;
		VkBuffer vertexBuffers[] = { vk_mesh->get_mesh_info().vertex_buffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(command_buffer, vk_mesh->get_mesh_info().index_buffer, 0, VK_INDEX_TYPE_UINT32);
		OPTICK_POP()

		OPTICK_PUSH("Draw")
		vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(vk_mesh->indices.size()), 1, 0, 0, 0);
		OPTICK_POP()
	}

	void Mesh_RenderPass::render_mesh(RenderScene* render_scene,RenderObject* render_object, VkDescriptorSet global_set, VkCommandBuffer command_buffer)
	{
		Material* material= render_scene->get_material(render_object->materialID);
		DrawMesh* mesh=render_scene->get_mesh(render_object->meshID);

		if (!material||!mesh)
		{
			return;
		}

		OPTICK_PUSH("Bind")
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline);

		viewport.width = cur_context.lock()->get_swapchain_extent().width;
		viewport.height = cur_context.lock()->get_swapchain_extent().height;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		MVP_Struct ubo{};
		ubo.model = render_object->transformMatrix;
		ubo.view = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_view_mat();
		ubo.proj = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_projection_mat();
		ubo.proj[1][1] *= -1;

		uint32_t offset = cpu_ubo_buffer.push(ubo);

		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout, 0, 1, &global_set, 1, &offset);


		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout, 1, 1, &material->pass_sets[MeshpassType::Forward], 0, nullptr);

		vkCmdPushConstants(command_buffer, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &material->parameters.z);

		VK_Mesh* vk_mesh = dynamic_cast<VK_Mesh*>(mesh->original);
		if (!vk_mesh) return;
		vk_mesh->init_mesh_info(cur_context.lock().get());
		VkBuffer vertexBuffers[] = { vk_mesh->get_mesh_info().vertex_buffer };
		VkDeviceSize offsets[] = { 0 };
		Singleton<DefaultSetting>::get_instance().mianshu += vk_mesh->indices.size()/3;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(command_buffer, vk_mesh->get_mesh_info().index_buffer, 0, VK_INDEX_TYPE_UINT32);
		OPTICK_POP()

		OPTICK_PUSH("Draw")
		vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(vk_mesh->indices.size()), 1, 0, 0, 0);
		OPTICK_POP()
	}

	
	void Mesh_RenderPass::dispatch_render_mesh(RenderScene* render_scene,unsigned int start_index, unsigned int end_index, VkDescriptorSet GlobalSet)
	{

		OPTICK_EVENT();
		std::weak_ptr<TaskSystem> task_system= Singleton<DefaultSetting>::get_instance().task_system;
		VkCommandBuffer command_buffer= cur_context.lock()->get_cur_threadid_command_buffer(task_system.lock()->thread_pool.get_current_thread_id());
		cur_context.lock()->thread_command_buffer_use_map[command_buffer]=1;

		std::vector< VkClearValue> clearColor(2);
		clearColor[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearColor[1].depthStencil = { 1.0f, 0 };
		cur_context.lock()->renderpass_begin_info_map[render_pass].renderPass=cur_context.lock()->mesh_pass;
		cur_context.lock()->renderpass_begin_info_map[render_pass].clearValueCount =0;
		cur_context.lock()->renderpass_begin_info_map[render_pass].pClearValues = nullptr;
		OPTICK_PUSH("BindPass")
		vkCmdBeginRenderPass(command_buffer, &(cur_context.lock()->renderpass_begin_info_map[render_pass]),VK_SUBPASS_CONTENTS_INLINE);
		OPTICK_POP()

		viewport.width = cur_context.lock()->get_swapchain_extent().width;
		viewport.height = cur_context.lock()->get_swapchain_extent().height;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = cur_context.lock()->get_swapchain_extent();

		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
		
		for (int i = start_index; i< end_index &&i < render_scene->get_renderables_size(); i++)
		{
			auto* it = const_cast<RenderObject*>(&(render_scene->get_renderable_obj(i)));
			render_mesh(render_scene, it, GlobalSet, command_buffer);
		}

		vkCmdEndRenderPass(command_buffer);

		//VK_Utils::Transition_ImageLayout(cur_context, command_buffer, cur_context.lock()->get_cur_swapchain_image(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void Mesh_RenderPass::dispatch_gpudriven_render_mesh(RenderScene* render_scene, unsigned int start_index, unsigned int end_index, VkDescriptorSet global_set,VkDescriptorSet object_data_set)
	{	
		OPTICK_EVENT();
		std::weak_ptr<TaskSystem> task_system = Singleton<DefaultSetting>::get_instance().task_system;
		VkCommandBuffer command_buffer = cur_context.lock()->get_cur_threadid_command_buffer(task_system.lock()->thread_pool.get_current_thread_id());
		cur_context.lock()->thread_command_buffer_use_map[command_buffer] = 1;

		std::vector< VkClearValue> clearColor(2);
		clearColor[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearColor[1].depthStencil = { 1.0f, 0 };
		cur_context.lock()->renderpass_begin_info_map[render_pass].renderPass = cur_context.lock()->mesh_pass;
		cur_context.lock()->renderpass_begin_info_map[render_pass].clearValueCount = 0;
		cur_context.lock()->renderpass_begin_info_map[render_pass].pClearValues = nullptr;
		OPTICK_PUSH("BindPass")
		vkCmdBeginRenderPass(command_buffer, &(cur_context.lock()->renderpass_begin_info_map[render_pass]), VK_SUBPASS_CONTENTS_INLINE);
		OPTICK_POP()

		viewport.width = cur_context.lock()->get_swapchain_extent().width;
		viewport.height = cur_context.lock()->get_swapchain_extent().height;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = cur_context.lock()->get_swapchain_extent();

		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
		bool is_bind_vertex_index = false;

		if (Singleton<DefaultSetting>::get_instance().is_enable_batch)
		{
			int draw_index = 0,i=0;
			for (auto& [k, v] : render_scene->merge_batch)
			{
				if (v.size() <= 0)
				{
					continue;
				}
				if (i<start_index||i >= end_index || i >= render_scene->merge_batch.size())
				{
					i++;
					draw_index += v.size();
					continue;
				}
				i++;
				const RenderObject& render_object = render_scene->get_renderable_obj(v[0].handle);
				Material* material = render_scene->get_material(render_object.materialID);
				DrawMesh* mesh = render_scene->get_mesh(render_object.meshID);
				if (!material || !mesh)
				{
					continue;
				}

				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline);


				MVP_Struct ubo{};
				ubo.model = render_object.transformMatrix;
				ubo.view = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_view_mat();
				ubo.proj = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_projection_mat();
				ubo.proj[1][1] *= -1;

				uint32_t offset = cpu_ubo_buffer.push(ubo);



				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout, 0, 1, &(descriptor_sets[0]), 1, &offset);

				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), 1, 1, &(material->pass_sets[MeshpassType::Forward]), 0, nullptr);

				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), 2, 1, &object_data_set, 0, nullptr);

				vkCmdPushConstants(command_buffer, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &material->parameters.z);

				if (is_bind_vertex_index == false)
				{

					VkBuffer vertexBuffers[] = { render_scene->merged_vertex_buffer._buffer };
					VkDeviceSize offsets[] = { 0 };

					vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

					vkCmdBindIndexBuffer(command_buffer, render_scene->merged_index_buffer._buffer, 0, VK_INDEX_TYPE_UINT32);
				}

				vkCmdDrawIndexedIndirect(command_buffer, render_scene->gpu_driven->drawIndirectBuffer._buffer, draw_index * sizeof(GPUIndirectObject), v.size(), sizeof(GPUIndirectObject));

				draw_index += v.size();
			}
		}
		else
		{ 
			for (int i = start_index; i < end_index && i < render_scene->get_renderables_size(); i++)
			{
				const RenderObject& render_object = render_scene->get_renderable_obj(i);
				Material* material = render_scene->get_material(render_object.materialID);
				DrawMesh* mesh = render_scene->get_mesh(render_object.meshID);
				if (!material || !mesh)
				{
					continue;
				}

				OPTICK_PUSH("Bind")
				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline);


				MVP_Struct ubo{};
				ubo.model = render_object.transformMatrix;
				ubo.view = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_view_mat();
				ubo.proj = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_projection_mat();
				ubo.proj[1][1] *= -1;

				uint32_t offset = cpu_ubo_buffer.push(ubo);



				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout, 0, 1, &(global_set), 1, &offset);

				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), 1, 1, &(material->pass_sets[MeshpassType::Forward]), 0, nullptr);

				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), 2, 1, &object_data_set, 0, nullptr);

				vkCmdPushConstants(command_buffer, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &material->parameters.z);

				OPTICK_POP()

				VK_Mesh* vk_mesh = dynamic_cast<VK_Mesh*>(mesh->original);
				if (vk_mesh)
				{
				

					VkBuffer vertexBuffers[] = { vk_mesh->get_mesh_info().vertex_buffer };
					VkDeviceSize offsets[] = { 0 };

					vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

					vkCmdBindIndexBuffer(command_buffer, vk_mesh->get_mesh_info().index_buffer, 0, VK_INDEX_TYPE_UINT32);
					OPTICK_PUSH("Draw")
					vkCmdDrawIndexedIndirect(command_buffer, render_scene->gpu_driven->drawIndirectBuffer._buffer, i * sizeof(GPUIndirectObject), 1, sizeof(GPUIndirectObject));
					OPTICK_POP()
				}
			}
		}
		vkCmdEndRenderPass(command_buffer);
	}

	void Mesh_RenderPass::draw_gpudriven(GraphicsContext* context, RenderScene* render_scene /*= nullptr*/)
	{

		if (Singleton<DefaultSetting>::get_instance().is_enable_gpu_driven)
		{
			cpu_ubo_buffer.reset();

			VK_GraphicsContext* vk_context = nullptr;
			vk_context = dynamic_cast<VK_GraphicsContext*>(context);
			if (vk_context == nullptr)
			{
				return;
			}

			if (!render_scene)
			{
				return;
			}

			viewport.width = cur_context.lock()->get_swapchain_extent().width;
			viewport.height = cur_context.lock()->get_swapchain_extent().height;
			vkCmdSetViewport(vk_context->get_cur_command_buffer(), 0, 1, &viewport);


			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = vk_context->get_swapchain_extent();

			vkCmdSetScissor(vk_context->get_cur_command_buffer(), 0, 1, &scissor);

			update_camera_uniform();

			VkDescriptorSet object_data_set = VK_NULL_HANDLE;
			VkDescriptorBufferInfo objectData = render_scene->gpu_driven->objectDataBuffer.get_info();

			VkDescriptorBufferInfo instanceIdMapData = render_scene->gpu_driven->instanceIdMapBuffer.get_info();
			auto builder = DescriptorBuilder::begin(Singleton<DefaultSetting>::get_instance().material_system->get_descriptorlayout_cache(), Singleton<DefaultSetting>::get_instance().material_system->get_descript_temp_pool());
			builder.bind_buffer(0, &objectData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
			builder.bind_buffer(1, &instanceIdMapData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
			builder.build(object_data_set);

			if (Singleton<DefaultSetting>::get_instance().is_enable_dispatch)
			{
				unsigned int thread_num = Singleton<DefaultSetting>::get_instance().task_system->thread_pool.get_max_thread_num();
				unsigned int clip_size; 
				if (Singleton<DefaultSetting>::get_instance().is_enable_batch)
				{
					clip_size = render_scene->merge_batch.size() / thread_num;
				}
				else
				{
					clip_size = render_scene->get_renderables_size() / thread_num;
				}

				for (unsigned int i = 0; i < thread_num; i++)
				{

					auto fut = Singleton<DefaultSetting>::get_instance().task_system->thread_pool.submit_message(&Mesh_RenderPass::dispatch_gpudriven_render_mesh, this, render_scene, clip_size * i, clip_size * (i + 1), descriptor_sets[0], object_data_set);
					vk_context->fut_que.push(std::move(fut));
				}
				
				//vk_context->execute_all_threadid_command_buffer();
			}
			else
			{ 
				VkCommandBuffer command_buffer = cur_context.lock()->get_cur_command_buffer();
				bool is_bind_vertex_index=false;
				if (Singleton<DefaultSetting>::get_instance().is_enable_batch)
				{
					OPTICK_PUSH("Batch_Draw")
					int index=0;
					for (auto& [k,v]: render_scene->merge_batch)
					{
						if (v.size()<=0)
						{
							continue;
						}
						const RenderObject& render_object = render_scene->get_renderable_obj(v[0].handle);
						Material* material = render_scene->get_material(render_object.materialID);
						DrawMesh* mesh = render_scene->get_mesh(render_object.meshID);
						if (!material || !mesh)
						{
							continue;
						}

						vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline);


						MVP_Struct ubo{};
						ubo.model = render_object.transformMatrix;
						ubo.view = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_view_mat();
						ubo.proj = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_projection_mat();
						ubo.proj[1][1] *= -1;

						uint32_t offset = cpu_ubo_buffer.push(ubo);



						vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout, 0, 1, &(descriptor_sets[0]), 1, &offset);

						vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), 1, 1, &(material->pass_sets[MeshpassType::Forward]), 0, nullptr);

						vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), 2, 1, &object_data_set, 0, nullptr);

						vkCmdPushConstants(command_buffer, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &material->parameters.z);

						if (is_bind_vertex_index == false)
						{

							VkBuffer vertexBuffers[] = { render_scene->merged_vertex_buffer._buffer };
							VkDeviceSize offsets[] = { 0 };

							vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

							vkCmdBindIndexBuffer(command_buffer, render_scene->merged_index_buffer._buffer, 0, VK_INDEX_TYPE_UINT32);
						}

						vkCmdDrawIndexedIndirect(command_buffer, render_scene->gpu_driven->drawIndirectBuffer._buffer, index * sizeof(GPUIndirectObject), v.size(), sizeof(GPUIndirectObject));
						index +=v.size();
					}
					OPTICK_POP()
				}
				else
				{ 
					OPTICK_PUSH("All_Draw")
					for (int i = 0;  i < render_scene->get_renderables_size(); i++)
					{
					
						const RenderObject& render_object = render_scene->get_renderable_obj(i);
						Material* material = render_scene->get_material(render_object.materialID);
						DrawMesh* mesh = render_scene->get_mesh(render_object.meshID);
						if (!material || !mesh)
						{
							continue;
						}

						vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline);


						MVP_Struct ubo{};
						ubo.model = render_object.transformMatrix;
						ubo.view = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_view_mat();
						ubo.proj = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_projection_mat();
						ubo.proj[1][1] *= -1;

						uint32_t offset = cpu_ubo_buffer.push(ubo);



						vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout, 0, 1, &(descriptor_sets[0]), 1, &offset);

						vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), 1, 1, &(material->pass_sets[MeshpassType::Forward]), 0, nullptr);

						vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), 2, 1, &object_data_set, 0, nullptr);

						vkCmdPushConstants(command_buffer, (material->pass_pso->pass_pso[MeshpassType::Forward]->pipeline_layout), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &material->parameters.z);
						if (Singleton<DefaultSetting>::get_instance().is_enable_batch)
						{
						
							if (is_bind_vertex_index == false)
							{

								VkBuffer vertexBuffers[] = { render_scene->merged_vertex_buffer._buffer };
								VkDeviceSize offsets[] = { 0 };

								vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

								vkCmdBindIndexBuffer(command_buffer, render_scene->merged_index_buffer._buffer, 0, VK_INDEX_TYPE_UINT32);
							}
						}
						else
						{
							VK_Mesh* vk_mesh = dynamic_cast<VK_Mesh*>(mesh->original);
							if (vk_mesh)
							{


								VkBuffer vertexBuffers[] = { vk_mesh->get_mesh_info().vertex_buffer };
								VkDeviceSize offsets[] = { 0 };

								vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

								vkCmdBindIndexBuffer(command_buffer, vk_mesh->get_mesh_info().index_buffer, 0, VK_INDEX_TYPE_UINT32);
								OPTICK_PUSH("Draw")
							}
						}
		
						
						vkCmdDrawIndexedIndirect(command_buffer, render_scene->gpu_driven->drawIndirectBuffer._buffer, i * sizeof(GPUIndirectObject), 1, sizeof(GPUIndirectObject));
						
					}
					OPTICK_POP()
				}
			}
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

		cpu_ubo_buffer.init(cur_context.lock().get(), uniform_buffers_memory[0]);

		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = cur_context.lock()->get_swapchain_extent().width;
		viewport.height = cur_context.lock()->get_swapchain_extent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
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




	void Mesh_RenderPass::draw(GraphicsContext* context, RenderScene* render_scene)
	{
		//update_uniformbuffer();

		cpu_ubo_buffer.reset();

		VK_GraphicsContext* vk_context=nullptr;
		vk_context= dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context==nullptr)
		{	
			return;
		}

		if (!render_scene)
		{
			return;
		}
		if (Singleton<DefaultSetting>::get_instance().is_enable_gpu_driven)
		{
			return draw_gpudriven(context,render_scene);
		}
		

		vkCmdBindPipeline(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		viewport.width = cur_context.lock()->get_swapchain_extent().width;
		viewport.height = cur_context.lock()->get_swapchain_extent().height;
		vkCmdSetViewport(vk_context->get_cur_command_buffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = vk_context->get_swapchain_extent();

		vkCmdSetScissor(vk_context->get_cur_command_buffer(), 0, 1, &scissor);

		vkCmdBindDescriptorSets(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1, 1, &descriptor_sets[1], 0, nullptr);

		update_camera_uniform();


		if (Singleton<DefaultSetting>::get_instance().is_enable_dispatch)
		{
			unsigned int thread_num = Singleton<DefaultSetting>::get_instance().task_system->thread_pool.get_max_thread_num();
			unsigned int clip_size= Singleton<DefaultSetting>::get_instance().gameobject_manager->prefab_renderables.size()/thread_num;

			for(unsigned int i=0;i< thread_num;i++)
			{ 
				
				auto fut= Singleton<DefaultSetting>::get_instance().task_system->thread_pool.submit_message(&Mesh_RenderPass::dispatch_render_mesh,this,render_scene, clip_size*i, clip_size*(i+1), descriptor_sets[0]);
				vk_context->fut_que.push(std::move(fut));
			}
			//vk_context->execute_all_threadid_command_buffer();
		}
		else
		{
			for (int i = 0; i < render_scene->get_renderables_size(); i++)
			{
				auto* it=const_cast<RenderObject*>(&(render_scene->get_renderable_obj(i)));
				render_mesh(render_scene,it, descriptor_sets[0], cur_context.lock()->get_cur_command_buffer());
				//render_mesh(&(Singleton<DefaultSetting>::get_instance().gameobject_manager->prefab_renderables[i]), descriptor_sets[0], cur_context.lock()->get_cur_command_buffer());
			}
		}
	}



    Mesh_RenderPass::Mesh_RenderPass()
	{

	}

	Mesh_RenderPass::~Mesh_RenderPass()
	{
		cpu_ubo_buffer.shutdown(cur_context.lock().get(),uniform_buffers_memory[0]);
		vkDestroyPipeline(cur_context.lock()->device->device, pipeline, nullptr);
		vkDestroyPipelineLayout(cur_context.lock()->device->device, pipeline_layout, nullptr);
		//svkDestroyRenderPass(cur_context.lock()->device->device, render_pass, nullptr);



		for (size_t i = 0; i < uniform_buffers.size(); i++) {
			vkDestroyBuffer(cur_context.lock()->device->device, uniform_buffers[i], nullptr);
			vkFreeMemory(cur_context.lock()->device->device, uniform_buffers_memory[i], nullptr);
		}

		//vkFreeDescriptorSets(cur_context.lock()->device->device,descriptor_pool,descriptor_sets.size(),descriptor_sets.data());

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

	void DynamicCPUUniformBuffer::init(VK_GraphicsContext* context, VkDeviceMemory gpu_memory)
	{
		align = context->device->gpu_props.limits.minUniformBufferOffsetAlignment;
		currentOffset = 0;

		vkMapMemory(context->device->device, gpu_memory, 0, VK_WHOLE_SIZE, 0, &mapped);
	}

	void DynamicCPUUniformBuffer::shutdown(VK_GraphicsContext* context, VkDeviceMemory gpu_memory)
	{
		vkUnmapMemory(context->device->device, gpu_memory);
	}

	uint32_t DynamicCPUUniformBuffer::push(void* data, size_t size)
	{
		uint32_t offset = currentOffset;
		char* target = (char*)mapped;
		target += currentOffset;
		memcpy(target, data, size);
		currentOffset += static_cast<uint32_t>(size);
		currentOffset = pad_uniform_buffer_size(currentOffset);

		return offset;
	}

	uint32_t DynamicCPUUniformBuffer::update(void* data, size_t size, uint32_t offset)
	{
		char* target = (char*)mapped;
		target += offset;
		memcpy(target, data, size);
		return offset;
	}

	void DynamicCPUUniformBuffer::reset()
	{
		currentOffset = 0;
	}

	uint32_t DynamicCPUUniformBuffer::pad_uniform_buffer_size(uint32_t originalSize)
	{
		size_t minUboAlignment = align;
		size_t alignedSize = originalSize;
		if (minUboAlignment > 0) {
			alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		return static_cast<uint32_t>(alignedSize);
	}

}
