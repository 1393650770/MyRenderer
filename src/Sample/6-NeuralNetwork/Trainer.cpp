#include "Trainer.h"
#include <iostream>
#include <chrono>
#include "RHI/RenderRHI.h"
// --   Vulkan includes removed - using RHI globals

using namespace MXRender;
using namespace MXRender::RHI;

namespace MXNN {

Trainer::Trainer(SequentialModel& model, MNISTDataLoader& train_loader,
				 MNISTDataLoader& test_loader, CommandList* cmd,
				 uint32_t epochs, uint32_t log_interval)
	: model_(model), train_loader_(train_loader), test_loader_(test_loader),
	  cmd_(cmd), epochs_(epochs), log_interval_(log_interval) {}

void Trainer::Train(ProgressCallback on_progress) {
	uint32_t total_batches_per_epoch = (train_loader_.NumImages() + model_.MaxBatchSize() - 1)
									   / model_.MaxBatchSize();

	Tensor input_buf({model_.MaxBatchSize(), train_loader_.ImageSize()});

	for (uint32_t epoch = 0; epoch < epochs_; ++epoch) {
		train_loader_.Shuffle();
		train_loader_.Reset();

		float epoch_loss = 0.0f;
		uint32_t batch_count = 0;
		auto epoch_start = std::chrono::high_resolution_clock::now();

		while (true) {
			std::vector<float> images;
			std::vector<uint8_t> labels;
			uint32_t n = train_loader_.NextBatch(images, labels, model_.MaxBatchSize());
			if (n == 0) break;

			// Upload batch to GPU
			input_buf.Upload(images.data());

			// Record training step --   using RHI lifecycle
			cmd_->Begin();

			float loss = model_.TrainStep(cmd_, input_buf, labels, n);

			cmd_->End();
			RHISubmitCommandListForQueue(cmd_, ENUM_QUEUE_TYPE::COMPUTE);

			epoch_loss += loss;
			batch_count++;

			if (on_progress && (batch_count % log_interval_ == 0 || n < model_.MaxBatchSize())) {
				float avg_loss = epoch_loss / static_cast<float>(batch_count);
				on_progress(epoch, batch_count, total_batches_per_epoch, avg_loss);
			}
		}

		auto epoch_end = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(epoch_end - epoch_start).count();
		float avg_loss = epoch_loss / static_cast<float>(batch_count);

		std::cout << "Epoch " << epoch << " | loss: " << avg_loss
				  << " | time: " << ms << "ms";

		// Evaluate on test set every epoch
		// float acc = Evaluate(); // TODO: fix inference path
		std::cout << /* " | test acc: " << (acc * 100.0f) << "%" << */ std::endl;
	}
}

float Trainer::Evaluate() {
	uint32_t correct = 0;
	uint32_t total = 0;

	test_loader_.Reset();

	while (true) {
		std::vector<float> images;
		std::vector<uint8_t> labels;
		uint32_t n = test_loader_.NextBatch(images, labels, model_.MaxBatchSize());
		if (n == 0) break;

		Tensor input_buf({n, test_loader_.ImageSize()});
		input_buf.Upload(images.data());

		// --   using RHI lifecycle
		cmd_->Begin();
		auto preds = model_.Predict(cmd_, input_buf, n);
		cmd_->End();
		RHISubmitCommandListForQueue(cmd_, ENUM_QUEUE_TYPE::COMPUTE);

		for (uint32_t i = 0; i < n; ++i) {
			if (preds[i] == labels[i]) correct++;
		}
		total += n;
	}

	return (total > 0) ? static_cast<float>(correct) / static_cast<float>(total) : 0.0f;
}

} // namespace MXNN
