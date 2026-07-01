#pragma once
#include "RHI/RenderBuffer.h"
#include "RHI/RenderRHI.h"

using namespace MXRender;
using namespace MXRender::RHI;

// GPU Tensor: wraps a Storage Buffer with exclusive ownership.
namespace MXNN {

// -- [AI:BEGIN] Float16 dtype
enum class ENUM_TENSOR_DTYPE : UInt8 { Float32 = 0, Float16 = 1 };
// -- [AI:END]

MYRENDERER_BEGIN_CLASS(Tensor)
#pragma region MATHOD
public:
	Tensor() MYDEFAULT;
	explicit Tensor(CONST Vector<UInt32>& in_shape, ENUM_TENSOR_DTYPE in_dtype = ENUM_TENSOR_DTYPE::Float32); // -- [AI]
	~Tensor();

	Tensor(CONST Tensor&) MYDELETE;
	Tensor& operator=(CONST Tensor&) MYDELETE;

	Tensor(Tensor&& in_other) noexcept;
	Tensor& operator=(Tensor&& in_other) noexcept;

	void Upload(CONST Float32* in_data);
	void Download(Float32* out_data) CONST;
	void Zero();
	void RandomNormal(Float32 in_mean, Float32 in_stddev);

	Buffer* GetBuffer() CONST { return buffer_; }
	CONST Vector<UInt32>& Shape() CONST { return shape_; }
	size_t ElementCount() CONST { return element_count_; }
	size_t ByteSize() CONST { return element_count_ * ElementByteSize(); } // -- [AI]
	ENUM_TENSOR_DTYPE METHOD(GetDtype)() CONST { return dtype_; }
	UInt32 METHOD(ElementByteSize)() CONST { return dtype_ == ENUM_TENSOR_DTYPE::Float16 ? 2u : 4u; }
protected:
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
	Buffer* buffer_ = nullptr;
	Vector<UInt32> shape_;
	size_t element_count_ = 0;
	ENUM_TENSOR_DTYPE dtype_ = ENUM_TENSOR_DTYPE::Float32; // -- [AI]
#pragma endregion
MYRENDERER_END_CLASS

} // namespace MXNN
