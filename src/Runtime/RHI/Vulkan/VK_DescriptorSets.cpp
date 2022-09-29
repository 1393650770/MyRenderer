
#include "VK_DescriptorSets.h"
#include <iostream>
#include <fstream>
#include "VK_Device.h"


namespace MXRender
{

	VK_DescriptorPool::VK_DescriptorPool(std::shared_ptr<VK_Device> InDevice, unsigned int InMaxDescriptorSets):device(InDevice),max_descriptorsets(InMaxDescriptorSets)
	{
		const unsigned int LimitMaxUniformBuffers = max_descriptorsets * 2;
		const unsigned int LimitMaxSamplers = max_descriptorsets / 2;
		const unsigned int LimitMaxCombinedImageSamplers = max_descriptorsets * 3;
		const unsigned int LimitMaxUniformTexelBuffers = max_descriptorsets / 2;
		const unsigned int LimitMaxStorageTexelBuffers = max_descriptorsets / 4;
		const unsigned int LimitMaxStorageBuffers = max_descriptorsets / 4;
		const unsigned int LimitMaxStorageImage = max_descriptorsets / 4;
		const unsigned int LimitMaxSampledImages = max_descriptorsets * 2;
		const unsigned int LimitMaxInputAttachments = max_descriptorsets / 16;

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
		PoolInfo.maxSets = max_descriptorsets;

		if (device.expired() == false&&vkCreateDescriptorPool(device.lock()->device, &PoolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	VK_DescriptorPool::~VK_DescriptorPool()
	{
		if (descriptor_pool != VK_NULL_HANDLE&&device.expired()==false)
		{
			vkDestroyDescriptorPool(device.lock()->device, descriptor_pool, nullptr);
		}
	}

	bool VK_DescriptorPool::allocate_descriptorset(VkDescriptorSetLayout Layout, VkDescriptorSet& OutSet)
	{
		if (descriptor_pool == VK_NULL_HANDLE|| device.expired())
		{
			return false;
		}
		VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo;
		DescriptorSetAllocateInfo.descriptorPool = descriptor_pool;
		DescriptorSetAllocateInfo.descriptorSetCount = 1;
		DescriptorSetAllocateInfo.pSetLayouts = &Layout;

		return vkAllocateDescriptorSets(device.lock()->device, &DescriptorSetAllocateInfo, &OutSet)== VK_SUCCESS;

	}

	bool VK_DescriptorPool::allocate_descriptorsets(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet& OutSets)
	{
		if (descriptor_pool == VK_NULL_HANDLE || device.expired())
		{
			return false;
		}
		VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = InDescriptorSetAllocateInfo;
		DescriptorSetAllocateInfo.descriptorPool = descriptor_pool;
		return vkAllocateDescriptorSets(device.lock()->device, &DescriptorSetAllocateInfo, &OutSets) == VK_SUCCESS;;
	}

	std::weak_ptr<VK_Device> VK_DescriptorPool::get_device() const
	{
		return device;
	}

	unsigned int VK_DescriptorPool::get_max_descriptorsets() const
	{
		return max_descriptorsets;
	}

	const VkDescriptorPool& VK_DescriptorPool::get_descriptor_pool()
	{
		return descriptor_pool;
	}


	VK_DescriptorSetLayout::VK_DescriptorSetLayout(std::shared_ptr<VK_Device> InDevice, unsigned int InMaxBindings):device(InDevice),max_bindings(InMaxBindings)
	{
		
	}

	VK_DescriptorSetLayout::~VK_DescriptorSetLayout()
	{
		if (descriptorset_layout == VK_NULL_HANDLE || device.expired())
		{
			return ;
		}
		vkDestroyDescriptorSetLayout(device.lock()->device, descriptorset_layout, nullptr);
	}

	void VK_DescriptorSetLayout::add_bindingdescriptor(unsigned int DescriptorSetIndex, const VkDescriptorSetLayoutBinding& BindingDescriptor)
	{
		if (DescriptorSetIndex >= max_bindings)
		{
			return;
		}
		if (DescriptorSetIndex > ubo_layout_binding_array.size())
		{
			while(DescriptorSetIndex > ubo_layout_binding_array.size())
				ubo_layout_binding_array.push_back(VkDescriptorSetLayoutBinding());
		}
		VkDescriptorSetLayoutBinding& Binding= ubo_layout_binding_array[DescriptorSetIndex];
		Binding.binding= BindingDescriptor.binding;
		Binding.descriptorCount = BindingDescriptor.descriptorCount;
		Binding.descriptorType = BindingDescriptor.descriptorType;
		Binding.pImmutableSamplers = BindingDescriptor.pImmutableSamplers;
		Binding.stageFlags = BindingDescriptor.stageFlags;
	}

	bool VK_DescriptorSetLayout::compile()
	{
		if ( device.expired())
		{
			return false;
		}
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = ubo_layout_binding_array.size();
		layoutInfo.pBindings = ubo_layout_binding_array.data();

		if (vkCreateDescriptorSetLayout(device.lock()->device, &layoutInfo, nullptr, &descriptorset_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
		return true;
	}


	VkDescriptorSetLayout& VK_DescriptorSetLayout::get_descriptorset_layout()
	{
		return descriptorset_layout;
	}

	std::weak_ptr<VK_Device> VK_DescriptorSetLayout::get_device() const
	{
		return device;
	}

	VK_VulkanLayout::VK_VulkanLayout(std::shared_ptr<VK_Device> InDevice, unsigned int InDescriptorSetLayoutNum ):device(InDevice)
	{
		VK_DescriptorSetLayout temp(InDevice);
		descriptorset_layout_array.resize(InDescriptorSetLayoutNum, temp);
		
	}

	VK_VulkanLayout::~VK_VulkanLayout()
	{
		if (PipelineLayout == VK_NULL_HANDLE || device.expired())
		{
			return;
		}
		vkDestroyPipelineLayout(device.lock()->device, PipelineLayout, nullptr);
	}

	bool VK_VulkanLayout::compile()
	{
		if (device.expired())
		{
			return false;
		}
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = descriptorset_layout_array.size();
		pipelineLayoutInfo.pSetLayouts = get_descriptorset_layout_data().data();

		if (vkCreatePipelineLayout(device.lock()->device, &pipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
		return true;
	}

	std::vector<VkDescriptorSetLayout> VK_VulkanLayout::get_descriptorset_layout_data()
	{
		std::vector<VkDescriptorSetLayout> ret(descriptorset_layout_array.size());

		for (int i=0;i< descriptorset_layout_array.size();++i)
		{
			ret[i]= descriptorset_layout_array[i].get_descriptorset_layout();
		}
		return ret;
	}

	VK_DescriptorSetLayout& VK_VulkanLayout::get_descriptorset_layout_by_index(unsigned int Index)
	{
		return descriptorset_layout_array[Index];
	}





	bool VK_DescriptorSets::update_descriptorsets(const std::string& SetKey, const std::vector<VkDescriptorSetLayout>& SetsLayout, std::vector<VkWriteDescriptorSet>& DSWriters, VkDescriptorSet& OutSets)
	{
		int WriterNums=DSWriters.size();
		if (descriptorset == VK_NULL_HANDLE|| device.expired())
		{
			return false;
		}
		for (int i = 0; i < WriterNums; ++i)
		{
			VkWriteDescriptorSet& DWriter= DSWriters[i];
			DWriter.dstSet= descriptorset;
			vkUpdateDescriptorSets(device.lock()->device, 1, &DWriter, 0, nullptr);
		}
		return true;
	}

	VK_DescriptorSets::VK_DescriptorSets(std::shared_ptr<VK_Device> InDevice)
	{
		device=InDevice;
	}

	VK_DescriptorSets::~VK_DescriptorSets()
	{
		
	}

}


