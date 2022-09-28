
#include "VK_DescriptorSets.h"
#include <iostream>
#include <fstream>
#include "VK_Device.h"


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

	bool VK_DescriptorPool::allocateDescriptorSet(VkDescriptorSetLayout Layout, VkDescriptorSet& OutSet)
	{
		if (DescriptorPool == VK_NULL_HANDLE|| Device.expired())
		{
			return false;
		}
		VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo;
		DescriptorSetAllocateInfo.descriptorPool = DescriptorPool;
		DescriptorSetAllocateInfo.descriptorSetCount = 1;
		DescriptorSetAllocateInfo.pSetLayouts = &Layout;

		return vkAllocateDescriptorSets(Device.lock()->Device, &DescriptorSetAllocateInfo, &OutSet)== VK_SUCCESS;

	}

	bool VK_DescriptorPool::allocateDescriptorSets(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet& OutSets)
	{
		if (DescriptorPool == VK_NULL_HANDLE || Device.expired())
		{
			return false;
		}
		VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = InDescriptorSetAllocateInfo;
		DescriptorSetAllocateInfo.descriptorPool = DescriptorPool;
		return vkAllocateDescriptorSets(Device.lock()->Device, &DescriptorSetAllocateInfo, &OutSets) == VK_SUCCESS;;
	}

	std::weak_ptr<VK_Device> VK_DescriptorPool::getDevice() const
	{
		return Device;
	}

	unsigned int VK_DescriptorPool::getMaxDescriptorSets() const
	{
		return MaxDescriptorSets;
	}

	const VkDescriptorPool& VK_DescriptorPool::getDescriptorPool()
	{
		return DescriptorPool;
	}


	VK_DescriptorSetLayout::VK_DescriptorSetLayout(std::shared_ptr<VK_Device> InDevice, unsigned int InMaxBindings):Device(InDevice),MaxBindings(InMaxBindings)
	{
		
	}

	VK_DescriptorSetLayout::~VK_DescriptorSetLayout()
	{
		if (DescriptorSetLayout == VK_NULL_HANDLE || Device.expired())
		{
			return ;
		}
		vkDestroyDescriptorSetLayout(Device.lock()->Device, DescriptorSetLayout, nullptr);
	}

	void VK_DescriptorSetLayout::addBindingDescriptor(unsigned int DescriptorSetIndex, const VkDescriptorSetLayoutBinding& BindingDescriptor)
	{
		if (DescriptorSetIndex >= MaxBindings)
		{
			return;
		}
		if (DescriptorSetIndex > UboLayoutBindingArray.size())
		{
			while(DescriptorSetIndex > UboLayoutBindingArray.size())
				UboLayoutBindingArray.push_back(VkDescriptorSetLayoutBinding());
		}
		VkDescriptorSetLayoutBinding& Binding= UboLayoutBindingArray[DescriptorSetIndex];
		Binding.binding= BindingDescriptor.binding;
		Binding.descriptorCount = BindingDescriptor.descriptorCount;
		Binding.descriptorType = BindingDescriptor.descriptorType;
		Binding.pImmutableSamplers = BindingDescriptor.pImmutableSamplers;
		Binding.stageFlags = BindingDescriptor.stageFlags;
	}

	bool VK_DescriptorSetLayout::compile()
	{
		if ( Device.expired())
		{
			return false;
		}
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = UboLayoutBindingArray.size();
		layoutInfo.pBindings = UboLayoutBindingArray.data();

		if (vkCreateDescriptorSetLayout(Device.lock()->Device, &layoutInfo, nullptr, &DescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
		return true;
	}


	VkDescriptorSetLayout& VK_DescriptorSetLayout::getDescriptorSetLayout()
	{
		return DescriptorSetLayout;
	}

	std::weak_ptr<VK_Device> VK_DescriptorSetLayout::getDevice() const
	{
		return Device;
	}

	VK_VulkanLayout::VK_VulkanLayout(std::shared_ptr<VK_Device> InDevice, unsigned int InDescriptorSetLayoutNum ):Device(InDevice)
	{
		VK_DescriptorSetLayout temp(InDevice);
		DescriptorSetLayoutArray.resize(InDescriptorSetLayoutNum, temp);
		
	}

	VK_VulkanLayout::~VK_VulkanLayout()
	{
		if (PipelineLayout == VK_NULL_HANDLE || Device.expired())
		{
			return;
		}
		vkDestroyPipelineLayout(Device.lock()->Device, PipelineLayout, nullptr);
	}

	bool VK_VulkanLayout::compile()
	{
		if (Device.expired())
		{
			return false;
		}
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = DescriptorSetLayoutArray.size();
		pipelineLayoutInfo.pSetLayouts = getDescriptorSetLayoutData().data();

		if (vkCreatePipelineLayout(Device.lock()->Device, &pipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
		return true;
	}

	std::vector<VkDescriptorSetLayout> VK_VulkanLayout::getDescriptorSetLayoutData()
	{
		std::vector<VkDescriptorSetLayout> ret(DescriptorSetLayoutArray.size());

		for (int i=0;i< DescriptorSetLayoutArray.size();++i)
		{
			ret[i]= DescriptorSetLayoutArray[i].getDescriptorSetLayout();
		}
		return ret;
	}

	VK_DescriptorSetLayout& VK_VulkanLayout::getDescriptorSetLayoutByIndex(unsigned int Index)
	{
		return DescriptorSetLayoutArray[Index];
	}





	bool VK_DescriptorSets::UpdateDescriptorSets(const std::string& SetKey, const std::vector<VkDescriptorSetLayout>& SetsLayout, std::vector<VkWriteDescriptorSet>& DSWriters, VkDescriptorSet& OutSets)
	{
		int WriterNums=DSWriters.size();
		if (DescriptorSet == VK_NULL_HANDLE|| Device.expired())
		{
			return false;
		}
		for (int i = 0; i < WriterNums; ++i)
		{
			VkWriteDescriptorSet& DWriter= DSWriters[i];
			DWriter.dstSet= DescriptorSet;
			vkUpdateDescriptorSets(Device.lock()->Device, 1, &DWriter, 0, nullptr);
		}
		return true;
	}

	VK_DescriptorSets::VK_DescriptorSets(std::shared_ptr<VK_Device> InDevice)
	{
		Device=InDevice;
	}

	VK_DescriptorSets::~VK_DescriptorSets()
	{
		
	}

}


