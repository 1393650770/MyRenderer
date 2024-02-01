
#include "VK_DescriptorSets.h"
#include <iostream>
#include <fstream>
#include "VK_Device.h"
#include <algorithm>


namespace MXRender
{
	/*
	VkDescriptorPool VK_DescriptorPool::create_pool()
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
		PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		PoolInfo.flags = 0;
		PoolInfo.poolSizeCount = Types.size();
		PoolInfo.pPoolSizes = Types.data();
		PoolInfo.maxSets = max_descriptorsets;
		PoolInfo.pNext = 0U;
		VkDescriptorPool temp_descriptor_pool;
		if (vkCreateDescriptorPool(device.lock()->device, &PoolInfo, nullptr, &temp_descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
		return temp_descriptor_pool;
	}

	VkDescriptorPool VK_DescriptorPool::grab_pool()
	{
		if (free_pools.size() > 0)
		{
			VkDescriptorPool pool = free_pools.back();
			free_pools.pop_back();
			return pool;
		}
		else 
		{
			return create_pool();
		}
	}

	VK_DescriptorPool::VK_DescriptorPool(std::shared_ptr<VK_Device> InDevice, unsigned int InMaxDescriptorSets) :device(InDevice), max_descriptorsets(InMaxDescriptorSets)
	{
		if (device.expired()) return;
	}

	VK_DescriptorPool::~VK_DescriptorPool()
	{
		if (device.expired()==false)
		{	

			//delete every pool held
			for (auto p : free_pools)
			{
				vkDestroyDescriptorPool(device.lock()->device , p, nullptr);
			}
			for (auto p : used_pools)
			{
				vkDestroyDescriptorPool(device.lock()->device , p, nullptr);
			}
		}

	}

	bool VK_DescriptorPool::allocate_descriptorset(VkDescriptorSetLayout Layout, VkDescriptorSet& OutSet)
	{
		if (device.expired())
		{
			return false;
		}
		if (descriptor_pool == VK_NULL_HANDLE)
		{
			descriptor_pool = grab_pool();
			used_pools.push_back(descriptor_pool);
		}

		VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo;
		DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		DescriptorSetAllocateInfo.descriptorPool = descriptor_pool;
		DescriptorSetAllocateInfo.descriptorSetCount = 1;
		DescriptorSetAllocateInfo.pSetLayouts = &Layout;
		DescriptorSetAllocateInfo.pNext=VK_NULL_HANDLE;
		VkResult allocResult = vkAllocateDescriptorSets(device.lock()->device, &DescriptorSetAllocateInfo, &OutSet) ;
		bool needReallocate = false;

		switch (allocResult) {
		case VK_SUCCESS:
			//all good, return
			already_create_set[descriptor_pool].push_back(OutSet);
			return true;

			break;
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			//reallocate pool
			needReallocate = true;
			break;
		default:
			//unrecoverable error
			needReallocate = true;
			return false;
		}

		if (needReallocate)
		{
			//allocate a new pool and retry
			descriptor_pool = grab_pool();
			used_pools.push_back(descriptor_pool);

			allocResult = vkAllocateDescriptorSets(device.lock()->device, &DescriptorSetAllocateInfo, &OutSet);

			if (allocResult == VK_SUCCESS)
			{
				already_create_set[descriptor_pool].push_back(OutSet);
				return true;
			}
		}
		return false;

	}

	bool VK_DescriptorPool::allocate_descriptorsets(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet& OutSets)
	{
		if (device.expired())
		{
			return false;
		}
		if (descriptor_pool == VK_NULL_HANDLE)
		{
			descriptor_pool = grab_pool();
			used_pools.push_back(descriptor_pool);
		}

		VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = InDescriptorSetAllocateInfo;
		DescriptorSetAllocateInfo.descriptorPool = descriptor_pool;
		VkResult allocResult = vkAllocateDescriptorSets(device.lock()->device, &DescriptorSetAllocateInfo, &OutSets);
		bool needReallocate = false;

		switch (allocResult) {
		case VK_SUCCESS:
			already_create_set[descriptor_pool].push_back(OutSets);
			return true;

			break;
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			//reallocate pool
			needReallocate = true;
			break;
		default:
			//unrecoverable error
			return false;
		}

		if (needReallocate)
		{
			//allocate a new pool and retry
			descriptor_pool = grab_pool();
			used_pools.push_back(descriptor_pool);

			allocResult = vkAllocateDescriptorSets(device.lock()->device, &DescriptorSetAllocateInfo, &OutSets);

			if (allocResult == VK_SUCCESS)
			{
				already_create_set[descriptor_pool].push_back(OutSets);
				return true;
			}
		}
		return false;
		

	}

	void VK_DescriptorPool::reset_descript_pool(VkDescriptorPoolResetFlags flags)
	{


		for (auto p : used_pools)
		{
			vkResetDescriptorPool(device.lock()->device, p, flags);
		}

		free_pools = used_pools;
		used_pools.clear();
		descriptor_pool = VK_NULL_HANDLE;
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
		if (descriptor_pool == VK_NULL_HANDLE)
		{
			descriptor_pool = grab_pool();
			used_pools.push_back(descriptor_pool);
		}
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
		if (DescriptorSetIndex+1 > layout_binding_array.size())
		{			
			while(DescriptorSetIndex+1 > layout_binding_array.size())
				layout_binding_array.push_back(VkDescriptorSetLayoutBinding());
		}
		VkDescriptorSetLayoutBinding& Binding= layout_binding_array[DescriptorSetIndex];
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
		layoutInfo.bindingCount = layout_binding_array.size();
		layoutInfo.pBindings = layout_binding_array.data();

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

	DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache* cache, VK_DescriptorPool* allocator)
	{
		DescriptorBuilder builder;
		builder.alloc = allocator;
		builder.cache = cache;
		return builder;
	}

	DescriptorBuilder& DescriptorBuilder::bind_buffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
	{
		VkDescriptorSetLayoutBinding newBinding{};

		newBinding.descriptorCount = 1;
		newBinding.descriptorType = type;
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = stageFlags;
		newBinding.binding = binding;

		bindings.push_back(newBinding);

		VkWriteDescriptorSet newWrite{};
		newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		newWrite.pNext = nullptr;

		newWrite.descriptorCount = 1;
		newWrite.descriptorType = type;
		newWrite.pBufferInfo = bufferInfo;
		newWrite.dstBinding = binding;

		writes.push_back(newWrite);
		return *this;
	}

	DescriptorBuilder& DescriptorBuilder::bind_image(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
	{
		VkDescriptorSetLayoutBinding newBinding{};

		newBinding.descriptorCount = 1;
		newBinding.descriptorType = type;
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = stageFlags;
		newBinding.binding = binding;

		bindings.push_back(newBinding);

		image_infos[binding].sampler = imageInfo->sampler;
		image_infos[binding].imageLayout = imageInfo->imageLayout;
		image_infos[binding].imageView = imageInfo->imageView;

		writes.push_back(VkWriteDescriptorSet());
		writes.back().sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes.back().pNext = nullptr;
		writes.back().descriptorCount = 1;
		writes.back().descriptorType = type;
		writes.back().pImageInfo = &(image_infos[binding]);
		writes.back().dstBinding = binding;


		return *this;
	}

	bool DescriptorBuilder::build(VkDescriptorSet& set, VkDescriptorSetLayout& layout)
	{
		//build layout first
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;

		layoutInfo.pBindings = bindings.data();
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());


		layout = cache->create_descriptor_layout(&layoutInfo);


		//allocate descriptor
		bool success = false;

		success=alloc->allocate_descriptorset(layout, set);
		if (!success) 
		{ 
			return false; 
		};

		//write descriptor

		for (VkWriteDescriptorSet& w : writes) {
			w.dstSet = set;
		}

		vkUpdateDescriptorSets(alloc->get_device().lock()->device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
		return true;
	}

	bool DescriptorBuilder::build(VkDescriptorSet& set)
	{
		VkDescriptorSetLayout layout;
		bool result= build(set, layout);
		return result;
	}



	

	void DescriptorLayoutCache::init(VkDevice newDevice)
	{
		device = newDevice;
	}

	void DescriptorLayoutCache::cleanup()
	{ 
		for (auto pair : layoutCache)
		{
			vkDestroyDescriptorSetLayout(device, pair.second, nullptr);
		}
	}

	VkDescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(VkDescriptorSetLayoutCreateInfo* info)
	{
		DescriptorLayoutInfo layoutinfo;
		layoutinfo.bindings.reserve(info->bindingCount);
		bool isSorted = true;
		int32_t lastBinding = -1;
		for (uint32_t i = 0; i < info->bindingCount; i++) 
		{
			layoutinfo.bindings.push_back(info->pBindings[i]);

			//check that the bindings are in strict increasing order
			if (static_cast<int32_t>(info->pBindings[i].binding) > lastBinding)
			{
				lastBinding = info->pBindings[i].binding;
			}
			else {
				isSorted = false;
			}
		}
		if (!isSorted)
		{
			std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {
				return a.binding < b.binding;
				});
		}

		auto it = layoutCache.find(layoutinfo);
		if (it != layoutCache.end())
		{
			return (*it).second;
		}
		else {
			VkDescriptorSetLayout layout;
			vkCreateDescriptorSetLayout(device, info, nullptr, &layout);

			//layoutCache.emplace()
			//add to cache
			layoutCache[layoutinfo] = layout;
			return layout;
		}
	}

	bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
	{
		if (other.bindings.size() != bindings.size())
		{
			return false;
		}
		else {
			//compare each of the bindings is the same. Bindings are sorted so they will match
			for (int i = 0; i < bindings.size(); i++) {
				if (other.bindings[i].binding != bindings[i].binding)
				{
					return false;
				}
				if (other.bindings[i].descriptorType != bindings[i].descriptorType)
				{
					return false;
				}
				if (other.bindings[i].descriptorCount != bindings[i].descriptorCount)
				{
					return false;
				}
				if (other.bindings[i].stageFlags != bindings[i].stageFlags)
				{
					return false;
				}
				if (other.bindings[i].pImmutableSamplers != bindings[i].pImmutableSamplers)
				{
					return false;
				}
			}
			return true;
		}
	}

	size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
	{
		using std::size_t;
		using std::hash;

		size_t result = hash<size_t>()(bindings.size());

		for (const VkDescriptorSetLayoutBinding& b : bindings)
		{

			size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;


			result ^= hash<size_t>()(binding_hash);
		}

		return result;
	}

	*/

}


