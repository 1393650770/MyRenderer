#include"VK_Memory.h"

#include "VK_Define.h"
#include "VK_Device.h"
#include "VK_Utils.h"
#include "../../Core/ConstGlobals.h"
#include "../../Core/ConstDefine.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

#define VK_MEMORY_MAX_SUB_ALLOCATION (64llu << 20llu) //64MB?
#define VK_MEMORY_KEEP_FREELIST_SORTED	1
#define VK_MEMORY_JOIN_FREELIST_ON_THE_FLY	(VK_MEMORY_KEEP_FREELIST_SORTED && 1)
#define VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS			0 // debugging

UInt32 g_vulkan_budget_percentage_scale = 100;
Int g_vulkan_use_buffer_binning = 0;

static VkMemoryPropertyFlags GetMemoryPropertyFlags(ENUM_VulkanAllocationFlags alloc_flags, bool is_has_unified_memory)
{
	VkMemoryPropertyFlags mem_flags = 0;

	//checkf(!(EnumHasAnyFlags(AllocFlags, ENUM_VulkanAllocationFlags::PreferBAR) && !EnumHasAnyFlags(AllocFlags, ENUM_VulkanAllocationFlags::HostVisible)), TEXT("PreferBAR should always be used with HostVisible."));

	if (EnumHasAnyFlags(alloc_flags, ENUM_VulkanAllocationFlags::HostCached))
	{
		mem_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	}
	else if (EnumHasAnyFlags(alloc_flags, ENUM_VulkanAllocationFlags::HostVisible))
	{
		mem_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		if (EnumHasAnyFlags(alloc_flags, ENUM_VulkanAllocationFlags::PreferBAR))
		{
			mem_flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
	}
	else
	{
		mem_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		if (is_has_unified_memory)
		{
			mem_flags |= (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
	}

	if (EnumHasAnyFlags(alloc_flags, ENUM_VulkanAllocationFlags::Memoryless))
	{
		mem_flags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
	}

	return mem_flags;
}

VK_DeviceMemoryAllocation::VK_DeviceMemoryAllocation()
{
    property.packed=0;
}

void* VK_DeviceMemoryAllocation::Map(VkDeviceSize in_size, VkDeviceSize offset)
{
    CHECK_WITH_LOG(property.is_can_be_mapped==false,"RHI Error: This Memory can not be mapped when Map!")
    if(!mapped_pointer)
    {
        CHECK_WITH_LOG(vkMapMemory(device_handle,memory,offset,in_size,0,&mapped_pointer)!=VK_SUCCESS,"RHI Error: This Memory map failed!")
    }
    return mapped_pointer;
}

void VK_DeviceMemoryAllocation::UnMap()
{
    if(mapped_pointer)
    {
        vkUnmapMemory(device_handle,memory);
        mapped_pointer=nullptr;
    }
}

VkDeviceMemory VK_DeviceMemoryAllocation::GetMemory()
{
    return memory;
}

VkDeviceSize VK_DeviceMemoryAllocation::GetSize()
{
    return size;
}

void* VK_DeviceMemoryAllocation::GetMappedPointer()
{
    if(property.is_can_be_mapped&&mapped_pointer)
    {
        return mapped_pointer;
    }
    return nullptr;
}

void VK_DeviceMemoryAllocation::FlushMappedMemory(VkDeviceSize in_size, VkDeviceSize offset)
{
    if(property.is_coherent==false)
    {
        CHECK_WITH_LOG(mapped_pointer==nullptr,"RHI Error: This Memory can not be mapped when Flush !")
        CHECK_WITH_LOG(in_size+offset>size,"RHI Error: Flush Mapped Memory is out size !")
        VkMappedMemoryRange range;
        range.sType=VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory=memory;
        range.offset=offset;
        range.size=in_size;
        
        CHECK_WITH_LOG(vkFlushMappedMemoryRanges(device_handle, 1, &range)!=VK_SUCCESS,"RHI Error: Flush Mapped Memory failed !")
    }
}

void VK_DeviceMemoryAllocation::InvalidateMappedMemory(VkDeviceSize in_size, VkDeviceSize offset)
{
    if(property.is_coherent==false)
    {
        CHECK_WITH_LOG(mapped_pointer==nullptr,"RHI Error: This Memory can not be mapped when Flush !")
        CHECK_WITH_LOG(in_size+offset>size,"RHI Error: Flush Mapped Memory is out size !")
        VkMappedMemoryRange range;
        range.sType=VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory=memory;
        range.offset=offset;
        range.size=in_size;
        
        CHECK_WITH_LOG(vkInvalidateMappedMemoryRanges(device_handle, 1, &range)!=VK_SUCCESS,"RHI Error: Invalidate Mapped Memory failed !")
    }
}

Bool VK_DeviceMemoryAllocation::GetIsCanBeMapped() CONST
{
    return property.is_can_be_mapped;
}

Bool VK_DeviceMemoryAllocation::GetIsCoherent() CONST
{
    return property.is_coherent;
}

Bool VK_DeviceMemoryAllocation::CheckIsMapped() CONST
{
	return mapped_pointer!=nullptr;
}

VK_DeviceMemoryManager::VK_DeviceMemoryManager(VK_Device* in_device):device(in_device)
{
    VkPhysicalDeviceMemoryProperties2 properties2;
    memory_budget.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
    properties2.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    properties2.pNext=&memory_budget;
    vkGetPhysicalDeviceMemoryProperties2(device->GetGpu(), &properties2);
    memcpy(&memory_properties,&(properties2.memoryProperties),sizeof(VkPhysicalDeviceMemoryProperties));
    for (UInt32 index= 0; index<VK_MAX_MEMORY_HEAPS;++index)
    {
        memory_budget.heapBudget[index]=g_vulkan_budget_percentage_scale*  memory_budget.heapBudget[index] / 100;
    }

    VkDeviceSize budget_size = 0;
    for (UInt32 index = 6; index < VK_MAX_MEMORY_HEAPS; ++index)
    {
        budget_size += memory_budget.heapBudget[index];
    }

    //vkGetPhysicalDeviceMemoryProperties(device->GetGpu(), &memory_properties);

    primary_heap_index = -1;
    UInt64 PrimaryHeapSize = 0;
    UInt32 NonLocalHeaps = 0;

    for(UInt32 i = 0; i < memory_properties.memoryHeapCount; ++i)
    {
			
        if (VKHasAllFlags(memory_properties.memoryHeaps[i].flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            if(memory_properties.memoryHeaps[i].size > PrimaryHeapSize)
            {
                primary_heap_index = i;
                PrimaryHeapSize = memory_properties.memoryHeaps[i].size;
            }
        }
        else
        {
            NonLocalHeaps++;
        }
    }
    if(0 == NonLocalHeaps)
    {
        primary_heap_index = -1; 
    }

    // Update bMemoryless support
    is_support_lazily_allocated=false;
    for (UInt32 i = 0; i < memory_properties.memoryTypeCount && !is_support_lazily_allocated; ++i)
    {
        is_support_lazily_allocated = VKHasAllFlags(memory_properties.memoryTypes[i].propertyFlags, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
    }

    memory_heaps.resize(memory_properties.memoryHeapCount);

}

VK_DeviceMemoryManager::~VK_DeviceMemoryManager()
{
}

VK_DeviceMemoryAllocation* VK_DeviceMemoryManager::Alloc(VkDeviceSize allocation_size, UInt32 memory_type_index,
    void* dedicated_allocate_info, float priority, bool is_external)
{
    if (dedicated_allocate_info==nullptr)
    {
        MemoryBlockKey key = {memory_type_index, allocation_size};
        MemoryBlock& block = memory_block_map[key];
        if(block.allocations.size() > 0)
        {
            MemoryBlock::FreeBlock alloc = block.allocations.back();
            block.allocations.pop_back();
            return alloc.allocation;
        }
    }
    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize=allocation_size;
    allocate_info.memoryTypeIndex=memory_type_index;

#if VULKAN_SUPPORTS_MEMORY_PRIORITY
    VkMemoryPriorityAllocateInfoEXT prio;
    if (device->GetOptionalExtensions().HasMemoryPriority)
    {
        prio.sType= VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT;
        prio.priority = priority;
        allocate_info.pNext = &prio;
    }
#endif
    
#if VULKAN_SUPPORTS_DEDICATED_ALLOCATION
    if (dedicated_allocate_info)
    {
        ((VkMemoryDedicatedAllocateInfoKHR*)dedicated_allocate_info)->pNext = allocate_info.pNext;
        allocate_info.pNext = dedicated_allocate_info;
    }
#endif
    VkMemoryAllocateFlagsInfo memory_allocate_flags_info;
    if (device->GetOptionalExtensions().HasBufferDeviceAddress)
    {
        memory_allocate_flags_info.sType= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memory_allocate_flags_info.pNext = allocate_info.pNext;
        allocate_info.pNext = &memory_allocate_flags_info;
    }

    VkDeviceMemory new_memory;
    VkResult result;
    result = vkAllocateMemory(device->GetDevice(), &allocate_info, VULKAN_CPU_ALLOCATOR, &new_memory);
    CHECK_WITH_LOG(result == VK_ERROR_OUT_OF_DEVICE_MEMORY || result == VK_ERROR_OUT_OF_HOST_MEMORY,"RHI Error : Cannot allocate Memory ! ");
    VK_DeviceMemoryAllocation* memory_allocation = new VK_DeviceMemoryAllocation;
    memory_allocation->device_handle = device->GetDevice();
    memory_allocation->memory = new_memory;
    memory_allocation->size = allocation_size;
    memory_allocation->property.memory_type_index = memory_type_index;
    memory_allocation->property.is_can_be_mapped = VKHasAllFlags(memory_properties.memoryTypes[memory_type_index].propertyFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    memory_allocation->property.is_coherent = VKHasAllFlags(memory_properties.memoryTypes[memory_type_index].propertyFlags, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    memory_allocation->property.is_cached = VKHasAllFlags(memory_properties.memoryTypes[memory_type_index].propertyFlags, VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
#if VULKAN_SUPPORTS_DEDICATED_ALLOCATION
    NewAllocation->property.is_dedicated_memory = dedicated_allocate_info != nullptr;
#endif
    ++num_allocations;
    max_num_allocations=std::max(num_allocations,max_num_allocations);

    //if (num_allocations == device->GetLimits().maxMemoryAllocationCount)


    UInt32 heap_index = memory_properties.memoryTypes[memory_type_index].heapIndex;
    memory_heaps[heap_index].allocations.push_back(memory_allocation);
    memory_heaps[heap_index].used_size += allocation_size;
    memory_heaps[heap_index].max_size = std::max(memory_heaps[heap_index].max_size, memory_heaps[heap_index].used_size);
    return memory_allocation;
}


void VK_DeviceMemoryManager::Free(VK_DeviceMemoryAllocation*& allocation)
{
    VkDeviceSize allocation_size = allocation->size;
    MemoryBlockKey key = { allocation->property.memory_type_index, allocation_size };
    MemoryBlock block = memory_block_map[key];
    MemoryBlock::FreeBlock free_block = {allocation, g_frame_number_render_thread};
    block.allocations.push_back(free_block);
    
    
    --num_allocations;

    UInt32 HeapIndex = memory_properties.memoryTypes[allocation->property.memory_type_index].heapIndex;

    memory_heaps[HeapIndex].used_size -= allocation->size;
    auto it= std::find(memory_heaps[HeapIndex].allocations.begin(),memory_heaps[HeapIndex].allocations.end(),allocation);
    if(it!=memory_heaps[HeapIndex].allocations.end())
        memory_heaps[HeapIndex].allocations.erase(it);
    allocation->property.is_freed_by_system = true;

    delete allocation;
    allocation = nullptr;
}


CONST UInt32 VK_DeviceMemoryManager::GetMemoryTypeNum() CONST
{
    return memory_properties.memoryTypeCount;
}

UInt32 VK_DeviceMemoryManager::GetHeapIndex(UInt32 memory_type_index)
{
	return memory_properties.memoryTypes[memory_type_index].heapIndex;
}
CONST VkPhysicalDeviceMemoryProperties& VK_DeviceMemoryManager::GetMemoryProperties() CONST
{
    return memory_properties;
}

VkResult VK_DeviceMemoryManager::GetMemoryTypeFromProperties(UInt32 type_bits, VkMemoryPropertyFlags properties, UInt32* out_type_index)
{
	for (UInt32 i = 0; i < memory_properties.memoryTypeCount && type_bits; ++i)
	{
		if ((type_bits & 1) == 1)
		{
			if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				*out_type_index = i;
				return VK_SUCCESS;
			}
		}
		type_bits >>= 1;
	}
	return VK_ERROR_FEATURE_NOT_PRESENT;
}

Bool VK_DeviceMemoryManager::GetIsSupportUnifiedMemory() CONST
{
    return is_support_unified_memory;
}

VkResult VK_DeviceMemoryManager::GetMemoryTypeFromPropertiesExcluding(UInt32 type_bits, VkMemoryPropertyFlags properties, UInt32 exclude_type_index, UInt32* out_type_index)
{
	for (UInt32 i = 0; i < memory_properties.memoryTypeCount && type_bits; i++)
	{
		if ((type_bits & 1) == 1)
		{
			if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties && exclude_type_index != i)
			{
				*out_type_index = i;
				return VK_SUCCESS;
			}
		}
		type_bits >>= 1;
	}
	return VK_ERROR_FEATURE_NOT_PRESENT;
}

VK_MemoryManager::VK_MemoryManager(VK_Device* in_device):device_memory_manager(in_device->GetDeviceMemoryManager()),device(in_device)
{
    CONST UInt32 type_bits = (1<<device_memory_manager->GetMemoryTypeNum())-1;
    CONST VkPhysicalDeviceMemoryProperties& memory_properties = device_memory_manager->GetMemoryProperties();
    resource_heaps.resize(memory_properties.memoryTypeCount);

	auto GetMemoryTypesFromPropertiesFunc = [memory_properties](UInt32 in_type_bits, VkMemoryPropertyFlags properties, Vector<UInt32>& out_type_indices)
	{
		for (UInt32 i = 0; i < memory_properties.memoryTypeCount && in_type_bits; ++i)
		{
			if ((in_type_bits & 1) == 1)
			{
				// Type is available, does it match user properties?
				if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					out_type_indices.push_back(i);
				}
			}
			in_type_bits >>= 1;
		}
		return out_type_indices.size() > 0;
	};

    {
        UInt32 type_index=0;
        device_memory_manager->GetMemoryTypeFromProperties(type_bits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &type_index);
        UInt64 heap_size =memory_properties.memoryHeaps[memory_properties.memoryTypes[type_index].heapIndex].size;
        resource_heaps[type_index]= new VK_MemoryResourceHeap(this,type_index, STAGING_HEAP_PAGE_SIZE);
        auto& page_size_buckets= resource_heaps[type_index]->page_size_buckets;
        VK_VulkanPageSizeBucket bucket0 = {STAGING_HEAP_PAGE_SIZE,STAGING_HEAP_PAGE_SIZE,VK_VulkanPageSizeBucket::BUCKET_MASK_IMAGE|VK_VulkanPageSizeBucket::BUCKET_MASK_BUFFER};
        VK_VulkanPageSizeBucket bucket1= { UINT64_MAX, 0, VK_VulkanPageSizeBucket::BUCKET_MASK_IMAGE | VK_VulkanPageSizeBucket::BUCKET_MASK_BUFFER };
        page_size_buckets.push_back(bucket0);
        page_size_buckets.push_back(bucket1);
    }

    {
        UInt32 type_index=0;
        UInt32 host_vis_cached_index=0;
        VkResult host_cache_result= device_memory_manager->GetMemoryTypeFromProperties(type_bits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, &host_vis_cached_index);
        UInt32 host_vis_index=0;
        VkResult host_result = device_memory_manager->GetMemoryTypeFromProperties(type_bits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &host_vis_index);
		if (host_cache_result == VK_SUCCESS)
		{
			type_index = host_vis_cached_index;
		}
		else if (host_result == VK_SUCCESS)
		{
            type_index = host_vis_index;
		}
        CHECK_WITH_LOG(host_cache_result != VK_SUCCESS&& host_result != VK_SUCCESS,"RHI Error : No Memory Type Found Supporting VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ! ")
        auto& page_size_buckets= resource_heaps[type_index]->page_size_buckets;
		VK_VulkanPageSizeBucket bucket0 = { STAGING_HEAP_PAGE_SIZE, STAGING_HEAP_PAGE_SIZE, VK_VulkanPageSizeBucket::BUCKET_MASK_IMAGE | VK_VulkanPageSizeBucket::BUCKET_MASK_BUFFER };
        VK_VulkanPageSizeBucket bucket1 = { UINT64_MAX, 0, VK_VulkanPageSizeBucket::BUCKET_MASK_IMAGE | VK_VulkanPageSizeBucket::BUCKET_MASK_BUFFER };
        page_size_buckets.push_back(bucket0);
        page_size_buckets.push_back(bucket1);
    }

	{
		Vector<UInt32> type_indices;
		GetMemoryTypesFromPropertiesFunc(type_bits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, type_indices);

		for (UInt32 index = 0; index < memory_properties.memoryTypeCount; ++index)
		{
			Int heap_index = memory_properties.memoryTypes[index].heapIndex;
			VkDeviceSize heap_size = memory_properties.memoryHeaps[heap_index].size;
			if (!resource_heaps[index])
			{
                resource_heaps[index] = new VK_MemoryResourceHeap(this, index);
                resource_heaps[index]->is_support_host_cached = ((memory_properties.memoryTypes[index].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
                resource_heaps[index]->is_support_lazily_allocated = ((memory_properties.memoryTypes[index].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
				auto& page_size_buckets = resource_heaps[index]->page_size_buckets;

/*#if PLATFORM_ANDROID
				FVulkanPageSizeBucket BucketImage = { UINT64_MAX, (uint32)ANDROID_MAX_HEAP_IMAGE_PAGE_SIZE, FVulkanPageSizeBucket::BUCKET_MASK_IMAGE };
				FVulkanPageSizeBucket BucketBuffer = { UINT64_MAX, (uint32)ANDROID_MAX_HEAP_BUFFER_PAGE_SIZE, FVulkanPageSizeBucket::BUCKET_MASK_BUFFER };
				PageSizeBuckets.Add(BucketImage);
				PageSizeBuckets.Add(BucketBuffer);
#else*/
				UInt32 small_allocation_threshold = 2 << 20;
                UInt32 large_allocation_threshold = VK_MEMORY_MAX_SUB_ALLOCATION;
				VkDeviceSize small_page_size = 8llu << 20;
				VkDeviceSize large_page_size = std::min<VkDeviceSize>(heap_size / 8, GPU_ONLY_HEAP_PAGE_SIZE);

				VK_VulkanPageSizeBucket bucket_small_image = { small_allocation_threshold, (UInt32)small_page_size, VK_VulkanPageSizeBucket::BUCKET_MASK_IMAGE };
                VK_VulkanPageSizeBucket bucket_large_image = { large_allocation_threshold, (UInt32)large_page_size, VK_VulkanPageSizeBucket::BUCKET_MASK_IMAGE };
                VK_VulkanPageSizeBucket bucket_small_buffer = { small_allocation_threshold, (UInt32)small_page_size, VK_VulkanPageSizeBucket::BUCKET_MASK_BUFFER };
                VK_VulkanPageSizeBucket bucket_large_buffer = { large_allocation_threshold, (UInt32)large_page_size, VK_VulkanPageSizeBucket::BUCKET_MASK_BUFFER };
                VK_VulkanPageSizeBucket bucket_remainder = { UINT64_MAX, 0, VK_VulkanPageSizeBucket::BUCKET_MASK_BUFFER | VK_VulkanPageSizeBucket::BUCKET_MASK_IMAGE };
                page_size_buckets.push_back(bucket_small_image);
                page_size_buckets.push_back(bucket_large_image);
                page_size_buckets.push_back(bucket_small_buffer);
                page_size_buckets.push_back(bucket_large_buffer);
                page_size_buckets.push_back(bucket_remainder);
			}
		}
	}

}


VK_DeviceMemoryManager* VK_MemoryManager::GetDeviceMemoryManager()
{
    return device_memory_manager;
}

Bool VK_MemoryManager::AllocateImageMemory(VK_Allocation& out_allocation, VkImage in_image, ENUM_VulkanAllocationFlags in_alloc_flags, UInt32 in_force_min_alignment /*= 1*/)
{
	VkImageMemoryRequirementsInfo2 image_memory_requirements_info;
	image_memory_requirements_info.sType= VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
	image_memory_requirements_info.image = in_image;

	VkMemoryDedicatedRequirements dedicated_requirements;
	dedicated_requirements.sType= VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;

	VkMemoryRequirements2 memory_requirements;
	memory_requirements.sType= VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
	memory_requirements.pNext = &dedicated_requirements;

	vkGetImageMemoryRequirements2(device->GetDevice(), &image_memory_requirements_info, &memory_requirements);

	// Allow caller to force his own alignment requirements
	memory_requirements.memoryRequirements.alignment = std::max(memory_requirements.memoryRequirements.alignment, (VkDeviceSize)in_force_min_alignment);

	if (dedicated_requirements.requiresDedicatedAllocation || dedicated_requirements.prefersDedicatedAllocation)
	{
		in_alloc_flags |= ENUM_VulkanAllocationFlags::Dedicated;
	}

	// For now, translate all the flags into a call to the legacy AllocateImageMemory() function
	const VkMemoryPropertyFlags memory_property_flags = GetMemoryPropertyFlags(in_alloc_flags, device_memory_manager->GetIsSupportUnifiedMemory());
	const bool is_external = EnumHasAllFlags(in_alloc_flags, ENUM_VulkanAllocationFlags::External);
	const bool is_force_separate_allocation = EnumHasAllFlags(in_alloc_flags, ENUM_VulkanAllocationFlags::Dedicated);

	//AllocateImageMemory
	{
		VkMemoryPropertyFlags use_memory_property_flags = memory_property_flags;
		UInt32 type_index = 0;
		VkResult result = device_memory_manager->GetMemoryTypeFromProperties(memory_requirements.memoryRequirements.memoryTypeBits, use_memory_property_flags, &type_index);
		bool is_mapped = VKHasAllFlags(use_memory_property_flags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		if ((result != VK_SUCCESS) || !resource_heaps[type_index])
		{
			if (VKHasAllFlags(use_memory_property_flags, VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
			{
				// Try non-cached flag
				use_memory_property_flags = use_memory_property_flags & ~VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			}

			if (VKHasAllFlags(memory_property_flags, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT))
			{
				// Try non-lazy flag
				use_memory_property_flags = use_memory_property_flags & ~VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
			}
		}

		const bool is_force_separate_allocation = is_external || VKHasAllFlags(use_memory_property_flags, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
		if (!resource_heaps[type_index]->AllocateResource(out_allocation, nullptr, ENUM_VK_HeapAllocationType::Image, memory_requirements.memoryRequirements.size, memory_requirements.memoryRequirements.alignment, is_mapped, is_force_separate_allocation, ENUM_VK_AllocationMetaType::EVulkanAllocationMetaImageOther, is_external))
		{

			use_memory_property_flags &= (~VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			CHECK_WITH_LOG(device_memory_manager->GetMemoryTypeFromPropertiesExcluding(memory_requirements.memoryRequirements.memoryTypeBits, use_memory_property_flags, type_index, &type_index) != VK_SUCCESS,
				"RHI Error: failed to getMemoryTypeFromPropertiesExcluding index !")
				is_mapped = VKHasAllFlags(use_memory_property_flags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			if (!resource_heaps[type_index]->AllocateResource(out_allocation, nullptr, ENUM_VK_HeapAllocationType::Image, memory_requirements.memoryRequirements.size, memory_requirements.memoryRequirements.alignment, is_mapped, is_force_separate_allocation, ENUM_VK_AllocationMetaType::EVulkanAllocationMetaImageOther, is_external))
			{
				return false;
			}
		}
		return true;
	}
}

Bool VK_MemoryManager::AllocateBufferMemory(VK_Allocation& out_allocation, VkBuffer in_buffer, ENUM_VulkanAllocationFlags in_alloc_flags, UInt32 in_force_min_alignment /*= 1*/)
{
	VkBufferMemoryRequirementsInfo2 buffer_memory_requirements_Info;
    buffer_memory_requirements_Info.sType= VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
	buffer_memory_requirements_Info.buffer = in_buffer;

	VkMemoryDedicatedRequirements dedicated_requirements;
    dedicated_requirements.sType= VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;

	VkMemoryRequirements2 memory_requirements;
    memory_requirements.sType= VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
	memory_requirements.pNext = &dedicated_requirements;

	vkGetBufferMemoryRequirements2(device ->GetDevice(), &buffer_memory_requirements_Info, &memory_requirements);

	// Allow caller to force his own alignment requirements
	memory_requirements.memoryRequirements.alignment = std::max(memory_requirements.memoryRequirements.alignment, (VkDeviceSize)in_force_min_alignment);

	if (dedicated_requirements.requiresDedicatedAllocation || dedicated_requirements.prefersDedicatedAllocation)
	{
		in_alloc_flags |= ENUM_VulkanAllocationFlags::Dedicated;
	}

	// For now, translate all the flags into a call to the legacy AllocateBufferMemory() function
	const VkMemoryPropertyFlags memory_property_flags = GetMemoryPropertyFlags(in_alloc_flags, device_memory_manager->GetIsSupportUnifiedMemory());
	const bool is_external = EnumHasAllFlags(in_alloc_flags, ENUM_VulkanAllocationFlags::External);
	const bool is_force_separate_allocation = EnumHasAllFlags(in_alloc_flags, ENUM_VulkanAllocationFlags::Dedicated);

    //AllocateBufferMemory
    {
        VkMemoryPropertyFlags use_memory_property_flags = memory_property_flags;
		UInt32 type_index = 0;
		VkResult result = device_memory_manager->GetMemoryTypeFromProperties(memory_requirements.memoryRequirements.memoryTypeBits, use_memory_property_flags, &type_index);
		bool is_mapped = VKHasAllFlags(use_memory_property_flags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		if ((result != VK_SUCCESS) || !resource_heaps[type_index])
		{
			if (VKHasAllFlags(use_memory_property_flags, VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
			{
				// Try non-cached flag
                use_memory_property_flags = use_memory_property_flags & ~VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			}

			if (VKHasAllFlags(memory_property_flags, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT))
			{
				// Try non-lazy flag
                use_memory_property_flags = use_memory_property_flags & ~VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
			}

			// Try another heap type
			UInt32 original_type_index = type_index;
			if (device_memory_manager->GetMemoryTypeFromPropertiesExcluding(memory_requirements.memoryRequirements.memoryTypeBits, use_memory_property_flags, (result == VK_SUCCESS) ? type_index : (UInt32)-1, &type_index) != VK_SUCCESS)
			{

			}
			if (!resource_heaps[type_index])
			{
			}
		}

		//check(MemoryReqs.size <= (uint64)MAX_uint32);

		if (!resource_heaps[type_index]->AllocateResource(out_allocation, nullptr, ENUM_VK_HeapAllocationType::Buffer, memory_requirements.memoryRequirements.size, memory_requirements.memoryRequirements.alignment, is_mapped, is_force_separate_allocation, ENUM_VK_AllocationMetaType::EVulkanAllocationMetaBufferOther, is_external))
		{
			// Try another memory type if the allocation failed
			CHECK_WITH_LOG(device_memory_manager->GetMemoryTypeFromPropertiesExcluding(memory_requirements.memoryRequirements.memoryTypeBits, memory_property_flags, type_index, &type_index)!= VK_SUCCESS,
                "RHI Error: failed to getMemoryTypeFromPropertiesExcluding index !")
            is_mapped = (memory_property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			if (resource_heaps[type_index])
            { 
                if (!(resource_heaps[type_index]->AllocateResource(out_allocation, nullptr, ENUM_VK_HeapAllocationType::Buffer, memory_requirements.memoryRequirements.size, memory_requirements.memoryRequirements.alignment, is_mapped, is_force_separate_allocation, ENUM_VK_AllocationMetaType::EVulkanAllocationMetaBufferOther, is_external)))
                {
                    return false;
                }
            }
		}
		return true;
    }

	if (out_allocation.IsValid())
	{
		if (EnumHasAllFlags(in_alloc_flags, ENUM_VulkanAllocationFlags::AutoBind))
		{
			VkBindBufferMemoryInfo bind_buffer_memory_info;
            bind_buffer_memory_info.sType= VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
			bind_buffer_memory_info.buffer = in_buffer;
			bind_buffer_memory_info.memory = out_allocation.GetDeviceMemoryHandle(device);
			bind_buffer_memory_info.memoryOffset = out_allocation.offset;

			CHECK_WITH_LOG(vkBindBufferMemory2(device->GetDevice(), 1, &bind_buffer_memory_info)!= VK_SUCCESS,
                            "RHI Error : failed to bind buffer memory !")
		}

	}
	else
	{
		if (!EnumHasAllFlags(in_alloc_flags, ENUM_VulkanAllocationFlags::NoError))
		{
			const bool IsHostMemory = EnumHasAnyFlags(in_alloc_flags, ENUM_VulkanAllocationFlags::HostVisible | ENUM_VulkanAllocationFlags::HostCached);
			//HandleOOM(false, IsHostMemory ? VK_ERROR_OUT_OF_HOST_MEMORY : VK_ERROR_OUT_OF_DEVICE_MEMORY, MemoryRequirements.memoryRequirements.size);
		}
	}

	return out_allocation.IsValid();
}

VK_MemoryResourceHeap::VK_MemoryResourceHeap(VK_MemoryManager* in_owner, UInt32 in_memory_type_index, UInt32 in_override_page_size /*= 0*/)
: owner(in_owner),memory_type_index(in_memory_type_index),override_page_size(in_override_page_size)
{
    heap_index=owner->GetDeviceMemoryManager()->GetHeapIndex(memory_type_index);
}

VK_MemoryResourceHeap::~VK_MemoryResourceHeap()
{

}

bool MetaTypeCanEvict(ENUM_VK_AllocationMetaType meta_type)
{
	switch (meta_type)
	{
	case ENUM_VK_AllocationMetaType::EVulkanAllocationMetaImageOther:
		return true;
	default:
		return false;
	}
}

Bool VK_MemoryResourceHeap::AllocateResource(VK_Allocation& out_allocation, VK_Evictable* allocation_owner, ENUM_VK_HeapAllocationType type, UInt32 size, UInt32 alignment, Bool is_map_allocation, Bool is_force_separate_allocation, ENUM_VK_AllocationMetaType meta_type, Bool is_external)
{

    VK_DeviceMemoryManager* device_memory_manager = owner->GetDeviceMemoryManager();
    VK_VulkanPageSizeBucket memory_bucket;
    UInt32 bucket_id= GetPageSizeBucket(memory_bucket,type,size , is_force_separate_allocation);
    Bool is_support_unified_memory= device_memory_manager->GetIsSupportUnifiedMemory();
    Vector<VK_MemoryResourceFragmentAllocator*>& used_pages = active_pages[bucket_id];
    ENUM_VK_AllocationType allocation_type = (type==ENUM_VK_HeapAllocationType::Image)? ENUM_VK_AllocationType::EVulkanAllocationImage : 
                                             (type == ENUM_VK_HeapAllocationType::Buffer) ? ENUM_VK_AllocationType::EVulkanAllocationBuffer: ENUM_VK_AllocationType::EVulkanAllocationEmpty;
    UInt8 allocation_flags = (!is_support_unified_memory&& MetaTypeCanEvict(meta_type))? VulkanAllocationFlagsCanEvict: 0;
    if(is_map_allocation)
    {
        allocation_flags |= VulkanAllocationFlagsMapped;
    }
    UInt32 allocation_size;
    if (is_force_separate_allocation == false)
    {
        if (size < memory_bucket.page_size)
        {
            for (Int index = 0 ; index < used_pages.size(); ++index)
            {
                VK_MemoryResourceFragmentAllocator* page = used_pages[index];
                if (page->GetSubresourceAllcatorFlags()== allocation_flags)
                {   
					if (page->TryAllocate(out_allocation,allocation_owner,size,alignment,meta_type))
					{
	                    return true;
					}
                }
            }
        }
		allocation_size = std::max(size,memory_bucket.page_size);
    }
	else
	{
		allocation_size = size;
	}

	VK_DeviceMemoryAllocation* device_memory_allocation = device_memory_manager->Alloc( allocation_size, memory_type_index, nullptr, VULKAN_MEMORY_HIGHEST_PRIORITY, is_external);
	if (!device_memory_allocation && size != allocation_size)
	{
		// Retry with a smaller size
		device_memory_allocation = device_memory_manager->Alloc( size, memory_type_index, nullptr, VULKAN_MEMORY_HIGHEST_PRIORITY, is_external);
		if (!device_memory_allocation)
		{
			return false;
		}
	}
	if (!device_memory_allocation)
	{
		//LOG
	}
	if (is_map_allocation)
	{
		device_memory_allocation->Map(allocation_size, 0);
	}

	UInt32 buffer_id = 0;
	//if (UseVulkanDescriptorCache())
	//{
	//	BufferId = ++GVulkanBufferHandleIdCounter;
	//}
	++page_id_counter;
	VK_MemoryResourceFragmentAllocator* Page = new VK_MemoryResourceFragmentAllocator(allocation_type,owner, allocation_flags, device_memory_allocation, memory_type_index, buffer_id);
	owner->RegisterSubresourceAllocator(Page);
	Page->bucket_id = bucket_id;
	active_pages[bucket_id].push_back(Page);

	used_memory += allocation_size;

	peak_page_size = std::max(peak_page_size, allocation_size);


	bool bOk = Page->TryAllocate(out_allocation, allocation_owner, size, alignment, meta_type);
    return bOk;
}

void VK_MemoryResourceHeap::ReleasePage(VK_MemoryResourceFragmentAllocator* in_page)
{
	owner->UnregisterSubresourceAllocator(in_page);
    VK_DeviceMemoryAllocation* allocation = in_page->memory_allocation;
	in_page->memory_allocation = 0;
	owner->GetDeviceMemoryManager()->Free(allocation);
    used_memory -= in_page->max_size;
	delete in_page;
}

void VK_MemoryResourceHeap::FreePage(VK_MemoryResourceFragmentAllocator* in_page)
{

	//in_page->FrameFreed = GFrameNumberRenderThread;

	if (in_page->type == ENUM_VK_AllocationType::EVulkanAllocationImageDedicated)
	{
		auto it = std::find(used_dedicated_pages.begin(), used_dedicated_pages.end(), in_page);
		if (it !=used_dedicated_pages.end())
		{
			used_dedicated_pages.erase(it);
		}
		ReleasePage(in_page);

	}
	else
	{
		UInt8 bucket_id = in_page->bucket_id;

		Vector<VK_MemoryResourceFragmentAllocator*>& pages = active_pages[bucket_id];
		auto it = std::find(pages.begin(), pages.end(), in_page);
		if (it != used_dedicated_pages.end())
		{
			pages.erase(it);
		}
		ReleasePage(in_page);
	}
}

UInt32 VK_MemoryResourceHeap::GetPageSizeBucket(VK_VulkanPageSizeBucket& out_bucket, ENUM_VK_HeapAllocationType type, UInt32 allocation_size, Bool is_force_single_allocation)
{
    if (is_force_single_allocation)
    {
        UInt32 bucket_id=page_size_buckets.size()-1;
        out_bucket=page_size_buckets[bucket_id];
        return bucket_id;
    }
    UInt32 mask=0;
    mask |= type== ENUM_VK_HeapAllocationType::Image? VK_VulkanPageSizeBucket::BUCKET_MASK_IMAGE : 0;
    mask |= type == ENUM_VK_HeapAllocationType::Buffer ? VK_VulkanPageSizeBucket::BUCKET_MASK_BUFFER : 0;
    for (VK_VulkanPageSizeBucket& bucket : page_size_buckets)
    {
        if (mask == (bucket.bucket_mask & mask) && allocation_size <= bucket.max_allocation)
        {
            out_bucket=bucket;
            return &bucket - &page_size_buckets[0];
        }
    }

    return 0xffffffff;
}

Bool VK_MemoryResourceFragmentAllocator::TryAllocate(VK_Allocation& out_allocation, VK_Evictable* allocation_owner, UInt32 in_size, UInt32 in_alignment, ENUM_VK_AllocationMetaType in_meta_type)
{
    if (is_evicting || is_locked)
    {
        return false;
    }
	in_alignment = std::max(in_alignment, alignment);
	for (Int index = 0; index < free_list.size(); ++index)
	{
        VK_Section& entry = free_list[index];
		UInt32 allocated_offset = entry.offset;
        UInt32 aligned_offset = Align(entry.offset, in_alignment);
        UInt32 alignment_adjustment = aligned_offset - entry.offset;
        UInt32 allocated_size = alignment_adjustment + in_size;
		if (allocated_size <= entry.size)
		{
            VK_Section::AllocateFromEntry(free_list, index, allocated_size);

			used_size += allocated_size;
			Int extra_offset = AllocateInternalData();
			out_allocation.Init(type, in_meta_type, (UInt64)buffer, in_size, aligned_offset, allocator_index, extra_offset, buffer_id);
			memory_used[(UInt32)in_meta_type] += allocated_size;
			static UInt32 uid_counter = 0;
			uid_counter++;
			internal_data[extra_offset].Init(out_allocation, allocation_owner, allocated_offset, allocated_size, in_alignment);
			alloc_calls++;
			num_sub_allocations++;

			is_defragging = false;
			return true;
		}
	}
	return false;
}

UInt8 VK_MemoryResourceFragmentAllocator::GetSubresourceAllcatorFlags()
{
    return subresource_allocator_flags;
}

void VK_MemoryResourceFragmentAllocator::Free(VK_Allocation& allocation)
{
	bool bTryFree = false;

	{
		free_calls++;
		UInt32 allocation_offset;
        UInt32 allocation_size;
		{
			VK_AllocationInternalInfo& data = internal_data[allocation.allocation_index];
			bool is_was_discarded = data.state == VK_AllocationInternalInfo::ENUM_VK_AllocationState::EFREEDISCARDED;
			CHECK_WITH_LOG(!(data.state == VK_AllocationInternalInfo::ENUM_VK_AllocationState::EALLOCATED || data.state == VK_AllocationInternalInfo::ENUM_VK_AllocationState::EFREEPENDING || data.state == VK_AllocationInternalInfo::ENUM_VK_AllocationState::EFREEDISCARDED),
                "RHI Error : failed to free vk_allocation because data.state error!");
			allocation_offset = data.allocation_offset;
			allocation_size = data.allocation_size;
			if (!is_was_discarded)
			{
				memory_used[(UInt32)allocation.meta_type] -= allocation_size;
			}
			data.state = VK_AllocationInternalInfo::ENUM_VK_AllocationState::EFREED;
			FreeInternalData(allocation.allocation_index);
            allocation.allocation_index = -1;
			if (is_was_discarded)
			{
				//this occurs if we do full defrag when there are pending frees. in that case the memory is just not moved to the new block.
				return;
			}
		}
		VK_Section new_free;
		new_free.offset = allocation_offset;
		new_free.size = allocation_size;
		//check(NewFree.Offset <= GetMaxSize());
		//check(NewFree.Offset + NewFree.Size <= GetMaxSize());
        VK_Section::Add(free_list, new_free);
		used_size -= allocation_size;
		num_sub_allocations--;
		//check(UsedSize >= 0);
		if (MergeFreeBlocks())
		{
			bTryFree = true; //cannot free here as it will cause incorrect lock ordering
		}
	}

	if (bTryFree)
	{
		owner_memory_manager->ReleaseSubresourceAllocator(this);
	}
}

void VK_MemoryManager::ReleaseSubresourceAllocator(VK_MemoryResourceFragmentAllocator* subresource_allocator)
{
	if (subresource_allocator->type == ENUM_VK_AllocationType::EVulkanAllocationPooledBuffer)
	{
		auto it = std::find(used_buffer_allocations[subresource_allocator->pool_size_index].begin(), used_buffer_allocations[subresource_allocator->pool_size_index].end(), subresource_allocator);
		if (it != used_buffer_allocations[subresource_allocator->pool_size_index].end())
		{
			used_buffer_allocations[subresource_allocator->pool_size_index].erase(it);
		}
		//SubresourceAllocator->FrameFreed = GFrameNumberRenderThread;
		free_buffer_allocations[subresource_allocator->pool_size_index].push_back(subresource_allocator);
	}
	else
	{
		VK_MemoryResourceHeap* Heap = resource_type_heaps[subresource_allocator->memory_type_index];
		Heap->FreePage(subresource_allocator);
	}
}

Bool VK_MemoryManager::FreeAllocation(VK_Allocation& allocation)
{
	CONST UInt32 index = allocation.allocator_index;
	GetSubresourceAllocator(index)->Free(allocation);
}

VK_MemoryResourceFragmentAllocator* VK_MemoryManager::GetSubresourceAllocator(const UInt32 allocator_index)
{
	return all_buffer_allocations[allocator_index];
}

VK_MemoryResourceFragmentAllocator::~VK_MemoryResourceFragmentAllocator()
{

}

Int VK_MemoryResourceFragmentAllocator::AllocateInternalData()
{
	Int free_listnode_head = internal_free_listnode_index;
	if (free_listnode_head < 0)
	{
		internal_data.emplace_back();
        Int result = internal_data.size()-1;
        internal_data[result].next_free = -1;
		return result;

	}
	else
	{
        internal_free_listnode_index = internal_data[free_listnode_head].next_free;
		internal_data[free_listnode_head].next_free = -1;
		return free_listnode_head;
	}
}

void VK_MemoryResourceFragmentAllocator::FreeInternalData(Int index)
{
	CHECK_WITH_LOG(!(internal_data[index].state == VK_AllocationInternalInfo::ENUM_VK_AllocationState::EUNUSED || internal_data[index].state == VK_AllocationInternalInfo::ENUM_VK_AllocationState::EFREED),
        "RHI Error : failed to free internal data because internaldata.state error!")
    CHECK_WITH_LOG(internal_data[index].next_free != -1,
        "RHI Error : failed to free vk_allocation because internaldata is free!")
    internal_data[index].next_free = internal_free_listnode_index;
    internal_free_listnode_index = index;
    internal_data[index].state = VK_AllocationInternalInfo::ENUM_VK_AllocationState::EUNUSED;
}

void VK_MemoryResourceFragmentAllocator::Destroy(VK_Device* device)
{

}

Bool VK_MemoryResourceFragmentAllocator::MergeFreeBlocks()
{
    VK_Section::MergeConsecutiveRanges(free_list);
	if (free_list.size() == 1)
	{
		if (num_sub_allocations == 0)
		{
			//check(UsedSize == 0);
			//checkf(FreeList[0].Offset == 0 && FreeList[0].Size == MaxSize, TEXT("Resource Suballocation leak, should have %d free, only have %d; missing %d bytes"), MaxSize, FreeList[0].Size, MaxSize - FreeList[0].Size);
			return true;
		}
	}
	return false;
}

VK_MemoryResourceFragmentAllocator::VK_MemoryResourceFragmentAllocator(ENUM_VK_AllocationType in_type, VK_MemoryManager* in_owner, UInt8 in_subresource_allocator_flags, VK_DeviceMemoryAllocation* in_device_memory_allocation, UInt32 in_memory_type_index, UInt32 buffer_id /*= 0xffffffff*/)
:type(in_type),owner_memory_manager(in_owner),subresource_allocator_flags(in_subresource_allocator_flags),memory_allocation(in_device_memory_allocation),memory_type_index(in_memory_type_index),buffer_id(buffer_id)
{
	max_size = in_device_memory_allocation->GetSize();

	if (in_device_memory_allocation->CheckIsMapped())
	{
		subresource_allocator_flags |= VulkanAllocationFlagsMapped;
	}
	else
	{
		subresource_allocator_flags &= ~VulkanAllocationFlagsMapped;
	}

	VK_Section FullRange;
	FullRange.offset = 0;
	FullRange.size = max_size;
	free_list.push_back(FullRange);
}

void VK_AllocationInternalInfo::Init(CONST VK_Allocation& alloc, VK_Evictable* in_allocation_owner, UInt32 in_allocation_offset, UInt32 in_allocation_size, UInt32 in_alignment)
{
	CHECK_WITH_LOG(state != ENUM_VK_AllocationState::EUNUSED,"RHI Error: failed to create allocation internal info !")
    state =  ENUM_VK_AllocationState::EALLOCATED;
	type = alloc.GetType();
	meta_type = alloc.meta_type;

	size = alloc.size;
	allocation_size = in_allocation_size;
	allocation_offset = in_allocation_offset;
	allocation_owner = in_allocation_owner;
	alignment = in_alignment;
}

void VK_Allocation::Init(ENUM_VK_AllocationType Type, ENUM_VK_AllocationMetaType MetaType, UInt64 Handle, UInt32 InSize, UInt32 AlignedOffset, UInt32 AllocatorIndex, UInt32 AllocationIndex, UInt32 BufferId)
{

}

void VK_Allocation::Free(VK_Device& Device)
{

}

Int VK_Section::Add(Vector<VK_Section>& ranges, CONST VK_Section& item)
{
#if	VK_MEMORY_KEEP_FREELIST_SORTED
	Int num_ranges = ranges.size();
	if (num_ranges <= 0)
	{
		ranges.push_back(item);
		return ranges.size() - 1;
	}

	VK_Section* data = ranges.data();
	for (Int index = 0; index < num_ranges; ++index)
	{
		if ((data[index].offset > item.offset))
		{
			return InsertAndTryToMerge(ranges, item, index);
		}
	}
	return AppendAndTryToMerge(ranges, item);
#else
	return ranges.push_back(item);
#endif
}

Int VK_Section::InsertAndTryToMerge(Vector<VK_Section>& ranges, CONST VK_Section& item, Int proposed_index)
{
#if !VK_MEMORY_JOIN_FREELIST_ON_THE_FLY
	Int ret = proposed_index;
	auto it = ranges.begin() + proposed_index;
	ranges.insert(it, item);
#else
	// there are four cases here
	// 1) nothing can be merged (distinct ranges)		 XXXX YYY ZZZZZ  =>   XXXX YYY ZZZZZ
	// 2) new range can be merged with the previous one: XXXXYYY  ZZZZZ  =>   XXXXXXX  ZZZZZ
	// 3) new range can be merged with the next one:     XXXX  YYYZZZZZ  =>   XXXX  ZZZZZZZZ
	// 4) new range perfectly fills the gap:             XXXXYYYYYZZZZZ  =>   XXXXXXXXXXXXXX

	// note: we can have a case where we're inserting at the beginning of the array (no previous element), but we won't have a case
	// where we're inserting at the end (no next element) - AppendAndTryToMerge() should be called instead
	CHECK_WITH_LOG(!(item.offset < ranges[proposed_index].offset), 
		("RHI Error: VK_Section::InsertAndTryToMerge() was called to append an element - internal logic error, VK_Section::AppendAndTryToMerge() should have been called instead."))
	Int ret = proposed_index;
	if (proposed_index == 0)
	{
		VK_Section& next_range = ranges[ret];

		if (next_range.offset == item.offset + item.size)
		{
			next_range.offset = item.offset;
			next_range.size += item.size;
		}
		else
		{
			auto ret_it = ranges.insert(ranges.begin(),item);
			ret = 0;
		}
	}
	else
	{
		// all cases apply
		VK_Section& next_range = ranges[proposed_index];
		VK_Section& prev_range = ranges[proposed_index - 1];

		// see if we can merge with previous
		if (prev_range.offset + prev_range.size	 == item.offset)
		{
			// case 2, can still end up being case 4
			prev_range.size += item.size;

			if (prev_range.offset + prev_range.size == next_range.offset)
			{
				// case 4
				prev_range.size += next_range.size;
				auto find_res= std::find(ranges.begin(), ranges.end(), next_range);
				if(find_res!=ranges.end())
					ranges.erase(find_res);
				ret = proposed_index - 1;
			}
		}
		else if (item.offset + item.size == next_range.offset)
		{
			// case 3
			next_range.offset = item.offset;
			next_range.size += item.size;
		}
		else
		{
			// case 1 - the new range is disjoint with both
			ret = proposed_index;
			auto it = ranges.begin() + proposed_index;
			ranges.insert(it, item);	// this can invalidate NextRange/PrevRange references, don't touch them after this
		}
	}
#endif
#if VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS
	SanityCheck(ranges);
#endif
	return ret;
}

Int VK_Section::AppendAndTryToMerge(Vector<VK_Section>& ranges, CONST VK_Section& item)
{
#if !VK_MEMORY_JOIN_FREELIST_ON_THE_FLY
	ranges.push_back(item);
	Int ret = ranges.size() - 1;
#else
	Int ret = ranges.size() - 1;
	// we only get here when we have an element in front of us
	CHECK_WITH_LOG((ret < 0), 
		("RHI Error: VK_Section::AppendAndTryToMerge() was called on an empty array."))
	VK_Section& prev_range = ranges[ret];
	if (prev_range.offset + prev_range.size == item.offset)
	{
		prev_range.size += item.size;
	}
	else
	{
		ranges.push_back(item);
		ret = ranges.size() - 1;
	}
#endif
#if VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS
	SanityCheck(ranges);
#endif
	return ret;
}

void VK_Section::AllocateFromEntry(Vector<VK_Section>& ranges, Int index, UInt32 size_to_allocate)
{
	VK_Section& entry = ranges[index];
	if (size_to_allocate < entry.size)
	{
		// Modify current free entry in-place.
		entry.size -= size_to_allocate;
		entry.offset += size_to_allocate;
	}
	else
	{
		// Remove this free entry.
		auto it = ranges.begin() + index;
		ranges.erase(it);
#if VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS
		SanityCheck(ranges);
#endif
	}
}

void VK_Section::SanityCheck(Vector<VK_Section>& ranges)
{
#if VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS
	Int num_ranges = ranges.size();
	if (num_ranges <= 0)
	{
		return;

	}

	VK_Section* data = ranges.data();
	for (Int index = 0; index < num_ranges - 1; ++index)
	{
		CHECK_WITH_LOG((data[index].offset + data[index].size >= data[index + 1].offset), 
					("RHI Error: VK_Section::SanityCheck() failed :Ranges are overlapping or adjoining!"))
		CHECK_WITH_LOG((data[index].offset >= data[index + 1].offset),
			("RHI Error: VK_Section::SanityCheck() failed : Array is not sorted!"))
	}
#endif
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

