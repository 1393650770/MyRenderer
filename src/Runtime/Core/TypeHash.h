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

#include <stdlib.h>
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)


struct Uint128_64
{
	inline Uint128_64(UInt64 InLo, UInt64 InHi) : lo(InLo), hi(InHi) {}
    UInt64 lo;
    UInt64 hi;
};


static UInt64 UNALIGNED_LOAD64(CONST Char* p) {
	UInt64 result;
	memcpy(&result, p, sizeof(result));
	return result;
}

static UInt32 UNALIGNED_LOAD32(CONST Char* p) {
	UInt32 result;
	memcpy(&result, p, sizeof(result));
	return result;
}

static UInt64 Fetch64(CONST Char* p) {
	return (UNALIGNED_LOAD64(p));
}

static UInt32 Fetch32(CONST Char* p) {
	return (UNALIGNED_LOAD32(p));
}

namespace CityHash_Internal
{
	// Some primes between 2^63 and 2^64 for various uses.
	static CONST UInt64 k0 = 0xc3a5c85c97cb3127ULL;
	static CONST UInt64 k1 = 0xb492b66fbe98f273ULL;
	static CONST UInt64 k2 = 0x9ae16a3b2f90404fULL;

	// Magic numbers for 32-bit hashing.  Copied from Murmur3.
	static CONST UInt32 c1 = 0xcc9e2d51;
	static CONST UInt32 c2 = 0x1b873593;
}

