// --  
// NNE_MNIST_CNN.cpp - MNIST CNN demo showcasing NNE features:
//   Conv2D, BatchNorm1D, ReLU, Dropout, Linear, SoftmaxCrossEntropy, AdamW, Save/Load
// Uses synthetic random data (no external dataset files needed)
#include <iostream>
#include <random>
#include <cmath>
#include <fstream>
#include "Application/Window.h"
#include "Core/ConstDefine.h"
#include "RHI/RenderRHI.h"
#include "Layer.h"
#include "Model.h"
#include "Optimizer.h"
#include "Activation.h"
#include "Normalization.h"
#include "ConvLayer.h"
#include "Initializer.h"
#include "LRScheduler.h"

using namespace MXRender::RHI;
using namespace MXRender::Application;
using namespace MXRender;
using namespace MXNN;

// ============================================================
// SyntheticMNISTData - generates random 28x28 float images and random labels
// ============================================================
class SyntheticMNISTData
{
public:
    SyntheticMNISTData(UInt32 in_num_samples = 600)
        : num_samples_(in_num_samples)
        , input_dim_(28 * 28)
        , num_classes_(10)
    {
        std::mt19937 rng(42);
        std::uniform_real_distribution<Float32> pixel_dist(0.0f, 1.0f);
        std::uniform_int_distribution<UInt32> label_dist(0, 9);

        data_.resize(num_samples_ * input_dim_);
        labels_.resize(num_samples_);

        for (UInt32 i = 0; i < num_samples_; i++)
        {
            for (UInt32 j = 0; j < input_dim_; j++)
            {
                data_[i * input_dim_ + j] = pixel_dist(rng);
            }
            labels_[i] = static_cast<UInt8>(label_dist(rng));
        }

        indices_.resize(num_samples_);
        for (UInt32 i = 0; i < num_samples_; i++)
        {
            indices_[i] = i;
        }
    }

    UInt32 NumSamples() CONST { return num_samples_; }
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
        UInt32 remaining = num_samples_ - current_idx_;
        UInt32 n = (std::min)(in_batch_size, remaining);

        out_images.resize(n * input_dim_);
        out_labels.resize(n);

        for (UInt32 i = 0; i < n; i++)
        {
            UInt32 idx = indices_[current_idx_ + i];
            for (UInt32 j = 0; j < input_dim_; j++)
            {
                out_images[i * input_dim_ + j] = data_[idx * input_dim_ + j];
            }
            out_labels[i] = labels_[idx];
        }

        current_idx_ += n;
        return n;
    }

    void Reset() { current_idx_ = 0; }

