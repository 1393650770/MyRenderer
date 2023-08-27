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
class VK_MemoryManager;
class VK_MemoryResourceHeap;
class VK_MemoryResourceFragmentAllocator;

enum class ENUM_VK_AllocationType : UInt8
{
	EVulkanAllocationEmpty=0,
	EVulkanAllocationPooledBuffer,
	EVulkanAllocationBuffer,
	EVulkanAllocationImage,
	EVulkanAllocationImageDedicated,
};

enum class ENUM_VK_AllocationMetaType : UInt8
{
	EVulkanAllocationMetaUnknown=0,
	EVulkanAllocationMetaUniformBuffer,
	EVulkanAllocationMetaMultiBuffer,
	EVulkanAllocationMetaRingBuffer,
	EVulkanAllocationMetaFrameTempBuffer,
	EVulkanAllocationMetaImageRenderTarget,
	EVulkanAllocationMetaImageOther,
	EVulkanAllocationMetaBufferUAV,
	EVulkanAllocationMetaBufferStaging,
	EVulkanAllocationMetaBufferOther,
	EVulkanAllocationMetaSize,
};

enum
{
	GPU_ONLY_HEAP_PAGE_SIZE = 128 * 1024 * 1024,
	STAGING_HEAP_PAGE_SIZE = 32 * 1024 * 1024,
	ANDROID_MAX_HEAP_PAGE_SIZE = 16 * 1024 * 1024,
	ANDROID_MAX_HEAP_IMAGE_PAGE_SIZE = 16 * 1024 * 1024,
	ANDROID_MAX_HEAP_BUFFER_PAGE_SIZE = 4 * 1024 * 1024,
};



MYRENDERER_BEGIN_STRUCT(MemoryHeapInfo)
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

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Evictable,public RenderResource)
#pragma region METHOD
public: 

#pragma endregion
MYRENDERER_END_CLASS

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
    CONST UInt32 METHOD(GetMemoryTypeNum)() CONST;
    UInt32 METHOD(GetHeapIndex)(UInt32 memory_type_index);
    CONST VkPhysicalDeviceMemoryProperties& METHOD(GetMemoryProperties)() CONST;
    VkResult METHOD(GetMemoryTypeFromProperties)(UInt32 type_bits, VkMemoryPropertyFlags properties, UInt32* out_type_index);
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
    Vector<MemoryHeapInfo> memory_heaps;
    Map<MemoryBlockKey,MemoryBlock> memory_block_map;

private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_STRUCT(VK_VulkanPageSizeBucket)
UInt64 max_allocation=0;
UInt32 page_size=0;
enum
{
	BUCKET_MASK_IMAGE = 0x1,
	BUCKET_MASK_BUFFER = 0x2,
};
UInt32 bucket_mask= BUCKET_MASK_BUFFER;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(VK_Section)

MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_MemoryResourceHeap, public RenderResource)
friend class VK_MemoryManager;
friend class VK_MemoryResourceFragmentAllocator;

#pragma region METHOD
public:
    VK_MemoryResourceHeap(VK_MemoryManager* in_owner, UInt32 InMemoryTypeIndex, UInt32 InOverridePageSize = 0);
    VIRTUAL ~VK_MemoryResourceHeap();
protected:

private:

#pragma endregion 
#pragma region MEMBER
public:

protected:
	enum {
		MAX_BUCKETS = 5,
	};
    Vector<VK_VulkanPageSizeBucket> page_size_buckets;
    VK_MemoryManager* owner;
    UInt32 memory_type_index=0;
    UInt32 override_page_size=0;
    UInt16 heap_index=0;
    Bool is_support_host_cached=false;
    Bool is_support_lazily_allocated=false;
    UInt8 defrag_count_down=0;
    UInt32 peak_page_size = 0;
    UInt64 used_memory=0;
    UInt64 page_id_counter=0;

    Vector<VK_MemoryResourceFragmentAllocator*> active_pages[MAX_BUCKETS];
    Vector<VK_MemoryResourceFragmentAllocator*> used_dedicated_pages;
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Allocation,public RenderResource)
#pragma region METHOD
public:
VK_Allocation() DEFAULT;
VIRTUAL ~VK_Allocation() DEFAULT;
protected:

private:

#pragma endregion 
#pragma region MEMBER
public:

protected:

private:

#pragma endregion 

MYRENDERER_END_CLASS


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_MemoryManager,public RenderResource)
friend class VK_MemoryResourceHeap;

#pragma region METHOD
public:
    VK_MemoryManager(VK_Device* in_device);
    VIRTUAL ~VK_MemoryManager() DEFAULT;

    Bool METHOD(AllocateBufferPooled)(VK_Allocation* out_allocation);
    Bool METHOD(AllocateImageMemory)(VK_Allocation* out_allocation);
    Bool METHOD(AllocateBufferMemory)(VK_Allocation* out_allocation);
    Bool METHOD(AllocateDedicatedImageMemory)(VK_Allocation* out_allocation);
    Bool METHOD(AllocateUniformBuffer)(VK_Allocation* out_allocation);

    VK_DeviceMemoryManager* METHOD(GetDeviceMemoryManager)();
protected:

private:
#pragma endregion 

#pragma region MEMBER
public:

protected:
    VK_DeviceMemoryManager* device_memory_manager;

    Vector<VK_MemoryResourceHeap*> resource_heaps;
private:
#pragma endregion 

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
