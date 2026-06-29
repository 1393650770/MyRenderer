#pragma once
#include <vector>
#include <random>
#include "RHI/RenderEnum.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderBuffer.h"

using namespace MXRender;
using namespace MXRender::RHI;

// GPU Tensor: wraps a Storage Buffer with exclusive ownership.
namespace MXNN {

class Tensor {
public:
	Tensor() = default;
	explicit Tensor(const std::vector<uint32_t>& shape);
	~Tensor();
	Tensor(const Tensor&) = delete;
	Tensor& operator=(const Tensor&) = delete;
	Tensor(Tensor&& other) noexcept;
	Tensor& operator=(Tensor&& other) noexcept;

	void Upload(const float* data);
	void Download(float* out_data) const;
	void Zero();
	void RandomNormal(float mean, float stddev);

	Buffer* GetBuffer() const { return buffer_; }
	const std::vector<uint32_t>& Shape() const { return shape_; }
	size_t ElementCount() const { return element_count_; }
	size_t ByteSize() const { return element_count_ * sizeof(float); }

private:
	Buffer* buffer_ = nullptr;
	std::vector<uint32_t> shape_;
	size_t element_count_ = 0;
};

} // namespace MXNN
