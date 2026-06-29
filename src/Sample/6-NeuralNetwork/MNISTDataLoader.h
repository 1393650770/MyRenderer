#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace MXNN {

// Parses MNIST IDX file format (big-endian binary)
class MNISTDataLoader {
public:
	MNISTDataLoader(const std::string& image_path, const std::string& label_path);

	// Load a batch. Returns number of samples loaded (may be < batch_size at end).
	// images_out: [batch * rows * cols] normalized floats in [0,1]
	// labels_out: [batch] uint8 class indices [0-9]
	uint32_t NextBatch(std::vector<float>& images_out, std::vector<uint8_t>& labels_out,
					   uint32_t batch_size);

	// Shuffle the dataset (randomizes the sample order)
	void Shuffle();

	// Reset to beginning of dataset
	void Reset();

	uint32_t NumImages() const { return num_images_; }
	uint32_t Rows() const { return rows_; }
	uint32_t Cols() const { return cols_; }
	uint32_t ImageSize() const { return rows_ * cols_; }

private:
	static uint32_t ReadBigEndianU32(std::ifstream& file);

	std::vector<std::vector<float>> images_;  // [num_images][rows*cols]
	std::vector<uint8_t> labels_;             // [num_images]
	std::vector<uint32_t> indices_;           // shuffled indices
	uint32_t num_images_ = 0;
	uint32_t rows_ = 0, cols_ = 0;
	uint32_t current_index_ = 0;
};

} // namespace MXNN
