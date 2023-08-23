#include"VK_Memory.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)
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
}

VK_DeviceMemoryManager::~VK_DeviceMemoryManager()
{
}

VK_DeviceMemoryAllocation* VK_DeviceMemoryManager::Alloc(VkDeviceSize allocation_size, UInt32 memory_type_index,
    void* dedicated_allocate_info, float priority, bool is_external, const char* file, UInt32 line)
{
    if (dedicated_allocate_info==nullptr)
    {
        
    }
}

void VK_DeviceMemoryManager::Free(VK_DeviceMemoryAllocation*& allocation)
{
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE