#include "MNISTDataLoader.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <cstring>

namespace MXNN {

uint32_t MNISTDataLoader::ReadBigEndianU32(std::ifstream& file) {
	unsigned char buf[4];
	file.read((char*)buf, 4);
	return (static_cast<uint32_t>(buf[0]) << 24) |
		   (static_cast<uint32_t>(buf[1]) << 16) |
		   (static_cast<uint32_t>(buf[2]) << 8)  |
		   (static_cast<uint32_t>(buf[3]));
}

MNISTDataLoader::MNISTDataLoader(const std::string& image_path, const std::string& label_path) {
	// --- Load images ---
	std::ifstream img_file(image_path, std::ios::binary);
	if (!img_file.is_open()) {
		std::cerr << "ERROR: Cannot open " << image_path << std::endl;
		return;
	}

	uint32_t magic = ReadBigEndianU32(img_file);
	if (magic != 0x00000803) {
		std::cerr << "ERROR: Bad image file magic: 0x" << std::hex << magic << std::endl;
		return;
	}

	num_images_ = ReadBigEndianU32(img_file);
	rows_ = ReadBigEndianU32(img_file);
	cols_ = ReadBigEndianU32(img_file);

	std::cout << "MNIST images: " << num_images_ << " x " << rows_ << "x" << cols_ << std::endl;

	uint32_t img_size = rows_ * cols_;
	images_.resize(num_images_);
	std::vector<unsigned char> raw(img_size);
	for (uint32_t i = 0; i < num_images_; ++i) {
		img_file.read((char*)raw.data(), img_size);
		images_[i].resize(img_size);
		for (uint32_t j = 0; j < img_size; ++j) {
			images_[i][j] = static_cast<float>(raw[j]) / 255.0f;
		}
	}

	// --- Load labels ---
	std::ifstream lbl_file(label_path, std::ios::binary);
	if (!lbl_file.is_open()) {
		std::cerr << "ERROR: Cannot open " << label_path << std::endl;
		return;
	}

	magic = ReadBigEndianU32(lbl_file);
	if (magic != 0x00000801) {
		std::cerr << "ERROR: Bad label file magic: 0x" << std::hex << magic << std::endl;
		return;
	}

	uint32_t num_labels = ReadBigEndianU32(lbl_file);
	if (num_labels != num_images_) {
		std::cerr << "WARNING: Label count " << num_labels << " != image count " << num_images_ << std::endl;
	}

	labels_.resize(num_labels);
	lbl_file.read((char*)labels_.data(), num_labels);

	std::cout << "MNIST labels: " << num_labels << std::endl;

	// Initialize sequential indices
	indices_.resize(num_images_);
	for (uint32_t i = 0; i < num_images_; ++i) indices_[i] = i;
}

uint32_t MNISTDataLoader::NextBatch(std::vector<float>& images_out,
									std::vector<uint8_t>& labels_out,
									uint32_t batch_size) {
	uint32_t img_size = rows_ * cols_;
	uint32_t remaining = num_images_ - current_index_;
	uint32_t n = std::min(batch_size, remaining);

	images_out.resize(n * img_size);
	labels_out.resize(n);

	for (uint32_t i = 0; i < n; ++i) {
		uint32_t idx = indices_[current_index_ + i];
		std::memcpy(images_out.data() + i * img_size, images_[idx].data(), img_size * sizeof(float));
		labels_out[i] = labels_[idx];
	}

	current_index_ += n;
	return n;
}

void MNISTDataLoader::Shuffle() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(indices_.begin(), indices_.end(), gen);
}

void MNISTDataLoader::Reset() {
	current_index_ = 0;
}

} // namespace MXNN
