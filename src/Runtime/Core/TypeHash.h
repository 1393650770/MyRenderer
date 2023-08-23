#pragma once

#ifndef _TYPEHASH_
#define _TYPEHASH_

#include"ConstDefine.h"

inline UInt32 METHOD(HashCombine)(UInt32 A ,UInt32 C)
{
    UInt32 B = 0x9e3779b9;
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
#endif 
