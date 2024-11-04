#pragma once
#ifndef _CONSTDEFINE_
#define _CONSTDEFINE_

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <string>
#include <queue>
#include <stack>
#include <array>
#include <stdexcept>
#include <boost/stacktrace.hpp>
#include <iostream>
#include "Reflection.h"
#ifndef MYRENDERER_C_INTERFACE
#    ifdef __cplusplus
#        define MYRENDERER_C_INTERFACE 0
#    else
#        define MYRENDERER_C_INTERFACE 1
#    endif
#endif

#ifdef _MSC_VER
#    define MYRENDERER_CALL_TYPE __cdecl
#else
#    define MYRENDERER_CALL_TYPE
#endif

#if UINTPTR_MAX == UINT64_MAX
#    define MYRENDERER_PLATFORM_64 1
#elif UINTPTR_MAX == UINT32_MAX
#    define MYRENDERER_PLATFORM_32 1
#else
#    pragma error Unexpected value of UINTPTR_MAX
#endif


#define MYRENDERER_BEGIN_NAMESPACE(Name) \
        namespace Name                 \
        {

#define MYRENDERER_END_NAMESPACE }

#define MYRENDERER_PUBLIC_DERIVE(TypeName)  public TypeName 
        

#define MYRENDERER_BEGIN_TYPED_ENUM(EnumName, EnumType) enum EnumName : EnumType \
                                {
#define MYRENDERER_END_TYPED_ENUM };

#define MYRENDERER_INITIALIZER(x) = x

#define MYRENDERER_GLOBAL_FUNCTION(FuncName) FuncName

#if defined(__REFLECTION_PARSER__)
#define META(...) __attribute__((annotate(#__VA_ARGS__)))
#define MYRENDERER_BEGIN_STRUCT(Name, ...) struct __attribute__((annotate(#__VA_ARGS__))) Name  \
                                    {
#define MYRENDERER_BEGIN_CLASS(Name, ...) class __attribute__((annotate(#__VA_ARGS__))) Name  \
                                    {
#define MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Name, ...) class __attribute__((annotate(#__VA_ARGS__))) Name : __VA_ARGS__ \
                                        {
#else
#define META(...)
#define MYRENDERER_BEGIN_STRUCT(Name, ...) struct Name  \
                                    {
#define MYRENDERER_BEGIN_CLASS(Name, ...) class Name  \
                                    { 
#define MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Name, ...) class Name : __VA_ARGS__ \
                                        {
#endif
#define MYRENDERER_END_CLASS  };

#define MYRENDERER_END_STRUCT };

#define MYRENDERER_TEMPLATE_HEAD(...)  template<__VA_ARGS__>

#define MYRENDERER_VALUE(x) = x

#define CHECK(Flag) if(Flag) { std::abort(); }
#define CHECK_WITH_LOG(Flag,LOG) if(Flag) \
                                {\
                                    boost::stacktrace::stacktrace stack_trace;\
									String log = String(LOG)+"\n"+boost::stacktrace::to_string(stack_trace);\
									std::cout<<log<<std::endl; \
                                   	throw std::runtime_error(String(LOG)+"\n"+boost::stacktrace::to_string(stack_trace)); \
                                }
#define CHECK_WITH_LOG_WARNING(Flag,LOG) if(Flag) \
                                {\
                                    boost::stacktrace::stacktrace stack_trace;\
                                   	std::cout<< String(LOG)+"\n"+boost::stacktrace::to_string(stack_trace) <<std::endl; \
                                }
#define VIRTUAL      virtual
#define CONST        const
#define MYDEFAULT      =default
#define PURE         = 0

#define MYDELETE       = delete

#define OVERRIDE      override
#define FINAL        final
#define REF          &
#define METHOD(Name) MYRENDERER_CALL_TYPE Name

#define STATIC_CAST(Pointer,Type) (static_cast<Type*>(Pointer))
#define DYNAMIC_CAST(Pointer,Type) (dynamic_cast<Type>(Pointer))
#define REINTERPRET_CAST(Pointer,Type) (reinterpret_cast<Type*>(Pointer))

using UInt8 = std::uint8_t;
using UInt16 = std::uint16_t;
using UInt32 = std::uint32_t;
using UInt64 = std::uint64_t;
using Int8 = std::int8_t;
using Int16 = std::int16_t;
using Int = std::int32_t;
using Int64 = std::int64_t;
using Bool = bool;
using Char = char;
using String = std::string;
using Text = std::string;
using Float32 = float;
using Float64 = double;

template <typename T>
using Vector = std::vector<T>; 

template <typename K,typename T, typename Hash = std::hash<K>>
using Map = std::unordered_map<K,T, Hash>;

template <typename T>
using Optional = std::optional<T>;

template <typename T>
using Set = std::unordered_set<T>;

template <typename T>
using Queue =std::queue<T>;

template <typename T>
using Stack = std::stack<T>;

template <class T, size_t Size>
using Array = std::array<T, Size>;

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#define CORE_API DLLEXPORT
#define TRACELOG_API DLLIMPORT
#define DESKTOPPLATFORM_API DLLIMPORT
#define ANALYTICS_API DLLIMPORT
#define DIRECTORYWATCHER_API DLLIMPORT


#define ENUM_CLASS_FLAGS(Enum) \
	inline           Enum& operator|=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator&=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator^=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator| (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator& (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator^ (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr Bool  operator! (Enum  E)             { return !(__underlying_type(Enum))E; } \
	inline constexpr Enum  operator~ (Enum  E)             { return (Enum)~(__underlying_type(Enum))E; }

// Friends all bitwise operators for enum classes so the definition can be kept private / protected.
#define FRIEND_ENUM_CLASS_FLAGS(Enum) \
	friend           Enum& operator|=(Enum& Lhs, Enum Rhs); \
	friend           Enum& operator&=(Enum& Lhs, Enum Rhs); \
	friend           Enum& operator^=(Enum& Lhs, Enum Rhs); \
	friend constexpr Enum  operator| (Enum  Lhs, Enum Rhs); \
	friend constexpr Enum  operator& (Enum  Lhs, Enum Rhs); \
	friend constexpr Enum  operator^ (Enum  Lhs, Enum Rhs); \
	friend constexpr bool  operator! (Enum  E); \
	friend constexpr Enum  operator~ (Enum  E);

template<typename Enum>
constexpr bool EnumHasAllFlags(Enum Flags, Enum Contains)
{
	using UnderlyingType = __underlying_type(Enum);
	return ((UnderlyingType)Flags & (UnderlyingType)Contains) == (UnderlyingType)Contains;
};

template<typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, Enum Contains)
{
	using UnderlyingType = __underlying_type(Enum);
	return ((UnderlyingType)Flags & (UnderlyingType)Contains) != 0;
};

template<typename Enum>
void EnumAddFlags(Enum & Flags, Enum FlagsToAdd)
{
	using UnderlyingType = __underlying_type(Enum);
	Flags = (Enum)((UnderlyingType)Flags | (UnderlyingType)FlagsToAdd);
};

template<typename Enum>
void EnumRemoveFlags(Enum& Flags, Enum FlagsToRemove)
{
	using UnderlyingType = __underlying_type(Enum);
	Flags = (Enum)((UnderlyingType)Flags & ~(UnderlyingType)FlagsToRemove);
};


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

	return (T)(((UInt64)Val + Alignment - 1) & ~(Alignment - 1));
};

template <bool AllowTouch, typename T>
bool CheckLineSectionOverlap(T Min0, T Max0, T Min1, T Max1)
{

	//     [------]         [------]
	//   Min0    Max0    Min1     Max1
	//
	//     [------]         [------]
	//   Min1    Max1    Min0     Max0
	if (AllowTouch)
	{
		return !(Min0 > Max1 || Min1 > Max0);
	}
	else
	{
		return !(Min0 >= Max1 || Min1 >= Max0);
	}
};
#endif 
