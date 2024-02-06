#pragma once

#ifndef _TYPEHASH_
#define _TYPEHASH_

#include"ConstDefine.h"
#include <type_traits>

inline UInt64 METHOD(HashCombine)(UInt64 A , UInt64 C)
{
    UInt64 B = 0x9e3779b9;
    A += B;
    A -= B; A -= C; A ^= (C>>13);
    B -= C; B -= A; B ^= (A<<8);
    C -= A; C -= B; C ^= (B>>13);
    A -= B; A -= C; A ^= (C>>12);
    B -= C; B -= A; B ^= (A<<16);
    C -= A; C -= B; C ^= (B>>5);
    A -= B; A -= C; A ^= (C>>3);
    B -= C; B -= A; B ^= (A<<10);
    C -= A; C -= B; C ^= (B>>15);
    return C;
}

template <typename T>
void HashCombine(UInt64& Seed, CONST T& Val) noexcept
{
    Seed ^= std::hash<T>{}(Val)+0x9e3779b9 + (Seed << 6) + (Seed >> 2);
}

template <typename FirstArgType, typename... RestArgsType>
inline void METHOD(HashCombine)(UInt64& Seed, CONST FirstArgType& FirstArg, CONST RestArgsType&... RestArgs) noexcept
{
	HashCombine(Seed, FirstArg);
	HashCombine(Seed, RestArgs...); // recursive call using pack expansion syntax
}

template <typename FirstArgType, typename... RestArgsType>
UInt64 METHOD(ComputeHash)(CONST FirstArgType& FirstArg, CONST RestArgsType&... RestArgs) noexcept
{
    UInt64 Seed = 0;
	HashCombine(Seed, FirstArg, RestArgs...);
	return Seed;
}

#endif 
