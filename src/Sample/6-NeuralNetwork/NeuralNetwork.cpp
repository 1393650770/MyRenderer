#include <iostream>
#include <cmath>
#include <random>
#include "Application/Window.h"
#include "RHI/RenderRHI.h"
#include "RHI/Vulkan/VK_RenderRHI.h"
#include "RHI/Vulkan/VK_CommandBuffer.h"
#include "RHI/Vulkan/VK_Device.h"
#include "RHI/Vulkan/VK_Queue.h"
#include "Tensor.h"
#include "Layer.h"
#include "Optimizer.h"
#include "Model.h"

using namespace MXRender::RHI;
using namespace MXRender::Application;
using namespace MXRender;
using namespace MXNN;

class SpiralDataset { /* same as before */
public:
	SpiralDataset(uint32_t n_points, uint32_t n_classes, float noise = 0.02f)
		: n_points_(n_points), n_classes_(n_classes), input_dim_(2) {
		std::mt19937 rng(42);
		std::normal_distribution<float> noise_dist(0.0f, noise);
		std::uniform_real_distribution<float> radius_dist(0.0f, 1.0f);
		data_.resize(n_points * 2); labels_.resize(n_points);
		for (uint32_t i = 0; i < n_points; i++) {
			uint32_t c = i % n_classes;
			float r = radius_dist(rng);
			float angle = r * 4.0f * 3.14159f + c * 2.0944f;
			data_[i * 2 + 0] = r * std::cos(angle) + noise_dist(rng);
			data_[i * 2 + 1] = r * std::sin(angle) + noise_dist(rng);
			labels_[i] = static_cast<uint8_t>(c);
		}
		indices_.resize(n_points);
		for (uint32_t i = 0; i < n_points; i++) indices_[i] = i;
	}
	uint32_t NumSamples() const { return n_points_; }
	uint32_t InputDim() const { return input_dim_; }
	uint32_t NumClasses() const { return n_classes_; }
	void Shuffle() { std::random_device rd; std::mt19937 gen(rd()); std::shuffle(indices_.begin(), indices_.end(), gen); }
	uint32_t NextBatch(std::vector<float>& images, std::vector<uint8_t>& labels, uint32_t batch_size) {
		uint32_t remaining = n_points_ - current_idx_;
		uint32_t n = (std::min)(batch_size, remaining);
		images.resize(n * input_dim_); labels.resize(n);
		for (uint32_t i = 0; i < n; i++) {
			uint32_t idx = indices_[current_idx_ + i];
			images[i * 2 + 0] = data_[idx * 2 + 0]; images[i * 2 + 1] = data_[idx * 2 + 1];
			labels[i] = labels_[idx];
		}
		current_idx_ += n; return n;
	}
	void Reset() { current_idx_ = 0; }
private:
	uint32_t n_points_, n_classes_, input_dim_;
	std::vector<float> data_; std::vector<uint8_t> labels_;
	std::vector<uint32_t> indices_; uint32_t current_idx_ = 0;
};

int main() {
	std::cerr << "=== START ===" << std::endl;
	Window window;
	std::cerr << "InitWindow..." << std::endl;
	window.InitWindow();
	std::cerr << "GetDevice..." << std::endl;
	auto* vk_rhi = static_cast<Vulkan::VulkanRHI*>(g_render_rhi);
	auto* device = vk_rhi->GetDevice();
	std::cerr << "Device OK" << std::endl;

	const uint32_t max_batch_size = 32;
	std::cerr << "Creating model..." << std::endl;
	SequentialModel model(max_batch_size);
	model.AddLayer(std::make_unique<LinearLayer>(2, 64, max_batch_size, true));
	std::cerr << "Hidden layer created" << std::endl;
	model.AddLayer(std::make_unique<SoftmaxCrossEntropyOutputLayer>(64, 3, max_batch_size));
	std::cerr << "Output layer created" << std::endl;
	model.SetOptimizer(std::make_unique<SGD>(0.1f, 0.9f));
	std::cerr << "Optimizer created" << std::endl;

	SpiralDataset dataset(600, 3);
	auto* cmd = device->GetCommandBufferManager()->GetOrCreateCommandBuffer(ENUM_QUEUE_TYPE::COMPUTE, false);
	std::cerr << "Cmd buffer acquired" << std::endl;

	Tensor input_buf({max_batch_size, dataset.InputDim()});
	std::cerr << "Input buf created, starting training..." << std::endl;

	for (uint32_t epoch = 0; epoch < 2; ++epoch) {
		dataset.Shuffle(); dataset.Reset();
		uint32_t batch_count = 0;
		while (true) {
			std::vector<float> images; std::vector<uint8_t> labels;
			uint32_t n = dataset.NextBatch(images, labels, max_batch_size);
			if (n == 0) break;
			std::cerr << "[B" << batch_count << "] Begin..." << std::endl;
			auto* vk_cmd = static_cast<Vulkan::VK_CommandBuffer*>(cmd);
			vk_cmd->Begin();
			std::cerr << "[B" << batch_count << "] TrainStep..." << std::endl;
			float loss = model.TrainStep(cmd, input_buf, labels, n);
			std::cerr << "[B" << batch_count << "] End, loss=" << loss << std::endl;
			vk_cmd->End();
			std::cerr << "[B" << batch_count << "] Submit..." << std::endl;
			device->GetQueue(ENUM_QUEUE_TYPE::COMPUTE)->Submit(vk_cmd);
			std::cerr << "[B" << batch_count << "] Done" << std::endl;
			batch_count++;
		}
		std::cout << "Epoch " << epoch << " done, " << batch_count << " batches" << std::endl;
	}
	std::cerr << "=== DONE ===" << std::endl;
	system("pause");
	return 0;
}
