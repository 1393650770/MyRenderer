
#include <iostream>
#include <fstream>
#include "VK_Device.h"
#include "VK_DescriptorPool.h"

namespace MXRender
{

	VK_DescriptorPool::VK_DescriptorPool(std::shared_ptr<VK_Device> InDevice, unsigned int InMaxDescriptorSets):Device(InDevice),MaxDescriptorSets(InMaxDescriptorSets)
	{
		const unsigned int LimitMaxUniformBuffers = MaxDescriptorSets * 2;
		const unsigned int LimitMaxSamplers = MaxDescriptorSets / 2;
		const unsigned int LimitMaxCombinedImageSamplers = MaxDescriptorSets * 3;
		const unsigned int LimitMaxUniformTexelBuffers = MaxDescriptorSets / 2;
		const unsigned int LimitMaxStorageTexelBuffers = MaxDescriptorSets / 4;
		const unsigned int LimitMaxStorageBuffers = MaxDescriptorSets / 4;
		const unsigned int LimitMaxStorageImage = MaxDescriptorSets / 4;
		const unsigned int LimitMaxSampledImages = MaxDescriptorSets * 2;
		const unsigned int LimitMaxInputAttachments = MaxDescriptorSets / 16;

		std::vector<VkDescriptorPoolSize> Types(10);
		Types[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		Types[0].descriptorCount = LimitMaxUniformBuffers;

		Types[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		Types[1].descriptorCount = LimitMaxUniformBuffers;


		Types[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		Types[2].descriptorCount = LimitMaxSamplers;


		Types[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		Types[3].descriptorCount = LimitMaxCombinedImageSamplers;


		Types[4].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		Types[4].descriptorCount = LimitMaxSampledImages;


		Types[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		Types[5].descriptorCount = LimitMaxUniformTexelBuffers;


		Types[6].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		Types[6].descriptorCount = LimitMaxStorageTexelBuffers;


		Types[7].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		Types[7].descriptorCount = LimitMaxStorageBuffers;


		Types[8].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		Types[8].descriptorCount = LimitMaxStorageImage;

		Types[9].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		Types[9].descriptorCount = LimitMaxInputAttachments;


		VkDescriptorPoolCreateInfo PoolInfo;
		PoolInfo.poolSizeCount = Types.size();
		PoolInfo.pPoolSizes = Types.data();
		PoolInfo.maxSets = MaxDescriptorSets;

		if (Device.expired() == false&&vkCreateDescriptorPool(Device.lock()->Device, &PoolInfo, nullptr, &DescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	VK_DescriptorPool::~VK_DescriptorPool()
	{
		if (DescriptorPool != VK_NULL_HANDLE&&Device.expired()==false)
		{
			vkDestroyDescriptorPool(Device.lock()->Device, DescriptorPool, nullptr);
		}
	}

	std::weak_ptr<VK_Device> VK_DescriptorPool::GetDevice() const
	{
		return Device;
	}

	unsigned int VK_DescriptorPool::GetMaxDescriptorSets() const
	{
		return MaxDescriptorSets;
	}

}


