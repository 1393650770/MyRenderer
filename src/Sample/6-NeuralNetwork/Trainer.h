#pragma once
#include <functional>
#include <cstdint>
#include "Model.h"
#include "MNISTDataLoader.h"
#include "RHI/RenderCommandList.h"

namespace MXNN {

using namespace MXRender::RHI;

using ProgressCallback = std::function<void(uint32_t epoch, uint32_t batch,
											uint32_t total_batches, float loss)>;

class Trainer {
public:
	Trainer(SequentialModel& model, MNISTDataLoader& train_loader,
			MNISTDataLoader& test_loader, CommandList* cmd,
			uint32_t epochs = 10, uint32_t log_interval = 50);

	void Train(ProgressCallback on_progress = nullptr);
	float Evaluate(); // returns test accuracy [0,1]

private:
	SequentialModel& model_;
	MNISTDataLoader& train_loader_;
	MNISTDataLoader& test_loader_;
	CommandList* cmd_;
	uint32_t epochs_;
	uint32_t log_interval_;
};

} // namespace MXNN
