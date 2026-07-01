# Plan: 修复 SequentialModel::Predict() 数组越界风险

## Context

`SequentialModel::Predict()` 中 `prob_data[s * num_classes + c]` 存在数组越界风险。根本原因是循环边界 `in_batch_size` 由调用者传入，与 `prob_data` 的实际大小（由输出张量决定）没有任何约束关系。当 `in_batch_size > 输出张量第一维` 时就会越界。

当前 call site 恰好不会触发（Transformer demo 中 `in_batch_size=8` ≤ `128`），但函数本身缺少防御性检查，未来修改可能引入 bug。

## 修改文件

**[Model.cpp](src/Sample/6-NeuralNetwork/Model.cpp)** — `Predict()` 方法（第 220-244 行）

### 修改内容

从输出张量 shape 推导**实际行数**，用 `std::min` 钳制循环边界：

```cpp
// 修改前（第 224-228 行附近）：
UInt32 num_classes = probs.Shape().back();
Vector<UInt8> predictions(in_batch_size);
for (UInt32 s = 0; s < in_batch_size; ++s)

// 修改后：
UInt32 num_classes = probs.Shape().back();
// 从输出张量推导实际行数，防止 in_batch_size 过大导致数组越界
UInt32 num_output_rows = static_cast<UInt32>(probs.ElementCount()) / num_classes;
UInt32 num_predictions = (std::min)(in_batch_size, num_output_rows);
Vector<UInt8> predictions(num_predictions);
for (UInt32 s = 0; s < num_predictions; ++s)
```

### 变更要点

| 变更 | 说明 |
|------|------|
| 新增 `num_output_rows` | 从张量实际大小推导行数（= ElementCount / num_classes） |
| 新增 `num_predictions` | `min(in_batch_size, num_output_rows)` 钳制 |
| `predictions` 大小 | 从 `in_batch_size` 改为 `num_predictions` |
| 循环边界 | 从 `in_batch_size` 改为 `num_predictions` |

### 为什么用 `ElementCount() / num_classes` 而不是 `Shape().front()`

- 对 2D 输出（当前所有场景）两者等价
- `ElementCount() / num_classes` 对高维张量也正确（将所有非类别的维度一起算作 batch 维）
- 更健壮

## 不需要修改的文件

- [Model.h](src/Sample/6-NeuralNetwork/Model.h) — 接口不变
- [NNE_Transformer.cpp](src/Sample/6-NeuralNetwork/NNE_Transformer.cpp) — call site 不变，返回值 `Vector<UInt8>` 大小对调用者透明
- [NNE_MNIST_CNN.cpp](src/Sample/6-NeuralNetwork/NNE_MNIST_CNN.cpp) — 同上

## 验证

1. **编译验证**：用 xmake 构建 `RendererSample-NNE_Transformer`
2. **运行验证**：运行 exe，确认训练 + 保存 + 加载 + 预测全流程正常
3. **边界测试**（可选）：临时修改 call site 传入超大 `in_batch_size`，确认不会崩溃且返回结果截断到实际行数
