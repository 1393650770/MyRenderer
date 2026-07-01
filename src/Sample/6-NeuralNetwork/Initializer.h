#pragma once
// -- [AI:BEGIN]
#ifndef _NN_INITIALIZER_
#define _NN_INITIALIZER_
#include "Tensor.h"
#include <random>
#include <cmath>

namespace MXNN {

enum class ENUM_INIT_METHOD : UInt8 { GlorotUniform = 0, HeNormal, HeUniform };

inline void InitializeWeights(Tensor& in_w, UInt32 in_fan_in, UInt32 in_fan_out, ENUM_INIT_METHOD in_m)
{
	size_t n = in_w.ElementCount();
	Vector<Float32> data(n);
	std::mt19937 rng(42u);
	if (in_m == ENUM_INIT_METHOD::GlorotUniform)
	{
		Float32 lim = std::sqrt(6.0f / (in_fan_in + in_fan_out));
		std::uniform_real_distribution<Float32> d(-lim, lim);
		for (size_t i = 0; i < n; ++i) data[i] = d(rng);
	}
	else if (in_m == ENUM_INIT_METHOD::HeNormal)
	{
		Float32 std = std::sqrt(2.0f / in_fan_in);
		std::normal_distribution<Float32> d(0.0f, std);
		for (size_t i = 0; i < n; ++i) data[i] = d(rng);
	}
	else
	{
		Float32 lim = std::sqrt(6.0f / in_fan_in);
		std::uniform_real_distribution<Float32> d(-lim, lim);
		for (size_t i = 0; i < n; ++i) data[i] = d(rng);
	}
	in_w.Upload(data.data());
}

inline void InitializeBias(Tensor& in_b, Float32 in_v = 0.0f)
{
	size_t n = in_b.ElementCount();
	Vector<Float32> data(n, in_v);
	in_b.Upload(data.data());
}

} // namespace MXNN
// -- [AI:END]
#endif
