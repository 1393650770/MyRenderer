#pragma once

#ifndef _CONSTGLOBALS_
#define _CONSTGLOBALS_

#include "ConstDefine.h"

extern CORE_API UInt32 g_frame_number_render_thread;

template <typename T>
struct TIsIntegral
{
	enum { Value = false };
};
template <> struct TIsIntegral<         bool> { enum { Value = true }; };
template <> struct TIsIntegral<         char> { enum { Value = true }; };
template <> struct TIsIntegral<signed   char> { enum { Value = true }; };
template <> struct TIsIntegral<unsigned char> { enum { Value = true }; };
template <> struct TIsIntegral<         char16_t> { enum { Value = true }; };
template <> struct TIsIntegral<         char32_t> { enum { Value = true }; };
template <> struct TIsIntegral<         wchar_t> { enum { Value = true }; };
template <> struct TIsIntegral<         short> { enum { Value = true }; };
template <> struct TIsIntegral<unsigned short> { enum { Value = true }; };
template <> struct TIsIntegral<         int> { enum { Value = true }; };
template <> struct TIsIntegral<unsigned int> { enum { Value = true }; };
template <> struct TIsIntegral<         long> { enum { Value = true }; };
template <> struct TIsIntegral<unsigned long> { enum { Value = true }; };
template <> struct TIsIntegral<         long long> { enum { Value = true }; };
template <> struct TIsIntegral<unsigned long long> { enum { Value = true }; };
template <typename T> struct TIsIntegral<const          T> { enum { Value = TIsIntegral<T>::Value }; };
template <typename T> struct TIsIntegral<      volatile T> { enum { Value = TIsIntegral<T>::Value }; };
template <typename T> struct TIsIntegral<const volatile T> { enum { Value = TIsIntegral<T>::Value }; };



template <typename T>
struct TIsPointer
{
	enum { Value = false };
};
template <typename T> struct TIsPointer<T*> { enum { Value = true }; };
template <typename T> struct TIsPointer<const          T> { enum { Value = TIsPointer<T>::Value }; };
template <typename T> struct TIsPointer<      volatile T> { enum { Value = TIsPointer<T>::Value }; };
template <typename T> struct TIsPointer<const volatile T> { enum { Value = TIsPointer<T>::Value }; };

template <typename T>
FORCEINLINE constexpr T Align(T Val, UInt64 Alignment)
{
	static_assert(TIsIntegral<T>::Value || TIsPointer<T>::Value, "Align expects an integer or pointer type");

	return (T)(((uint64)Val + Alignment - 1) & ~(Alignment - 1));
}
#endif 