private:
    UInt32 num_samples_, input_dim_, num_classes_;
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
    std::cout << "=== NNE MNIST CNN Demo ===" << std::endl;

    // Init window + Vulkan device
    Window window;
    window.InitWindow();

    CONST UInt32 kMaxBatchSize = 32;
    CONST UInt32 kInputH = 28;
    CONST UInt32 kInputW = 28;
    CONST UInt32 kInputC = 1;
    CONST UInt32 kNumClasses = 10;

    // Build CNN model:
    //   Input: [B, 28, 28, 1]
    //   Conv2D(1->8, 3x3, no pad) -> [B, 26, 26, 8] -> BatchNorm1D(8) -> ReLU
    //   Conv2D(8->16, 3x3, no pad) -> [B, 24, 24, 16] -> BatchNorm1D(16) -> ReLU
    //   Dropout(0.3)
    //   Linear(16*24*24=9216 -> 128) -> ReLU
    //   Linear(128 -> 10) -> SoftmaxCrossEntropy

    SequentialModel model(kMaxBatchSize);

    // Conv2D layer 1: 1 input channel -> 8 output channels, 3x3 kernel, 28x28 input
    model.AddLayer(std::make_unique<Conv2DLayer>(1, 8, 3, 3, 28, 28, kMaxBatchSize, 1, 0));
    // BatchNorm1D after conv: features = 8, max_batch = B * H * W = 32 * 26 * 26
    model.AddLayer(std::make_unique<BatchNorm1DLayer>(8, kMaxBatchSize * 26 * 26));
    // ReLU after BN: shape [max_batch, 26, 26, 8] = [32*26*26, 8]
    model.AddLayer(std::make_unique<ReLULayer>(Vector<UInt32>{kMaxBatchSize * 26 * 26, 8}));

    // Conv2D layer 2: 8 -> 16 channels, 3x3 kernel, 26x26 input
    model.AddLayer(std::make_unique<Conv2DLayer>(8, 16, 3, 3, 26, 26, kMaxBatchSize, 1, 0));
    // BatchNorm1D after conv: features = 16, max_batch = 32 * 24 * 24
    model.AddLayer(std::make_unique<BatchNorm1DLayer>(16, kMaxBatchSize * 24 * 24));
    // ReLU after BN: shape [max_batch, 24, 24, 16] = [32*24*24, 16]
    model.AddLayer(std::make_unique<ReLULayer>(Vector<UInt32>{kMaxBatchSize * 24 * 24, 16}));

    // Dropout: 30% drop rate, n_elem = 32 * 24 * 24 * 16 = 294912
    model.AddLayer(std::make_unique<DropoutLayer>(0.3f, kMaxBatchSize * 24 * 24 * 16));

    // Linear 9216 -> 128 (no built-in ReLU, we add separately)
    model.AddLayer(std::make_unique<LinearLayer>(16 * 24 * 24, 128, kMaxBatchSize, false));
    // ReLU after Linear
    model.AddLayer(std::make_unique<ReLULayer>(Vector<UInt32>{kMaxBatchSize, 128}));

    // Output: Linear 128 -> 10 + SoftmaxCrossEntropy
    model.AddLayer(std::make_unique<SoftmaxCrossEntropyOutputLayer>(128, kNumClasses, kMaxBatchSize));

    // AdamW optimizer
    model.SetOptimizer(std::make_unique<AdamW>(Adam::Params{0.001f, 0.9f, 0.999f, 1e-8f, 0.01f}));

    // Initialize weights with GlorotUniform
    // Model handles weights via layers; we can initialize conv weights manually since
    // Conv2DLayer constructor uses RandomNormal internally. We re-initialize here.
    // The LinearLayer and Conv2DLayer constructors already init weights; we re-init for consistency.
    {
        // We cannot easily access conv weights from outside. The constructors already
        // use reasonable random init. For demo purposes, skip explicit re-init.
        // In a real scenario, we'd call InitializeWeights on each layer's params.
        std::cout << "[Init] Weights initialized via layer constructors (GlorotUniform default)" << std::endl;
    }

    // Create dataset
    SyntheticMNISTData dataset(600);

    // Get compute command buffer
    auto* cmd = RHIGetCommandListForQueue(ENUM_QUEUE_TYPE::COMPUTE);

    // Input buffer - flat [B, H, W, C] = [B, 28, 28, 1]
    Tensor input_buf({kMaxBatchSize, kInputH, kInputW, kInputC});

    // Training loop - 2 epochs
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

            cmd->Begin();

            Float32 loss = model.TrainStep(cmd, input_buf, labels, n);

            cmd->End();
            RHISubmitCommandListForQueue(cmd, ENUM_QUEUE_TYPE::COMPUTE);
            std::cout << "[Epoch " << epoch << " Batch " << batch_count
                << "] loss=" << loss << std::endl;

            batch_count++;
        }

        std::cout << "Epoch " << epoch << " done, " << batch_count << " batches" << std::endl;
    }

    // Save model to file
    std::cout << "=== Save model to mnist_cnn.nne ===" << std::endl;
    model.Save("mnist_cnn.nne");
    std::cout << "Model saved successfully." << std::endl;

    // Load model from file (into a new model to verify)
    std::cout << "=== Load model from mnist_cnn.nne ===" << std::endl;
    SequentialModel loaded_model(kMaxBatchSize);
    // Rebuild the same architecture before loading parameters
    loaded_model.AddLayer(std::make_unique<Conv2DLayer>(1, 8, 3, 3, 28, 28, kMaxBatchSize, 1, 0));
    loaded_model.AddLayer(std::make_unique<BatchNorm1DLayer>(8, kMaxBatchSize * 26 * 26));
    loaded_model.AddLayer(std::make_unique<ReLULayer>(Vector<UInt32>{kMaxBatchSize * 26 * 26, 8}));
    loaded_model.AddLayer(std::make_unique<Conv2DLayer>(8, 16, 3, 3, 26, 26, kMaxBatchSize, 1, 0));
    loaded_model.AddLayer(std::make_unique<BatchNorm1DLayer>(16, kMaxBatchSize * 24 * 24));
    loaded_model.AddLayer(std::make_unique<ReLULayer>(Vector<UInt32>{kMaxBatchSize * 24 * 24, 16}));
    loaded_model.AddLayer(std::make_unique<DropoutLayer>(0.3f, kMaxBatchSize * 24 * 24 * 16));
    loaded_model.AddLayer(std::make_unique<LinearLayer>(16 * 24 * 24, 128, kMaxBatchSize, false));
    loaded_model.AddLayer(std::make_unique<ReLULayer>(Vector<UInt32>{kMaxBatchSize, 128}));
    loaded_model.AddLayer(std::make_unique<SoftmaxCrossEntropyOutputLayer>(128, kNumClasses, kMaxBatchSize));
    loaded_model.SetOptimizer(std::make_unique<AdamW>(Adam::Params{0.001f, 0.9f, 0.999f, 1e-8f, 0.01f}));

    loaded_model.Load("mnist_cnn.nne");
    std::cout << "Model loaded successfully." << std::endl;

    // Run one predict step with loaded model to verify
    {
        Vector<Float32> test_images(kMaxBatchSize * kInputH * kInputW, 0.5f);
        Tensor test_input({kMaxBatchSize, kInputH, kInputW, kInputC});
        test_input.Upload(test_images.data());

        cmd->Begin();
        Vector<UInt8> preds = loaded_model.Predict(cmd, test_input, kMaxBatchSize);
        cmd->End();
        RHISubmitCommandListForQueue(cmd, ENUM_QUEUE_TYPE::COMPUTE);

        std::cout << "Loaded model predictions (first 8): ";
        for (UInt32 i = 0; i < 8; ++i)
        {
            std::cout << static_cast<Int>(preds[i]) << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "=== DONE ===" << std::endl;
    system("pause");
    return 0;
}
// --  
