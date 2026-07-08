#include "Tensor.h"
#include <cstring>
#include <random>
#include "Core/ConstDefine.h"

using namespace MXRender::RHI;
using namespace MXRender;

namespace MXNN {

Tensor::Tensor(CONST Vector<UInt32>& in_shape, ENUM_TENSOR_DTYPE in_dtype) // --  
	: shape_(in_shape)
	, dtype_(in_dtype)
{
	element_count_ = 1;
	for (auto d : shape_)
	{
		element_count_ *= d;
	}

	BufferDesc desc;
	desc.type = ENUM_BUFFER_TYPE::Storage;
	// --   FP16 support: allocate half-size buffer
	if (in_dtype == ENUM_TENSOR_DTYPE::Float16) {
		desc.stride = (UInt32)element_count_ * 2u;
		desc.size = (UInt32)element_count_ * 2u;
	} else {
		desc.stride = (UInt32)element_count_ * sizeof(Float32);
		desc.size = (UInt32)element_count_ * sizeof(Float32);
	}
	buffer_ = RHICreateBuffer(desc);
}

Tensor::~Tensor()
{
	if (buffer_)
	{
		delete buffer_;
		buffer_ = nullptr;
	}
}

Tensor::Tensor(Tensor&& in_other) noexcept
	: buffer_(in_other.buffer_)
	, shape_(std::move(in_other.shape_))
	, element_count_(in_other.element_count_)
	, dtype_(in_other.dtype_) // --  
{
	in_other.buffer_ = nullptr;
	in_other.element_count_ = 0;
}

Tensor& Tensor::operator=(Tensor&& in_other) noexcept
{
	if (this != &in_other)
	{
		if (buffer_)
		{
			delete buffer_;
		}
		buffer_ = in_other.buffer_;
		shape_ = std::move(in_other.shape_);
		element_count_ = in_other.element_count_;
		dtype_ = in_other.dtype_; // --  
		in_other.buffer_ = nullptr;
		in_other.element_count_ = 0;
	}
	return *this;
}

void Tensor::Upload(CONST Float32* in_data)
{
	void* ptr = RHIMapBuffer(buffer_, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memcpy(ptr, in_data, ByteSize());
	RHIUnmapBuffer(buffer_);
}

void Tensor::Download(Float32* out_data) CONST
{
	void* ptr = RHIMapBuffer(buffer_, ENUM_MAP_TYPE::Read, ENUM_MAP_FLAG::None);
	std::memcpy(out_data, ptr, ByteSize());
	RHIUnmapBuffer(buffer_);
}

void Tensor::Zero()
{
	void* ptr = RHIMapBuffer(buffer_, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memset(ptr, 0, ByteSize());
	RHIUnmapBuffer(buffer_);
}

void Tensor::RandomNormal(Float32 in_mean, Float32 in_stddev)
{
	Vector<Float32> data(element_count_);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution<Float32> dist(in_mean, in_stddev);
	for (size_t i = 0; i < element_count_; ++i)
	{
		data[i] = dist(gen);
	}
	Upload(data.data());
}

} // namespace MXNN
