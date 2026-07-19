#pragma once
#ifndef _RESOURCEREGISTRY_
#define _RESOURCEREGISTRY_

#include "ResourceHandle.h"
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)

// -- 稀疏 --  --  --  --  --  --
//  --  --  T*  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --
MYRENDERER_TEMPLATE_HEAD(typename T)
struct ResourceSlot
{
	UInt32 generation = 0;
	T*     ptr = nullptr;
	String debug_name;
	UInt32 next_free = 0;  //  --  --  --  --  --  -- 0 =  --  --  --
};

// --  --  --  --  --  --  --  --  --  --  O(1)  --  -- / --  -- / --  --  --
//  --  --  --  --  --  Vector<T> / Map<K,V>  --  --  --  --  --  --  --  --
//  --  --  T*  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --
MYRENDERER_TEMPLATE_HEAD(typename T)
MYRENDERER_BEGIN_CLASS(ResourceRegistry)
#pragma region METHOD
public:
	ResourceRegistry(UInt32 initial_capacity = 256)
	{
		slots.resize(initial_capacity);
		for (UInt32 i = 1; i < initial_capacity - 1; ++i)
			slots[i].next_free = i + 1;
		slots[initial_capacity - 1].next_free = 0;
		free_head = 1;
	}

	//  --  --  --  --  --  --  handle  --  --  --  --  --  T*  --  --  --  --  --  --
	GenericHandle METHOD(Allocate)(T* ptr, CONST String& debug_name)
	{
		if (free_head == 0) Grow();
		UInt32 idx = free_head;
		auto REF slot = slots[idx];
		free_head = slot.next_free;

		UInt32 gen = slot.generation;
		slot.ptr = ptr;
		slot.debug_name = debug_name;
		slot.next_free = 0;

		return ((idx & kHandleIndexMask) << kHandleGenBits) | (gen & kHandleGenMask);
	}

	//  --  --  --  --  --  -- bump generation  --  --  --  --  --  --  handle  --  --  --
	//  --  --  --  T*  --  --  --  --  --  --  --  --  --  --  --  --
	T* METHOD(Free)(GenericHandle handle)
	{
		UInt32 idx = handle >> kHandleGenBits;
		UInt32 gen = handle & kHandleGenMask;
		if (idx == 0 || idx >= slots.size()) return nullptr;
		auto REF slot = slots[idx];
		if (slot.generation != gen) return nullptr; //  --  --  --  --  handle

		T* old_ptr = slot.ptr;
		slot.ptr = nullptr;
		slot.debug_name.clear();
		slot.generation = ((gen + 1) & kHandleGenMask);
		if (slot.generation == 0) slot.generation = 1;
		slot.next_free = free_head;
		free_head = idx;
		return old_ptr;
	}

	//  --  --  --  --  --  --  --  --  handle  --  --  --  --  --  nullptr
	T* METHOD(Resolve)(GenericHandle handle) CONST
	{
		UInt32 idx = handle >> kHandleGenBits;
		UInt32 gen = handle & kHandleGenMask;
		if (idx == 0 || idx >= slots.size()) return nullptr;
		CONST auto REF slot = slots[idx];
		if (slot.generation != gen) return nullptr;
		return slot.ptr;
	}

	//  --  --  --  --  --  --  --  --  --  --
	void METHOD(ForEach)(std::function<void(GenericHandle, T*)> callback) CONST
	{
		for (UInt32 i = 1; i < slots.size(); ++i)
		{
			if (slots[i].ptr)
			{
				GenericHandle h = ((i & kHandleIndexMask) << kHandleGenBits) | (slots[i].generation & kHandleGenMask);
				callback(h, slots[i].ptr);
			}
		}
	}

	UInt32 METHOD(GetActiveCount)() CONST
	{
		UInt32 count = 0;
		for (UInt32 i = 1; i < slots.size(); ++i)
			if (slots[i].ptr) ++count;
		return count;
	}

	UInt32 METHOD(GetCapacity)() CONST { return (UInt32)slots.size(); }

	//  --  --  --  --  --  --  --  --（ --  --  --   generation  --  --）
	//  --  --  Editor Panel  --  --  ax::NodeEditor  --  UInt64 ID  --  --  --
	T* METHOD(FindByIndex)(UInt32 index) CONST
	{
		if (index == 0 || index >= slots.size()) return nullptr;
		return slots[index].ptr;
	}

protected:
	void METHOD(Grow)()
	{
		UInt32 old_size = (UInt32)slots.size();
		UInt32 new_size = old_size * 2;
		slots.resize(new_size);
		for (UInt32 i = old_size; i < new_size - 1; ++i)
		{
			slots[i].generation = 1;
			slots[i].next_free = i + 1;
		}
		slots[new_size - 1].generation = 1;
		slots[new_size - 1].next_free = 0;
		free_head = old_size;
	}
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
	Vector<ResourceSlot<T>> slots;
	UInt32 free_head = 0;
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
#endif
