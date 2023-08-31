#include"VK_Memory.h"

#include "VK_Define.h"
#include "VK_Device.h"
#include "VK_Utils.h"
#include "../../Core/ConstGlobals.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

#define VK_MEMORY_MAX_SUB_ALLOCATION (64llu << 20llu) //64MB?
UInt32 g_vulkan_budget_percentage_scale = 100;

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

Bool VK_DeviceMemoryAllocation::GetIsCanBeMapped() const
{
    return property.is_can_be_mapped;
}

Bool VK_DeviceMemoryAllocation::GetIsCoherent() const
{
    return property.is_coherent;
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
    void* dedicated_allocate_info, float priority, bool is_external, const char* file, UInt32 line)
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

VK_MemoryManager::VK_MemoryManager(VK_Device* in_device):device_memory_manager(in_device->GetDeviceMemoryManager())
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

Bool VK_MemoryManager::AllocateBufferPooled(VK_Allocation* out_allocation)
{

}

VK_DeviceMemoryManager* VK_MemoryManager::GetDeviceMemoryManager()
{
    return device_memory_manager;
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

Bool VK_MemoryResourceHeap::AllocateResource(VK_Allocation& out_allocation, VK_Evictable* allocation_owner, ENUM_VK_HeapAllocationType type, UInt32 size, UInt32 alignment, Bool is_map_allocation, Bool is_force_separate_allocation, ENUM_VK_AllocationMetaType meta_type, Bool is_external, CONST Char* file, UInt32 line)
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
					if (page->TryAllocate(out_allocation,allocation_owner,size,alignment,meta_type,file,line))
					{
	                    return true;
					}
                }
            }
        }

    }
    return false;
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

Bool VK_MemoryResourceFragmentAllocator::TryAllocate(VK_Allocation& out_allocation, VK_Evictable* owner, UInt32 in_size, UInt32 in_alignment, ENUM_VK_AllocationMetaType in_meta_type, CONST Char* file, UInt32 line)
{

}

UInt8 VK_MemoryResourceFragmentAllocator::GetSubresourceAllcatorFlags()
{
    return subresource_allocator_flags;
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

