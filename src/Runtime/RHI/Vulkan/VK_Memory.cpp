#include"VK_Memory.h"

#include "VK_Define.h"
#include "VK_Device.h"
#include "VK_Utils.h"
#include "../../Core/ConstGlobals.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
    MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

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
VK_MemoryManager::VK_MemoryManager(VK_Device* in_device):device_memory_manager(in_device->GetDeviceMemoryManager())
{
    CONST UInt32 type_bit = (1<<device_memory_manager->GetMemoryTypeNum())-1;
    CONST VkPhysicalDeviceMemoryProperties& memory_properties = device_memory_manager->GetMemoryProperties();
    resource_heaps.resize(memory_properties.memoryTypeCount)

}
VK_DeviceMemoryManager* VK_MemoryManager::GetDeviceMemoryManager()
{
    return device_memory_manager;
}

VK_MemoryResourceHeap::VK_MemoryResourceHeap(VK_MemoryManager* InOwner, UInt32 InMemoryTypeIndex, UInt32 InOverridePageSize /*= 0*/)
: owner(InOwner),memory_type_index(InMemoryTypeIndex),override_page_size(InOverridePageSize)
{
    heap_index=owner->GetDeviceMemoryManager()->GetHeapIndex(memory_type_index);
}

VK_MemoryResourceHeap::~VK_MemoryResourceHeap()
{

}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE