#pragma once
#ifndef _RESOURCEHANDLE_
#define _RESOURCEHANDLE_

#include "ConstDefine.h"
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)

// -- 32-bit Handle λ -- --
// | 31 ... 12 | 11 ... 0 |
// |   index   |   gen    |
// |  20 bits  |  12 bits |
using GenericHandle = UInt32;
static constexpr GenericHandle kInvalidHandle = 0xFFFFFFFF;
static constexpr UInt32 kHandleIndexBits = 20;
static constexpr UInt32 kHandleGenBits   = 12;
static constexpr UInt32 kHandleIndexMask = (1 << kHandleIndexBits) - 1;
static constexpr UInt32 kHandleGenMask   = (1 << kHandleGenBits) - 1;

// --  --  引擎  --  Handle  --  --
// Tag  --  --  --  --  --  --   enum class  --  --  --  --  --  --  --
// Core  --  --  --  --  --  --  --  --
MYRENDERER_TEMPLATE_HEAD(typename Tag)
struct ResourceHandle
{
	GenericHandle value = kInvalidHandle;

	Bool   METHOD(IsValid)()      CONST { return value != kInvalidHandle; }
	UInt32 METHOD(GetIndex)()     CONST { return (value >> kHandleGenBits) & kHandleIndexMask; }
	UInt32 METHOD(GetGeneration)() CONST { return value & kHandleGenMask; }

	static ResourceHandle METHOD(Make)(UInt32 index, UInt32 generation)
	{
		ResourceHandle h;
		h.value = ((index & kHandleIndexMask) << kHandleGenBits) | (generation & kHandleGenMask);
		return h;
	}

	Bool operator==(CONST ResourceHandle& rhs) CONST { return value == rhs.value; }
	Bool operator!=(CONST ResourceHandle& rhs) CONST { return value != rhs.value; }
};

// Hash  --  --  Map<Handle, T>  --  --
MYRENDERER_TEMPLATE_HEAD(typename Tag)
struct ResourceHandleHash
{
	size_t operator()(ResourceHandle<Tag> h) const { return std::hash<UInt32>{}(h.value); }
};

//  --  --  --  --  --  --  GenericHandle (raw UInt32)  --  --  --  --
inline UInt32 GetHandleIndex(GenericHandle h) { return (h >> kHandleGenBits) & kHandleIndexMask; }
inline UInt32 GetHandleGeneration(GenericHandle h) { return h & kHandleGenMask; }
inline Bool IsHandleValid(GenericHandle h) { return h != kInvalidHandle; }

MYRENDERER_END_NAMESPACE
#endif
