// NNE_Transformer.cpp - Tiny Transformer demo showcasing NNE features:
//   Linear, MultiHeadAttention, LayerNorm, ResidualBlock, GELU, SoftmaxCrossEntropy,
//   CosineAnnealingLR, Adam, Save/Load
// Uses synthetic random sequences (no external dataset files needed)
#include <iostream>
#include <random>
#include <cmath>
#include "Application/Window.h"
#include "Core/ConstDefine.h"
#include "RHI/RenderRHI.h"
#include "Layer.h"
#include "Model.h"
#include "Optimizer.h"
#include "Activation.h"
#include "Normalization.h"
#include "AttentionLayer.h"
#include "ResidualBlock.h"
#include "Initializer.h"
#include "LRScheduler.h"

using namespace MXRender::RHI;
using namespace MXRender::Application;
using namespace MXRender;
using namespace MXNN;

// ============================================================
// SyntheticSequenceData - generates random sequence classification data
// Shape: [batch, seq_len, input_dim]
// ============================================================
class SyntheticSequenceData
{
public:
    SyntheticSequenceData(UInt32 in_num_samples, UInt32 in_seq_len, UInt32 in_input_dim, UInt32 in_num_classes)
        : num_samples_(in_num_samples)
        , seq_len_(in_seq_len)
        , input_dim_(in_input_dim)
        , num_classes_(in_num_classes)
    {
        std::mt19937 rng(42);
        std::uniform_real_distribution<Float32> val_dist(-1.0f, 1.0f);
        std::uniform_int_distribution<UInt32> label_dist(0, num_classes_ - 1);

        UInt32 sample_size = seq_len_ * input_dim_;
        data_.resize(num_samples_ * sample_size);
        labels_.resize(num_samples_);

        for (UInt32 i = 0; i < num_samples_; i++)
        {
            for (UInt32 j = 0; j < sample_size; j++)
            {
                data_[i * sample_size + j] = val_dist(rng);
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
    UInt32 SeqLen() CONST { return seq_len_; }
    UInt32 FeatureSize() CONST { return seq_len_ * input_dim_; }
    UInt32 NumClasses() CONST { return num_classes_; }

    void Shuffle()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(indices_.begin(), indices_.end(), gen);
    }

    UInt32 NextBatch(Vector<Float32>& out_data, Vector<UInt8>& out_labels, UInt32 in_batch_size)
    {
        UInt32 remaining = num_samples_ - current_idx_;
        UInt32 n = (std::min)(in_batch_size, remaining);

        UInt32 sample_size = seq_len_ * input_dim_;
        out_data.resize(n * sample_size);
        out_labels.resize(n);

        for (UInt32 i = 0; i < n; i++)
        {
            UInt32 idx = indices_[current_idx_ + i];
            for (UInt32 j = 0; j < sample_size; j++)
            {
                out_data[i * sample_size + j] = data_[idx * sample_size + j];
            }
            out_labels[i] = labels_[idx];
        }

        current_idx_ += n;
        return n;
    }

    void Reset() { current_idx_ = 0; }

private:
    UInt32 num_samples_, seq_len_, input_dim_, num_classes_;
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
    std::cout << "=== NNE Tiny Transformer Demo ===" << std::endl;

    // Init window + Vulkan device
    Window window;
    window.InitWindow();

    CONST UInt32 kMaxBatch = 8;
    CONST UInt32 kMaxSeqLen = 16;
    CONST UInt32 kInputDim = 8;
    CONST UInt32 kDModel = 32;
    CONST UInt32 kNumHeads = 4;
    CONST UInt32 kFFDim = 64;
    CONST UInt32 kNumClasses = 5;

    // Flattened batch size for LayerNorm and activation layers:
    //   input is flat [B*T, d] where B*T varies
    CONST UInt32 kFlatBatch = kMaxBatch * kMaxSeqLen;

    // Build Tiny Transformer model:
    //   Input: [B, T, input_dim] = flat [B*T, input_dim]
    //   Linear(input_dim -> d_model) -> [B*T, d_model]
    //   MultiHeadAttention(d_model=32, num_heads=4) -> [B*T, d_model]
    //   ResidualBlock wrapping:
    //     Linear(32 -> 64) -> GELU -> Linear(64 -> 32)
    //   -> [B*T, d_model]
    //   LayerNorm(d_model) -> normalizes per row
    //   Linear(32 -> num_classes=5) -> SoftmaxCrossEntropy
    //
    // Note: MultiHeadAttention input is [B, T, d_model] which is the same flat buffer
    // as [B*T, d_model]. LayerNorm normalizes per row (B*T rows, d_model features).

    SequentialModel model(kMaxBatch);

    // Embedding: Linear(input_dim -> d_model), no relu
    model.AddLayer(std::make_unique<LinearLayer>(kInputDim, kDModel, kFlatBatch, false));

    // Multi-Head Self-Attention
    model.AddLayer(std::make_unique<MultiHeadAttentionLayer>(kDModel, kNumHeads, kMaxBatch, kMaxSeqLen));

    // Residual MLP block:
    //   LayerNorm(32) -> Linear(32->64) -> GELU -> Linear(64->32) -> LayerNorm(32)
    // The ResidualBlock wraps the sub-layers with skip connection.
    // n_elem = B * T * d_model = kFlatBatch * kDModel
    {
        auto residual = std::make_unique<ResidualBlock>(kFlatBatch * kDModel);
        residual->AddSubLayer(std::make_unique<LayerNorm>(kDModel, kFlatBatch));
        residual->AddSubLayer(std::make_unique<LinearLayer>(kDModel, kFFDim, kFlatBatch, false));
        residual->AddSubLayer(std::make_unique<GELULayer>(Vector<UInt32>{kFlatBatch, kFFDim}));
        residual->AddSubLayer(std::make_unique<LinearLayer>(kFFDim, kDModel, kFlatBatch, false));
        residual->AddSubLayer(std::make_unique<LayerNorm>(kDModel, kFlatBatch));
        model.AddLayer(std::move(residual));
    }

    // Output head: Linear(d_model -> num_classes)
    model.AddLayer(std::make_unique<SoftmaxCrossEntropyOutputLayer>(kDModel, kNumClasses, kFlatBatch));

    // Adam optimizer
    model.SetOptimizer(std::make_unique<Adam>(Adam::Params{0.0001f, 0.9f, 0.999f, 1e-8f, 0.0f}));

    // CosineAnnealingLR scheduler: lr_max=0.0001, lr_min=1e-6, period=50 steps
    CosineAnnealingLR lr_scheduler(0.0001f, 1e-6f, 50);

    std::cout << "[Init] Model built. Architecture:" << std::endl;
    std::cout << "  Linear(" << kInputDim << " -> " << kDModel << ")" << std::endl;
    std::cout << "  MultiHeadAttention(d=" << kDModel << ", heads=" << kNumHeads << ")" << std::endl;
    std::cout << "  ResidualBlock(MLP: " << kDModel << "->" << kFFDim << "->" << kDModel << " + LayerNorm)" << std::endl;
    std::cout << "  SoftmaxCrossEntropy(" << kDModel << " -> " << kNumClasses << ")" << std::endl;

    // Create dataset: 256 samples, seq_len=16, input_dim=8, 5 classes
    SyntheticSequenceData dataset(256, kMaxSeqLen, kInputDim, kNumClasses);

    // Get compute command buffer
    auto* cmd = RHIGetCommandListForQueue(ENUM_QUEUE_TYPE::COMPUTE);

    // Input buffer: flat [B, T, input_dim] = [B*T, input_dim]
    Tensor input_buf({kMaxBatch, kMaxSeqLen, kInputDim});

    // Training loop - 3 epochs
    for (UInt32 epoch = 0; epoch < 3; ++epoch)
    {
        dataset.Shuffle();
        dataset.Reset();

        UInt32 batch_count = 0;
        while (true)
        {
            Vector<Float32> data;
            Vector<UInt8> labels;
            UInt32 n = dataset.NextBatch(data, labels, kMaxBatch);
            if (n == 0)
            {
                break;
            }

            input_buf.Upload(data.data());

            cmd->Begin();

            Float32 loss = model.TrainStep(cmd, input_buf, labels, n);

            cmd->End();
            RHISubmitCommandListForQueue(cmd, ENUM_QUEUE_TYPE::COMPUTE);
            model.ClearTempSRBs();

            Float32 current_lr = lr_scheduler.GetLR(batch_count);
            std::cout << "[Epoch " << epoch << " Batch " << batch_count
                << "] loss=" << loss << " lr=" << current_lr << std::endl;

            batch_count++;
        }

        std::cout << "Epoch " << epoch << " done, " << batch_count << " batches" << std::endl;
    }

    // Save model to file
    std::cout << "=== Save model to transformer.nne ===" << std::endl;
    model.Save("transformer.nne");
    std::cout << "Model saved successfully." << std::endl;

    // Load model from file (rebuild architecture first)
    std::cout << "=== Load model from transformer.nne ===" << std::endl;
    SequentialModel loaded_model(kMaxBatch);
    loaded_model.AddLayer(std::make_unique<LinearLayer>(kInputDim, kDModel, kFlatBatch, false));
    loaded_model.AddLayer(std::make_unique<MultiHeadAttentionLayer>(kDModel, kNumHeads, kMaxBatch, kMaxSeqLen));
    {
        auto residual = std::make_unique<ResidualBlock>(kFlatBatch * kDModel);
        residual->AddSubLayer(std::make_unique<LayerNorm>(kDModel, kFlatBatch));
        residual->AddSubLayer(std::make_unique<LinearLayer>(kDModel, kFFDim, kFlatBatch, false));
        residual->AddSubLayer(std::make_unique<GELULayer>(Vector<UInt32>{kFlatBatch, kFFDim}));
        residual->AddSubLayer(std::make_unique<LinearLayer>(kFFDim, kDModel, kFlatBatch, false));
        residual->AddSubLayer(std::make_unique<LayerNorm>(kDModel, kFlatBatch));
        loaded_model.AddLayer(std::move(residual));
    }
    loaded_model.AddLayer(std::make_unique<SoftmaxCrossEntropyOutputLayer>(kDModel, kNumClasses, kFlatBatch));
    loaded_model.SetOptimizer(std::make_unique<Adam>(Adam::Params{0.0001f, 0.9f, 0.999f, 1e-8f, 0.0f}));

    loaded_model.Load("transformer.nne");
    std::cout << "Model loaded successfully." << std::endl;

    // Run one predict step with loaded model
    {
        Vector<Float32> test_data(kMaxBatch * kMaxSeqLen * kInputDim, 0.0f);
        Tensor test_input({kMaxBatch, kMaxSeqLen, kInputDim});
        test_input.Upload(test_data.data());

        cmd->Begin();
        loaded_model.PredictForward(cmd, test_input);
        cmd->End();
        RHISubmitCommandListForQueue(cmd, ENUM_QUEUE_TYPE::COMPUTE);
        Vector<UInt8> preds = loaded_model.GetPredictions(kMaxBatch);

        std::cout << "Loaded model predictions (all " << kMaxBatch << " samples): ";
        for (UInt32 i = 0; i < kMaxBatch; ++i)
        {
            std::cout << static_cast<Int>(preds[i]) << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "=== DONE ===" << std::endl;
    system("pause");
    return 0;
}
