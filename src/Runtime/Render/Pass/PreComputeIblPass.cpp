#include "PreComputeIblPass.h"
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

#include "vulkan/vulkan_core.h"

namespace MXRender
{



	void PreComputeIBL_RenderPass::setup_descriptorset_layout()
	{
		std::shared_ptr<VK_Device> device = cur_context.lock()->device;

		descriptorset_layout=std::make_shared<VK_DescriptorSetLayout>(device,5);
	
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		

		descriptorset_layout->add_bindingdescriptor(0, uboLayoutBinding);

		descriptorset_layout->compile();
	}

	void PreComputeIBL_RenderPass::setup_pipelines()
	{
		std::shared_ptr<VK_Device> device= cur_context.lock()->device;

		VK_Shader cur_shader(device, "","","","Shader/pre_compute_ibl_comp.spv");

		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = cur_shader.shader_modules[ENUM_SHADER_STAGE::Shader_Compute];
		computeShaderStageInfo.pName = "main";


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		//pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pSetLayouts = &descriptorset_layout->get_descriptorset_layout();

		if (vkCreatePipelineLayout(cur_context.lock()->device->device, &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

		pipelineInfo.stage = computeShaderStageInfo;

		pipelineInfo.layout = pipeline_layout;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateComputePipelines(cur_context.lock()->device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}



	void PreComputeIBL_RenderPass::setup_buffer()
	{
		input_data=std::vector<float>(1024,1.0f);


		buffers.resize(1);
		buffers_memory.resize(1);

		for (size_t i = 0; i < buffers.size(); i++) {
			VK_Utils::Create_VKBuffer( cur_context.lock()->device, input_data.size()*sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffers[i], buffers_memory[i]);
		}
	}

	void PreComputeIBL_RenderPass::setup_descriptorpool()
	{
		std::vector<VkDescriptorPoolSize> poolSize(1);
		poolSize[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSize[0].descriptorCount = static_cast<uint32_t>(cur_context.lock()->get_max_frame_num());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSize.size();
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(3);

		if (vkCreateDescriptorPool(cur_context.lock()->device->device, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void PreComputeIBL_RenderPass::setup_descriptorsets()
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
			bufferInfo.buffer = buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = input_data.size()*sizeof(float);


			std::vector< VkWriteDescriptorSet> descriptorWrite(1);
			descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[0].dstSet = descriptor_sets[i];
			descriptorWrite[0].dstBinding = 0;
			descriptorWrite[0].dstArrayElement = 0;
			descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite[0].descriptorCount = 1;
			descriptorWrite[0].pBufferInfo = &bufferInfo;

			
			vkUpdateDescriptorSets(cur_context.lock()->device->device, descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
		}
		
	}

	void PreComputeIBL_RenderPass::update_buffer()
	{
		void* data;
		vkMapMemory(cur_context.lock()->device->device, buffers_memory[0], 0, input_data.size()*sizeof(float), 0, &data);
		memcpy(data,input_data.data(),input_data.size() * sizeof(float));
		vkUnmapMemory(cur_context.lock()->device->device, buffers_memory[0]);
	}



	void PreComputeIBL_RenderPass::initialize(const PassInfo& init_info, PassOtherInfo* other_info)
	{
		VKPassCommonInfo* vk_info = static_cast<VKPassCommonInfo*>(other_info);
		cur_context = vk_info->context;
		pass_info = init_info;
		render_pass=vk_info->render_pass;

		setup_descriptorset_layout();
		setup_pipelines();
		setup_buffer();
		setup_descriptorpool();
		setup_descriptorsets();
	}

	void PreComputeIBL_RenderPass::print(GraphicsContext* context)
	{
		VK_GraphicsContext* vk_context = nullptr;
		vk_context = dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context == nullptr)
		{
			return;
		}
		std::vector<float> output_data(1024,0.0f);
		void* data;
		vkMapMemory(vk_context->device->device, buffers_memory[0], 0, input_data.size()*sizeof(float), 0, &data);
		memcpy(output_data.data(), data, output_data.size() * sizeof(float));
		vkUnmapMemory(vk_context->device->device, buffers_memory[0]);

		std::cout << "output data:\n";
		for (size_t i = 0; i < output_data.size(); ++i)
		{
			if (i % 64 == 0 && i != 0) std::cout << '\n';
			std::cout << output_data[i];
		}
	}

	void PreComputeIBL_RenderPass::post_initialize()
    {
    }

    void PreComputeIBL_RenderPass::set_commonInfo(const PassInfo& init_info)
    {
    }

    void PreComputeIBL_RenderPass::prepare_pass_data(const GraphicsContext& context)
    {
    }




	void PreComputeIBL_RenderPass::draw(GraphicsContext* context)
	{
		
		update_buffer();

		VK_GraphicsContext* vk_context=nullptr;
		vk_context= dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context==nullptr)
		{	
			return;
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(vk_context->get_cur_command_buffer(), &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		QueueFamilyIndices indices= vk_context->get_queuefamily();
		if (indices.computeFamily.value()!= indices.graphicsFamily.value())
		{
			VkBufferMemoryBarrier buffer_barrier =
			{
				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				nullptr,
				0,
				VK_ACCESS_SHADER_WRITE_BIT,
				indices.graphicsFamily.value(),
				indices.computeFamily.value(),
				buffers[0],
				0,
				input_data.size()*sizeof(float) };

			vkCmdPipelineBarrier(
				vk_context->get_cur_command_buffer(),
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				0,
				0, nullptr,
				1, &buffer_barrier,
				0, nullptr);
		}


		vkCmdBindPipeline(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);



		vkCmdBindDescriptorSets(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_sets[0], 0, nullptr);


		vkCmdDispatch(vk_context->get_cur_command_buffer(),uint32_t(input_data.size()/256),1,1);
		
		VkBufferMemoryBarrier memory_barrier;
		memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		memory_barrier.buffer = buffers[0];
		memory_barrier.size = input_data.size() * sizeof(float);
		memory_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.pNext=VK_NULL_HANDLE;
		memory_barrier.offset=0;

		vkCmdPipelineBarrier(
			vk_context->get_cur_command_buffer(),
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0, nullptr,
			1, &memory_barrier,
			0, nullptr);

		vkEndCommandBuffer(vk_context->get_cur_command_buffer());

		VkCommandBuffer command_buffers[]{ vk_context->get_cur_command_buffer() };
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = command_buffers;
		vkQueueSubmit(vk_context->computeQueue, 1, &submit_info, VK_NULL_HANDLE);

		if (vkQueueWaitIdle(vk_context->computeQueue) != VK_SUCCESS)
			throw std::runtime_error("failed to wait queue idle!");
	}



    PreComputeIBL_RenderPass::PreComputeIBL_RenderPass()
	{

	}

	PreComputeIBL_RenderPass::~PreComputeIBL_RenderPass()
	{
		vkDestroyPipeline(cur_context.lock()->device->device, pipeline, nullptr);
		vkDestroyPipelineLayout(cur_context.lock()->device->device, pipeline_layout, nullptr);


		for (size_t i = 0; i < buffers.size(); i++) {
			vkDestroyBuffer(cur_context.lock()->device->device, buffers[i], nullptr);
			vkFreeMemory(cur_context.lock()->device->device, buffers_memory[i], nullptr);
		}

		vkDestroyDescriptorPool(cur_context.lock()->device->device, descriptor_pool, nullptr);


	}

	VkRenderPass& PreComputeIBL_RenderPass::get_render_pass() 
	{
		return render_pass;
	}


	MXRender::PreComputeIBL_RenderPass::PreComputeIBL_RenderPass(const PassInfo& init_info)
    {
        pass_info = init_info;
    }
}
