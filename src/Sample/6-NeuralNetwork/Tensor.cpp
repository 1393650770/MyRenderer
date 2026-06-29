#include "Tensor.h"
#include <cstring>
#include <random>

using namespace MXRender::RHI;

// MXNN::Tensor implementations (Tensor.h declares inside namespace MXNN)
namespace MXNN {

Tensor::Tensor(const std::vector<uint32_t>& shape) : shape_(shape) {
	element_count_ = 1;
	for (auto d : shape_) element_count_ *= d;

	BufferDesc desc;
	desc.type = ENUM_BUFFER_TYPE::Storage;
	desc.stride = 4;
	desc.size = static_cast<UInt32>(element_count_ * sizeof(float));
	buffer_ = RHICreateBuffer(desc);
}

Tensor::~Tensor() { if (buffer_) { delete buffer_; buffer_ = nullptr; } }

Tensor::Tensor(Tensor&& other) noexcept
	: buffer_(other.buffer_), shape_(std::move(other.shape_)), element_count_(other.element_count_) {
	other.buffer_ = nullptr; other.element_count_ = 0;
}

Tensor& Tensor::operator=(Tensor&& other) noexcept {
	if (this != &other) {
		if (buffer_) delete buffer_;
		buffer_ = other.buffer_; shape_ = std::move(other.shape_);
		element_count_ = other.element_count_; other.buffer_ = nullptr; other.element_count_ = 0;
	}
	return *this;
}

void Tensor::Upload(const float* data) {
	void* ptr = RHIMapBuffer(buffer_, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memcpy(ptr, data, ByteSize());
	RHIUnmapBuffer(buffer_);
}

void Tensor::Download(float* out_data) const {
	void* ptr = RHIMapBuffer(buffer_, ENUM_MAP_TYPE::Read, ENUM_MAP_FLAG::None);
	std::memcpy(out_data, ptr, ByteSize());
	RHIUnmapBuffer(buffer_);
}

void Tensor::Zero() {
	void* ptr = RHIMapBuffer(buffer_, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memset(ptr, 0, ByteSize());
	RHIUnmapBuffer(buffer_);
}

void Tensor::RandomNormal(float mean, float stddev) {
	std::vector<float> data(element_count_);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution<float> dist(mean, stddev);
	for (size_t i = 0; i < element_count_; ++i) data[i] = dist(gen);
	Upload(data.data());
}

} // namespace MXNN
