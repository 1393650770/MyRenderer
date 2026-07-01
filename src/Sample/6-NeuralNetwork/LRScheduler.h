#pragma once
#ifndef _NN_LRSCHEDULER_
#define _NN_LRSCHEDULER_
#include "Core/ConstDefine.h"
#include <cmath>

namespace MXNN {

MYRENDERER_BEGIN_CLASS(LRScheduler)
#pragma region MATHOD
public:
	LRScheduler() MYDEFAULT;
	VIRTUAL ~LRScheduler() MYDEFAULT;
	VIRTUAL Float32 METHOD(GetLR)(UInt32 in_step) PURE;
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(StepLR, public LRScheduler)
#pragma region MATHOD
public:
	StepLR(Float32 in_lr, Float32 in_gamma, UInt32 in_step_size)
		: lr_(in_lr), gamma_(in_gamma), step_size_(in_step_size) {}
	VIRTUAL Float32 METHOD(GetLR)(UInt32 in_step) OVERRIDE FINAL
	{
		return lr_ * std::pow(gamma_, static_cast<Float32>(in_step / step_size_));
	}
protected:
	Float32 lr_, gamma_;
	UInt32 step_size_;
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(CosineAnnealingLR, public LRScheduler)
#pragma region MATHOD
public:
	CosineAnnealingLR(Float32 in_lr_max, Float32 in_lr_min, UInt32 in_T)
		: lr_max_(in_lr_max), lr_min_(in_lr_min), T_(in_T) {}
	VIRTUAL Float32 METHOD(GetLR)(UInt32 in_step) OVERRIDE FINAL
	{
		Float32 t = static_cast<Float32>(in_step % T_);
		return lr_min_ + 0.5f * (lr_max_ - lr_min_) * (1.0f + std::cos(3.1415926535f * t / static_cast<Float32>(T_)));
	}
protected:
	Float32 lr_max_, lr_min_;
	UInt32 T_;
#pragma endregion
MYRENDERER_END_CLASS

} // namespace MXNN
#endif
