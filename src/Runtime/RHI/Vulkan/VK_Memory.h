#pragma once
#ifndef _VK_MEMORY_
#define _VK_MEMORY_

#include <vulkan/vulkan_core.h>

#include "../../Core/ConstDefine.h"
#include "optick.h"
#include "../RenderRource.h"
#include "../../Core/TypeHash.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;
class VK_DeviceMemoryAllocation;
class VK_DeviceMemoryManager;

MYRENDERER_BEGIN_STRUCT(MemoryHeap)
VkDeviceSize used_size=0;
VkDeviceSize max_size=0;
Vector<VK_DeviceMemoryAllocation*> allocations;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(MemoryBlockKey)
UInt32 memory_type_index=0;
VkDeviceSize block_size=0;
bool operator==(const MemoryBlockKey& other) const
{
    return memory_type_index == other.memory_type_index&& block_size == other.block_size;
}
size_t operator()(const MemoryBlockKey & p) const{
    return HashCombine(std::hash<UInt32>()(p.block_size) , std::hash<UInt32>()(p.memory_type_index));
}
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(MemoryBlock)
MemoryBlockKey key;

    MYRENDERER_BEGIN_STRUCT(FreeBlock)
    VK_DeviceMemoryAllocation* allocation;
    UInt32 frame_freed;
    MYRENDERER_END_STRUCT

Vector<FreeBlock> allocations;

MYRENDERER_END_STRUCT


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_DeviceMemoryAllocation,public RenderResource)
friend  VK_DeviceMemoryManager;

#pragma region METHOD
public:
VK_DeviceMemoryAllocation();
VIRTUAL ~VK_DeviceMemoryAllocation() DEFAULT;

void* METHOD(Map)(VkDeviceSize in_size,VkDeviceSize offset);
void METHOD(UnMap)();
VkDeviceMemory METHOD(GetMemory)();
VkDeviceSize METHOD(GetSize)();
void* METHOD(GetMappedPointer)();
void METHOD(FlushMappedMemory)(VkDeviceSize in_size,VkDeviceSize offset);
void METHOD(InvalidateMappedMemory)(VkDeviceSize in_size,VkDeviceSize offset);
Bool METHOD(GetIsCanBeMapped)() const;
Bool METHOD(GetIsCoherent)()const;
protected:
    
private:

#pragma endregion

#pragma region MEMBER
public:
union MemoryProperty
{
    struct
    {
        UInt16 memory_type_index : 8;
        UInt16 is_can_be_mapped : 1;
        UInt16 is_coherent : 1;
        UInt16 is_cached : 1;
        UInt16 is_freed_by_system : 1;
        UInt16 is_dedicated_memory : 1;
        
    };
    UInt16 packed;
};
protected:
VkDeviceSize size= 0;
VkDevice device_handle=VK_NULL_HANDLE;
VkDeviceMemory memory=VK_NULL_HANDLE;
void* mapped_pointer=nullptr;

MemoryProperty property;

private:

#pragma endregion
MYRENDERER_END_CLASS



MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_DeviceMemoryManager,public RenderResource)

#pragma region METHOD
public: 
VK_DeviceMemoryManager(VK_Device* in_device);
VIRTUAL~VK_DeviceMemoryManager() OVERRIDE;

VK_DeviceMemoryAllocation* METHOD(Alloc)(VkDeviceSize allocation_size, UInt32 memory_type_index, void* dedicated_allocate_info, float priority, bool is_external, const char* file, UInt32 line);

void METHOD(Free)(VK_DeviceMemoryAllocation*& allocation);

protected:
    
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
VK_Device* device=nullptr;
UInt32 num_allocations=0;
UInt32 max_num_allocations=0;
VkPhysicalDeviceMemoryBudgetPropertiesEXT memory_budget;
VkPhysicalDeviceMemoryProperties memory_properties;
Int primary_heap_index=0;
Bool is_support_lazily_allocated=false;
Vector<MemoryHeap> memory_heaps;
Map<MemoryBlockKey,MemoryBlock> memory_block_map;

private:

#pragma endregion


MYRENDERER_END_CLASS


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_MemoryManager,public RenderResource)

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
