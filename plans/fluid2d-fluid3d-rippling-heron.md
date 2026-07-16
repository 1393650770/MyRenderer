# Fluid3D：帧头新增 ClearPass（vkCmdClearColorImage 替代 compute fill）

## Context

用户建议把泼溅纹理的清屏从 SplatPass 内部拆出来，放到帧头一个专门的 Clear pass。查证后确认这是净简化：引擎已有 `CommandList::ClearTexture`（`VK_CommandBuffer.cpp:157-246`），可用 `vkCmdClearColorImage` 替代现在的两个 fill_img compute dispatch，并删掉一整套 fill 资源。

可行性已验证：
- `ClearTexture` 有录制路径（`RHICmdClearTexture` + replay case）→ ThreeThread 安全
- 非附件彩色纹理走 else 分支：内部 `TransitionTextureState(tex, CopyDest)` → `ClearColorImage`（先 `FlushBarriers()`，TRANSFER_DST_OPTIMAL 布局，`VK_CommandBuffer.h:221-232`）
- `VkClearColorValue` union：引擎填 `float32[]`，R32_UINT 镜像按 `.uint32` 读同一内存 → 传 `+INFINITY`（位型 0x7F800000）即 FAR 哨兵，传 `0.0f` 即 0
- ink 纹理 usage 无 depth 位、不在 state_cache.render_targets 中 → 必走 else（ClearColorImage）分支
- `GenerateImageCreateInfo` 恒加 TRANSFER_DST usage → vkCmdClearColorImage 合法

## 改动（仅 `src/Sample/8-Fluid3D/Fluid3D.cpp` + 删 1 个 shader 文件）

### 1. 新增 Fluid3DClearPass（第一个 AddRenderPass，帧头）

```
setup:   builder.Write(ink_depth_res, ENUM_RESOURCE_STATE::CopyDest);
         builder.Write(ink_thick_res, ENUM_RESOURCE_STATE::CopyDest);
execute: RecordClear(cmd):
         cmd->ClearTexture(ink_depth_u, {+INFINITY, 0, 0, 0});   // uint 位型 = FAR 哨兵
         cmd->ClearTexture(ink_thick_u, {0.0f, 0, 0, 0});
SetIsCullable(false)
```
`+INFINITY` 用 `std::numeric_limits<Float32>::infinity()`。ClearTexture 内部自转 CopyDest；SplatPass 开头已有的 `TransitionTextureState(…, UnorderedAccess)` 把它转回 GENERAL（TRANSFER→COMPUTE 屏障自动生成，首个 dispatch flush）。

### 2. RecordSplat 删除两个 fill_img dispatch

只留 transition + splat dispatch。

### 3. 删除 fill_img 全套资源

- `resource/Shader/Sample/fluid3d_fill_img.comp` 文件删除
- C++：`pso_fill_img`、`srb_fill_far`、`srb_fill_zero`、`fill_far`、`fill_zero` buffer 及其创建/上传/绑定/析构、shader 加载与 PSO 创建、`FAR_BITS` 常量（哨兵改由 +inf clear 产生；shader 侧 blur_h 的 isinf 判断不变）
- **保留** `fill_buf.comp` / `fill_grid`（cell_count 是 buffer，无 vkCmdFillBuffer 封装，仍用 compute 清）
- 新增成员方法 `RecordClear(RHI::CommandList*)`

### 4. 每帧状态环（depth_u/thick_u）

Clear(UAV→CopyDest，首帧 Undefined→CopyDest) → Splat(CopyDest→UAV) → BlurH(imageLoad, 保持 UAV) → 下一帧 Clear。

## 验证

1. `xmake build RendererSample-Fluid3D` → `xmake run RendererSample-Fluid3D`
2. LMB 喷水 8s：水流+积水正常、无残留；松开后轨迹立即消失（clear 生效的直接证据）
3. 静置：画面干净（纸底+池线+水体），无闪烁；RMB 扰动正常
4. 控制台无 validation 报错（messenger 对 ERROR 会 abort）
5. 回归构建 Editor + Fluid2D（无 Runtime 改动，应直接通过）

## 关键文件

- `src/Sample/8-Fluid3D/Fluid3D.cpp` — 新增 ClearPass、删 fill_img 资源
- `resource/Shader/Sample/fluid3d_fill_img.comp` — 删除
- 参考：`src/Runtime/RHI/Vulkan/VK_CommandBuffer.cpp:157-246`（ClearTexture）、`VK_CommandBuffer.h:221-232`（ClearColorImage）
