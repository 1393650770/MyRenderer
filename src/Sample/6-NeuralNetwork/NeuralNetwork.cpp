#include <iostream>
#include <cmath>
#include <random>
#include "Application/Window.h"
#include "Core/ConstDefine.h"
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

// ============================================================
// SpiralDataset — synthetic 2D spiral for classification
// ============================================================
class SpiralDataset
{
public:
	SpiralDataset(UInt32 in_num_points, UInt32 in_num_classes, Float32 in_noise = 0.02f)
		: num_points_(in_num_points)
		, num_classes_(in_num_classes)
		, input_dim_(2)
	{
		std::mt19937 rng(42);
		std::normal_distribution<Float32> noise_dist(0.0f, in_noise);
		std::uniform_real_distribution<Float32> radius_dist(0.0f, 1.0f);

		data_.resize(num_points_ * 2);
		labels_.resize(num_points_);

		for (UInt32 i = 0; i < num_points_; i++)
		{
			UInt32 c = i % num_classes_;
			Float32 r = radius_dist(rng);
			Float32 angle = r * 4.0f * 3.14159f + c * 2.0944f;
			data_[i * 2 + 0] = r * std::cos(angle) + noise_dist(rng);
			data_[i * 2 + 1] = r * std::sin(angle) + noise_dist(rng);
			labels_[i] = static_cast<UInt8>(c);
		}

		indices_.resize(num_points_);
		for (UInt32 i = 0; i < num_points_; i++)
		{
			indices_[i] = i;
		}
	}

	UInt32 NumSamples() CONST { return num_points_; }
	UInt32 InputDim() CONST { return input_dim_; }
	UInt32 NumClasses() CONST { return num_classes_; }

	void Shuffle()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::shuffle(indices_.begin(), indices_.end(), gen);
	}

	UInt32 NextBatch(Vector<Float32>& out_images, Vector<UInt8>& out_labels, UInt32 in_batch_size)
	{
		UInt32 remaining = num_points_ - current_idx_;
		UInt32 n = (std::min)(in_batch_size, remaining);

		out_images.resize(n * input_dim_);
		out_labels.resize(n);

		for (UInt32 i = 0; i < n; i++)
		{
			UInt32 idx = indices_[current_idx_ + i];
			out_images[i * 2 + 0] = data_[idx * 2 + 0];
			out_images[i * 2 + 1] = data_[idx * 2 + 1];
			out_labels[i] = labels_[idx];
		}

		current_idx_ += n;
		return n;
	}

	void Reset() { current_idx_ = 0; }

private:
	UInt32 num_points_, num_classes_, input_dim_;
	Vector<Float32> data_;
	Vector<UInt8> labels_;
	Vector<UInt32> indices_;
	UInt32 current_idx_ = 0;
};

// ============================================================
// main
// ============================================================
int main()
{
	std::cout << "=== NeuralNetwork Spiral Classification ===" << std::endl;

	// Init window + Vulkan device
	Window window;
	window.InitWindow();

	auto* vk_rhi = STATIC_CAST(g_render_rhi, Vulkan::VulkanRHI);
	auto* device = vk_rhi->GetDevice();

	// Build model: 2 → 64(ReLU) → 3(Softmax)
	CONST UInt32 kMaxBatchSize = 32;
	SequentialModel model(kMaxBatchSize);
	model.AddLayer(std::make_unique<LinearLayer>(2, 64, kMaxBatchSize, true));
	model.AddLayer(std::make_unique<SoftmaxCrossEntropyOutputLayer>(64, 3, kMaxBatchSize));
	model.SetOptimizer(std::make_unique<SGD>(0.1f, 0.9f));

	// Create dataset
	SpiralDataset dataset(600, 3);

	// Get compute command buffer
	auto* cmd = device->GetCommandBufferManager()->GetOrCreateCommandBuffer(
		ENUM_QUEUE_TYPE::COMPUTE, false);

	// Input buffer
	Tensor input_buf({kMaxBatchSize, dataset.InputDim()});

	// Training loop
	for (UInt32 epoch = 0; epoch < 2; ++epoch)
	{
		dataset.Shuffle();
		dataset.Reset();

		UInt32 batch_count = 0;
		while (true)
		{
			Vector<Float32> images;
			Vector<UInt8> labels;
			UInt32 n = dataset.NextBatch(images, labels, kMaxBatchSize);
			if (n == 0)
			{
				break;
			}

			input_buf.Upload(images.data());

			auto* vk_cmd = STATIC_CAST(cmd, Vulkan::VK_CommandBuffer);
			vk_cmd->Begin();

			Float32 loss = model.TrainStep(cmd, input_buf, labels, n);

			device->GetQueue(ENUM_QUEUE_TYPE::COMPUTE)->Submit(vk_cmd);
			vk_cmd->command_state = MXRender::RHI::Vulkan::VK_CommandBuffer::EState::NeedReset;
			std::cout << "[Epoch " << epoch << " Batch " << batch_count
				<< "] loss=" << loss << std::endl;

			batch_count++;
		}

		std::cout << "Epoch " << epoch << " done, " << batch_count << " batches" << std::endl;
	}

	std::cout << "=== DONE ===" << std::endl;
	system("pause");
	return 0;
}
