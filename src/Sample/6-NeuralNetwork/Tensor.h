#pragma once
#include "RHI/RenderBuffer.h"
#include "RHI/RenderRHI.h"

using namespace MXRender;
using namespace MXRender::RHI;

// GPU Tensor: wraps a Storage Buffer with exclusive ownership.
namespace MXNN {

MYRENDERER_BEGIN_CLASS(Tensor)
#pragma region MATHOD
public:
	Tensor() MYDEFAULT;
	explicit Tensor(CONST Vector<UInt32>& in_shape);
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
	size_t ByteSize() CONST { return element_count_ * sizeof(Float32); }
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
#pragma endregion
MYRENDERER_END_CLASS

} // namespace MXNN
