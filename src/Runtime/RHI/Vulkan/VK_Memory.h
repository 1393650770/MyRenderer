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
class VK_Allocation;

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

enum class ENUM_VK_HeapAllocationType :UInt32
{
    None=0,
    Image,
    Buffer,
    Count
};

enum ENUM_VK_LegacyVulkanAllocationFlags
{
	VulkanAllocationFlagsMapped = 0x1,
	VulkanAllocationFlagsCanEvict = 0x2,
};

enum class ENUM_VulkanAllocationFlags : UInt16
{
	None = 0x0000,

	HostVisible = 0x0001,    // Will be written from CPU, will likely contain the HOST_VISIBLE + HOST_COHERENT flags
	HostCached = 0x0002,    // Will be used for readback, will likely contain the HOST_CACHED flag.  Implies HostVisible.
	PreferBAR = 0x0004,    // Will be allocated from a HOST_VISIBLE + DEVICE_LOCAL heap if available and possible (HOST_VISIBLE if not)

	Dedicated = 0x0008,	  // Will not share a memory block with other resources
	External = 0x0010,    // To be used with VK_KHR_external_memory
	Memoryless = 0x0020,	  // Will use LAZILY_ALLOCATED

	NoError = 0x0040,    // OOM is not fatal, return an invalid allocation
	AutoBind = 0x0080,	  // Will automatically bind the allocation to the supplied VkBuffer/VkImage, avoids an extra lock to bind separately
};
ENUM_CLASS_FLAGS(ENUM_VulkanAllocationFlags)

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
    VkResult METHOD(GetMemoryTypeFromPropertiesExcluding)(UInt32 TypeBits, VkMemoryPropertyFlags Properties, UInt32 ExcludeTypeIndex, UInt32* OutTypeIndex);
    Bool METHOD(GetIsSupportUnifiedMemory)() CONST;
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
	Bool is_support_unified_memory=false;
    Bool is_support_memory_less=false;

private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_STRUCT(VK_VulkanPageSizeBucket)

    UInt64 max_allocation = 0;
    UInt32 page_size = 0;
    enum
    {
	    BUCKET_MASK_IMAGE = 0x1,
	    BUCKET_MASK_BUFFER = 0x2,
    };
    UInt32 bucket_mask = BUCKET_MASK_BUFFER;

MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(VK_Section)
    UInt32 offset;
    UInt32 size;

    Bool operator<(CONST VK_Section& in) CONST
    {
	    return offset < in.offset;
    }

    static void MergeConsecutiveRanges(Vector<VK_Section>& Ranges);

    /** Tries to insert the item so it has index ProposedIndex, but may end up merging it with neighbors */
    static Int InsertAndTryToMerge(Vector<VK_Section>& Ranges, CONST VK_Section& Item, Int ProposedIndex);

    /** Tries to append the item to the end but may end up merging it with the neighbor */
    static Int AppendAndTryToMerge(Vector<VK_Section>& Ranges, CONST VK_Section& Item);

    /** Attempts to allocate from an entry - can remove it if it was used up*/
    static void AllocateFromEntry(Vector<VK_Section>& Ranges, Int Index, UInt32 SizeToAllocate);

    /** Sanity checks an array of ranges */
    static void SanityCheck(Vector<VK_Section>& Ranges);

    /** Adds to the array while maintaing the sort. */
    static Int Add(Vector<VK_Section>& Ranges, CONST VK_Section& Item);

MYRENDERER_END_STRUCT


/// <summary>
/// 用来记录信息，方便做迁移和碎片整理
/// </summary>
MYRENDERER_BEGIN_STRUCT(VK_AllocationInternalInfo)
    enum class ENUM_VK_AllocationState :Int
    {
	    EUNUSED,
	    EALLOCATED,
	    EFREED,
	    EFREEPENDING,
	    EFREEDISCARDED, // if a defrag happens with a pending free, its put into this state, so we can ignore the deferred delete once it happens.
    };


    void METHOD(Init)(CONST VK_Allocation& alloc, VK_Evictable* in_allocation_owner, UInt32 allocation_offset, UInt32 allocation_size, UInt32 alignment);

    ENUM_VK_AllocationState state = ENUM_VK_AllocationState::EUNUSED;
    ENUM_VK_AllocationType type = ENUM_VK_AllocationType::EVulkanAllocationEmpty;
    ENUM_VK_AllocationMetaType meta_type = ENUM_VK_AllocationMetaType::EVulkanAllocationMetaUnknown;

    UInt32 size = 0;
    UInt32 allocation_size = 0;
    UInt32 allocation_offset = 0;
    VK_Evictable* allocation_owner = 0;
    UInt32 alignment = 0;

    Int next_free = -1;

MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS(VK_MemoryResourceFragmentAllocator)
#pragma region METHOD
public:
    VK_MemoryResourceFragmentAllocator();
    VIRTUAL ~VK_MemoryResourceFragmentAllocator();

    Bool METHOD(TryAllocate)(VK_Allocation& out_allocation, VK_Evictable* owner, UInt32 in_size, UInt32 in_alignment, ENUM_VK_AllocationMetaType in_meta_type);
    void METHOD(Free)(VK_Allocation& allocation);
    void METHOD(Destroy)(VK_Device* device);
    UInt8 METHOD(GetSubresourceAllcatorFlags)();

protected:
    Int METHOD(AllocateInternalData)();
    void METHOD(FreeInternalData)(Int index);
    Bool METHOD(MergeFreeBlocks)();
private:

#pragma endregion

