# 补全所有 vkCmd 方法的 bypass 分支

## 当前状态

VK_CommandBuffer 中产生 vkCmd 的方法共 ~18 个，目前只有 7 个有 bypass。缺 11 个。

## 缺的方法

| # | 方法 | 位置 | 需要新增 |
|---|------|------|---------|
| 1 | Draw | cpp | ✅ 已有 |
| 2 | Dispatch | cpp | ✅ 已有 |
| 3 | TransitionTextureState | cpp | ✅ 已有 |
| 4 | FlushBarriers | cpp | ✅ 已有 |
| 5 | ClearTexture | cpp | ✅ 已有 |
| 6 | ResourceBarrier | cpp | ✅ 已有 |
| 7 | SetPushConstants | cpp | ✅ 已有 |
| 8 | Begin | header | ❌ |
| 9 | End | header | ❌ |
| 10 | CopyBuffer | header | ❌ |
| 11 | BeginRenderPass | header | ❌ |
| 12 | EndRenderPass | header | ❌ |
| 13 | BeginDynamicRendering | cpp | ❌ |
| 14 | EndDynamicRendering | header | ❌ |
| 15 | CopyBufferToImage | cpp | ❌ (当前 return) |
| 16 | WriteTimestamp | header | ❌ |
| 17 | ClearAttachment | header | ❌ |
| 18 | MemoryBarrier | cpp | ❌ |

## 需要新增的命令结构体

RenderCommandList.h 增加：RHICmdCopyBuffer, RHICmdBegin, RHICmdEnd, RHICmdWriteTimestamp

## 改法

每个方法加一行：
```cpp
if (!bypass) { recorded_commands.push_back(std::make_unique<RHICmdXxx>(args...)); return; }
```

Replay 中加对应 case 分支。

## 改动文件

| 文件 | 改动 |
|------|------|
| [VK_CommandBuffer.cpp](src/Runtime/RHI/Vulkan/VK_CommandBuffer.cpp) | 补 4 个 bypass 分支 |
| [VK_CommandBuffer.h](src/Runtime/RHI/Vulkan/VK_CommandBuffer.h) | 补 7 个 bypass 分支 |
| [RenderCommandList.h](src/Runtime/RHI/RenderCommandList.h) | +4 命令结构体 |
