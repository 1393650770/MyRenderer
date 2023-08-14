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
#include <stdexcept>

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


#define MYRENDERER_BEGIN_STRUCT(Name) struct Name  \
                                    {

#define MYRENDERER_BEGIN_CLASS(Name) class Name  \
                                    {

#define MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Name, ...) class Name : __VA_ARGS__ \
                                                {
#define MYRENDERER_END_CLASS };

#define MYRENDERER_END_STRUCT };

#define MYRENDERER_VALUE(x) = x

#define CHECK(Flag) if(Flag) { std::abort(); }
#define CHECK_WITH_LOG(Flag,LOG) if(Flag) \
                                {\
                                    throw std::runtime_error(LOG); \
                                }

#define THIS
#define THIS_
#define VIRTUAL      virtual
#define CONST        const
#define DEFAULT      =default
#define PURE         = 0
#define OVERIDE      override
#define FINAL        final
#define REF          &
#define METHOD(Name) MYRENDERER_CALL_TYPE Name

#define STATIC_CAST(Pointer,Type) static_cast<Type*>(Pointer)
#define DYNAMIC_CAST(Pointer,Type) dynamic_cast<Type*>(Pointer)

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

template <typename K,typename T>
using Map = std::unordered_map<K,T>;

template <typename T>
using Optional = std::optional<T>;

template <typename T>
using Set = std::unordered_set<T>;

template <typename T>
using Queue =std::queue<T>;

template <typename T>
using Stack = std::stack<T>;

#endif 