static UInt32 fmix(UInt32 h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

static UInt32 Rotate32(UInt32 val, int shift) {
	// Avoid shifting by 32: doing so yields an undefined result.
	return shift == 0 ? val : ((val >> shift) | (val << (32 - shift)));
}

template<typename T>
void SwapValues(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}

#undef PERMUTE3
#define PERMUTE3(a, b, c) do { SwapValues(a, b); SwapValues(a, c); } while (0)

static UInt32 Mur(UInt32 a, UInt32 h) {
	using namespace CityHash_Internal;

	// Helper from Murmur3 for combining two 32-bit values.
	a *= c1;
	a = Rotate32(a, 17);
	a *= c2;
	h ^= a;
	h = Rotate32(h, 19);
	return h * 5 + 0xe6546b64;
}

// Hash 128 input bits down to 64 bits of output.
// This is intended to be a reasonably good hash function.
inline UInt64 METHOD(CityHash128to64)(CONST Uint128_64& x) {
	// Murmur-inspired hashing.
	CONST UInt64 kMul = 0x9ddfea08eb382d69ULL;
	UInt64 a = (x.lo ^ x.hi) * kMul;
	a ^= (a >> 47);
	UInt64 b = (x.hi ^ a) * kMul;
	b ^= (b >> 47);
	b *= kMul;
	return b;
}
static UInt32 Hash32Len13to24(CONST Char* s, UInt32 len) {
	UInt32 a = Fetch32(s - 4 + (len >> 1));
	UInt32 b = Fetch32(s + 4);
	UInt32 c = Fetch32(s + len - 8);
	UInt32 d = Fetch32(s + (len >> 1));
	UInt32 e = Fetch32(s);
	UInt32 f = Fetch32(s + len - 4);
	UInt32 h = len;

	return fmix(Mur(f, Mur(e, Mur(d, Mur(c, Mur(b, Mur(a, h)))))));
}

static UInt32 Hash32Len0to4(CONST Char* s, UInt32 len) {
	using namespace CityHash_Internal;

	UInt32 b = 0;
	UInt32 c = 9;
	for (UInt32 i = 0; i < len; i++) 
	{
		signed char v = s[i];
		b = b * c1 + v;
		c ^= b;
	}
	return fmix(Mur(b, Mur(len, c)));
}

static UInt32 Hash32Len5to12(CONST Char* s, UInt32 len) {
	UInt32 a = len, b = len * 5, c = 9, d = b;
	a += Fetch32(s);
	b += Fetch32(s + len - 4);
	c += Fetch32(s + ((len >> 1) & 4));
	return fmix(Mur(c, Mur(b, Mur(a, d))));
}


static UInt64 Rotate(UInt64 val, Int shift) {
	// Avoid shifting by 64: doing so yields an undefined result.
	return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

static UInt64 ShiftMix(UInt64 val) {
	return val ^ (val >> 47);
}

static UInt64 HashLen16(UInt64 u, UInt64 v) {
	return CityHash128to64({ u, v });
}

static UInt64 HashLen16(UInt64 u, UInt64 v, UInt64 mul) {
	// Murmur-inspired hashing.
	UInt64 a = (u ^ v) * mul;
	a ^= (a >> 47);
	UInt64 b = (v ^ a) * mul;
	b ^= (b >> 47);
	b *= mul;
	return b;
}

static UInt64 HashLen0to16(CONST Char* s, UInt32 len) {
	using namespace CityHash_Internal;

	if (len >= 8) {
		UInt64 mul = k2 + len * 2;
		UInt64 a = Fetch64(s) + k2;
		UInt64 b = Fetch64(s + len - 8);
		UInt64 c = Rotate(b, 37) * mul + a;
		UInt64 d = (Rotate(a, 25) + b) * mul;
		return HashLen16(c, d, mul);
	}
	if (len >= 4) {
		UInt64 mul = k2 + len * 2;
		UInt64 a = Fetch32(s);
		return HashLen16(len + (a << 3), Fetch32(s + len - 4), mul);
	}
	if (len > 0) {
		UInt8 a = s[0];
		UInt8 b = s[len >> 1];
		UInt8 c = s[len - 1];
		UInt32 y = static_cast<UInt32>(a) + (static_cast<UInt32>(b) << 8);
		UInt32 z = len + (static_cast<UInt32>(c) << 2);
		return ShiftMix(y * k2 ^ z * k0) * k2;
	}
	return k2;
}

// This probably works well for 16-byte strings as well, but it may be overkill
// in that case.
static UInt64 HashLen17to32(CONST Char* s, UInt32 len) {
	using namespace CityHash_Internal;

	UInt64 mul = k2 + len * 2;
	UInt64 a = Fetch64(s) * k1;
	UInt64 b = Fetch64(s + 8);
	UInt64 c = Fetch64(s + len - 8) * mul;
	UInt64 d = Fetch64(s + len - 16) * k2;
	return HashLen16(Rotate(a + b, 43) + Rotate(c, 30) + d,
		a + Rotate(b + k2, 18) + c, mul);
}

// Return a 16-byte hash for 48 bytes.  Quick and dirty.
// Callers do best to use "random-looking" values for a and b.
static Uint128_64 WeakHashLen32WithSeeds(
	UInt64 w, UInt64 x, UInt64 y, UInt64 z, UInt64 a, UInt64 b) {
	a += w;
	b = Rotate(b + a + z, 21);
	UInt64 c = a;
	a += x;
	a += y;
	b += Rotate(a, 44);
	return { (a + z), (b + c) };
}

// Return a 16-byte hash for s[0] ... s[31], a, and b.  Quick and dirty.
static Uint128_64 WeakHashLen32WithSeeds(
	CONST Char* s, UInt64 a, UInt64 b) {
	return WeakHashLen32WithSeeds(Fetch64(s),
		Fetch64(s + 8),
		Fetch64(s + 16),
		Fetch64(s + 24),
		a,
		b);
}

// Return an 8-byte hash for 33 to 64 bytes.
static UInt64 HashLen33to64(CONST Char* s, UInt32 len) {
	using namespace CityHash_Internal;

	UInt64 mul = k2 + len * 2;
	UInt64 a = Fetch64(s) * k2;
	UInt64 b = Fetch64(s + 8);
	UInt64 c = Fetch64(s + len - 24);
	UInt64 d = Fetch64(s + len - 32);
	UInt64 e = Fetch64(s + 16) * k2;
	UInt64 f = Fetch64(s + 24) * 9;
	UInt64 g = Fetch64(s + len - 8);
	UInt64 h = Fetch64(s + len - 16) * mul;
	UInt64 u = Rotate(a + g, 43) + (Rotate(b, 30) + c) * 9;
	UInt64 v = ((a + g) ^ d) + f + 1;
	UInt64 w = bswap_64((u + v) * mul) + h;
	UInt64 x = Rotate(e + f, 42) + c;
	UInt64 y = (bswap_64((v + w) * mul) + g) * mul;
	UInt64 z = e + f + c;
	a = bswap_64((x + z) * mul + y) + b;
	b = ShiftMix((z + a) * mul + d + h) * mul;
	return b + x;
}


// Hash function for a byte array.
inline UInt64 METHOD(CityHash64)(CONST Char* s, UInt32 len)
{
	using namespace CityHash_Internal;

	if (len <= 32) {
		if (len <= 16) {
			return HashLen0to16(s, len);
		}
		else {
			return HashLen17to32(s, len);
		}
	}
	else if (len <= 64) {
		return HashLen33to64(s, len);
	}

	// For strings over 64 bytes we hash the end first, and then as we
	// loop we keep 56 bytes of state: v, w, x, y, and z.
	UInt64 x = Fetch64(s + len - 40);
	UInt64 y = Fetch64(s + len - 16) + Fetch64(s + len - 56);
	UInt64 z = HashLen16(Fetch64(s + len - 48) + len, Fetch64(s + len - 24));
	Uint128_64 v = WeakHashLen32WithSeeds(s + len - 64, len, z);
	Uint128_64 w = WeakHashLen32WithSeeds(s + len - 32, y + k1, x);
	x = x * k1 + Fetch64(s);

	// Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
	len = (len - 1) & ~static_cast<UInt32>(63);
	do {
		x = Rotate(x + y + v.lo + Fetch64(s + 8), 37) * k1;
		y = Rotate(y + v.hi + Fetch64(s + 48), 42) * k1;
		x ^= w.hi;
		y += v.lo + Fetch64(s + 40);
		z = Rotate(z + w.lo, 33) * k1;
		v = WeakHashLen32WithSeeds(s, v.hi * k1, x + w.lo);
		w = WeakHashLen32WithSeeds(s + 32, z + w.hi, y + Fetch64(s + 16));
		SwapValues(z, x);
		s += 64;
		len -= 64;
	} while (len != 0);
	return HashLen16(HashLen16(v.lo, w.lo) + ShiftMix(y) * k1 + z,
		HashLen16(v.hi, w.hi) + x);
}
// Hash function for a byte array.  For convenience, two seeds are also
// hashed into the result.
inline  UInt64 METHOD(CityHash64WithSeeds)(CONST Char* s, UInt32 len,
	UInt64 seed0, UInt64 seed1)
{
	using namespace CityHash_Internal;

	return HashLen16(CityHash64(s, len) - seed0, seed1);
}

// Hash function for a byte array.  For convenience, a 64-bit seed is also
// hashed into the result.
inline  UInt64 METHOD(CityHash64WithSeed)(CONST Char* s, UInt32 len, UInt64 seed)
{
	using namespace CityHash_Internal;

	return CityHash64WithSeeds(s, len, k2, seed);
}



// Hash function for a byte array.  Most useful in 32-bit binaries.
inline  UInt32 METHOD(CityHash32)(CONST Char* s, UInt32 len)
{
	using namespace CityHash_Internal;

	if (len <= 24) {
		return len <= 12 ?
			(len <= 4 ? Hash32Len0to4(s, len) : Hash32Len5to12(s, len)) :
			Hash32Len13to24(s, len);
	}

	// len > 24
	UInt32 h = len, g = c1 * len, f = g;
	UInt32 a0 = Rotate32(Fetch32(s + len - 4) * c1, 17) * c2;
	UInt32 a1 = Rotate32(Fetch32(s + len - 8) * c1, 17) * c2;
	UInt32 a2 = Rotate32(Fetch32(s + len - 16) * c1, 17) * c2;
	UInt32 a3 = Rotate32(Fetch32(s + len - 12) * c1, 17) * c2;
	UInt32 a4 = Rotate32(Fetch32(s + len - 20) * c1, 17) * c2;
	h ^= a0;
	h = Rotate32(h, 19);
	h = h * 5 + 0xe6546b64;
	h ^= a2;
	h = Rotate32(h, 19);
	h = h * 5 + 0xe6546b64;
	g ^= a1;
	g = Rotate32(g, 19);
	g = g * 5 + 0xe6546b64;
	g ^= a3;
	g = Rotate32(g, 19);
	g = g * 5 + 0xe6546b64;
	f += a4;
	f = Rotate32(f, 19);
	f = f * 5 + 0xe6546b64;
	UInt32 iters = (len - 1) / 20;
	do {
		UInt32 _a0 = Rotate32(Fetch32(s) * c1, 17) * c2;
		UInt32 _a1 = Fetch32(s + 4);
		UInt32 _a2 = Rotate32(Fetch32(s + 8) * c1, 17) * c2;
		UInt32 _a3 = Rotate32(Fetch32(s + 12) * c1, 17) * c2;
		UInt32 _a4 = Fetch32(s + 16);
		h ^= _a0;
		h = Rotate32(h, 18);
		h = h * 5 + 0xe6546b64;
		f += _a1;
		f = Rotate32(f, 19);
		f = f * c1;
		g += _a2;
		g = Rotate32(g, 18);
		g = g * 5 + 0xe6546b64;
		h ^= _a3 + _a1;
		h = Rotate32(h, 19);
		h = h * 5 + 0xe6546b64;
		g ^= _a4;
		g = bswap_32(g) * 5;
		h += _a4 * 5;
		h = bswap_32(h);
		f += _a0;
		PERMUTE3(f, h, g);
		s += 20;
	} while (--iters != 0);
	g = Rotate32(g, 11) * c1;
	g = Rotate32(g, 17) * c1;
	f = Rotate32(f, 11) * c1;
	f = Rotate32(f, 17) * c1;
	h = Rotate32(h + g, 19);
	h = h * 5 + 0xe6546b64;
	h = Rotate32(h, 17) * c1;
	h = Rotate32(h + f, 19);
	h = h * 5 + 0xe6546b64;
	h = Rotate32(h, 17) * c1;
	return h;
}


#endif 