#pragma region MEMBER
public:
protected:
    UInt32 memory_used[(UInt32)ENUM_VK_AllocationMetaType::EVulkanAllocationMetaSize];
    ENUM_VK_AllocationType type;
    VK_MemoryManager* owner_memory_manager = nullptr;
    VkMemoryPropertyFlags memory_property_flags;
    UInt32 max_size=0;
    UInt32 alignment=0;
    UInt32 frame_freed=0;
    UInt32 last_defrag_frame=0;
    Int64 used_size=0;
	VkBufferUsageFlags buffer_usage_flags;
	VkBuffer buffer;
    UInt32 buffer_id;
    Int pool_size_index;
    UInt32 allocator_index;
    UInt8 subresource_allocator_flags;
    UInt8 bucket_id;
	Bool is_evicting = false;
    Bool is_locked = false;
    Bool is_defragging = false;

    UInt32 num_sub_allocations = 0;
    UInt32 alloc_calls = 0;
    UInt32 free_calls = 0;
    Vector<VK_Section> free_list;
    Vector<VK_AllocationInternalInfo> internal_data;
    Int internal_free_listnode_index=-1;
private:

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(VK_MemoryResourceHeap)
friend class VK_MemoryManager;
friend class VK_MemoryResourceFragmentAllocator;

#pragma region METHOD
public:
    VK_MemoryResourceHeap(VK_MemoryManager* in_owner, UInt32 InMemoryTypeIndex, UInt32 InOverridePageSize = 0);
    VIRTUAL ~VK_MemoryResourceHeap();

    UInt32 METHOD(GetPageSizeBucket)(VK_VulkanPageSizeBucket& out_bucket,ENUM_VK_HeapAllocationType type, UInt32 allocation_size,Bool is_force_single_allocation);
protected:
    Bool METHOD(TryRealloc)(VK_Allocation& out_allocation);
    Bool METHOD(AllocateResource)(VK_Allocation& out_allocation,  VK_Evictable* allocation_owner, ENUM_VK_HeapAllocationType type, UInt32 size, UInt32 alignment, Bool is_map_allocation, Bool is_force_separate_allocation, ENUM_VK_AllocationMetaType meta_type, Bool is_external);
    Bool METHOD(AllocateDedicatedImage)(VK_Allocation& out_allocation,  VK_Evictable* allocation_owner, ENUM_VK_HeapAllocationType type, UInt32 size, UInt32 alignment, Bool is_map_allocation, Bool is_force_separate_allocation, ENUM_VK_AllocationMetaType meta_type, Bool is_external);
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
    void METHOD(Init)(ENUM_VK_AllocationType Type, ENUM_VK_AllocationMetaType MetaType, UInt64 Handle, UInt32 InSize, UInt32 AlignedOffset, UInt32 AllocatorIndex, UInt32 AllocationIndex, UInt32 BufferId);
    void METHOD(Free)(VK_Device& Device);
    void Swap(VK_Allocation& Other);
    void Reference(CONST VK_Allocation& Other); //point to other, but don't take ownership
    bool HasAllocation();

    void Disown(); //disown & own should be used if ownership is transferred.
    void Own();
    bool IsValid() CONST;

    inline ENUM_VK_AllocationType METHOD(GetType)() CONST
    {
	    return (ENUM_VK_AllocationType)type;
    }
    inline void METHOD(SetType)(ENUM_VK_AllocationType in_type)
    {
	    type = (UInt8)in_type;
    }

    void* GetMappedPointer(VK_Device* Device);
    void FlushMappedMemory(VK_Device* Device);
    void InvalidateMappedMemory(VK_Device* Device);
    VkBuffer GetBufferHandle() const;
    UInt32 GetBufferAlignment(VK_Device* Device) const;
    VkDeviceMemory GetDeviceMemoryHandle(VK_Device* Device) const;
    VK_MemoryResourceFragmentAllocator* GetSubresourceAllocator(VK_Device* Device) const;
    void BindBuffer(VK_Device* Device, VkBuffer Buffer);
    void BindImage(VK_Device* Device, VkImage Image);

protected:

private:

#pragma endregion 
#pragma region MEMBER
public:
	UInt64 vulkan_handle = 0;
    UInt32 handle_id = 0;
    UInt32 size = 0;
    UInt32 offset = 0;
    UInt32 allocation_index = 0;
    UInt16 allocator_index = 0;
	ENUM_VK_AllocationMetaType meta_type = ENUM_VK_AllocationMetaType::EVulkanAllocationMetaUnknown;
    //type= ENUM_VK_AllocationType
    UInt8 type : 7;
    UInt8 is_has_owner_ship : 1;

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

    //Bool METHOD(AllocateBufferPooled)(VK_Allocation* out_allocation);
    //Bool METHOD(AllocateImageMemory)(VK_Allocation* out_allocation);
    Bool METHOD(AllocateBufferMemory)(VK_Allocation& out_allocation, VkBuffer InBuffer, ENUM_VulkanAllocationFlags InAllocFlags, UInt32 InForceMinAlignment = 1);
    //Bool METHOD(AllocateDedicatedImageMemory)(VK_Allocation* out_allocation);
    //Bool METHOD(AllocateUniformBuffer)(VK_Allocation* out_allocation);

    VK_DeviceMemoryManager* METHOD(GetDeviceMemoryManager)();
protected:

private:
#pragma endregion 

#pragma region MEMBER
public:

protected:
    VK_DeviceMemoryManager* device_memory_manager;
    VK_Device* device;
    Vector<VK_MemoryResourceHeap*> resource_heaps;
private:
#pragma endregion 

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
