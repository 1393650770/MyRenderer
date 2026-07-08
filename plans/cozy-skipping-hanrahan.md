# 修复 Sample 的 BackBuffer 访问模式

## Context

Sample 的 execute lambda 捕获了 `BeginRender()` 中初始化的 `backbuffer_rtv`/`backbuffer_dsv` 指针（`[=]` 值捕获）：

```cpp
// ❌ Sample — 捕获 init-time 指针
RHI::Texture* backbuffer_rtv = window->GetViewport()->GetCurrentBackBufferRTV();
auto* rt_resource = graph.AddRetainedResource("BackBuffer", rt_desc, backbuffer_rtv);

auto pass = graph.AddRenderPass<TestData>("MainPass", ..., [=](...) {
    Vector<Texture*> rtvs = { backbuffer_rtv };  // 始终指向第 0 张 swapchain image！
});
```

但 Editor 的 execute lambda 是**每次帧动态获取**当前 swapchain image：

```cpp
// ✅ Editor — 每次帧动态获取
[=](...) {
    rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
};
```

**结果**：swapchain 循环（image 0 → image 1）时，Sample 始终渲染到 init-time 捕获的 image 0。Frame 0 acquire image 0 没问题，但 Frame 1 acquire image 1 时 Sample 还在往 image 0 上发 barrier/渲染——image 0 在 Frame 0 已经 present 了，触发 "use of presentable image outside acquire-present window" 验证错误。

## 修复

每个 Sample 的 execute lambda 中，把 `backbuffer_rtv` / `backbuffer_dsv` 替换为动态获取：

```cpp
// setup lambda 中 BackBuffer 注册不变（只在 init 时注册一次）
builder.Write(rt_resource);
if (ds_resource) builder.Write(ds_resource);

// execute lambda 中：动态获取当前 swapchain image
[=](CONST TestData& data, CommandList* in_cmd_list) {
    auto* vp = this->GetWindow()->GetViewport();
    Vector<Texture*> rtvs = { vp->GetCurrentBackBufferRTV() };
    Texture* dsv = vp->GetCurrentBackBufferDSV();
    // ... 其余不变 ...
});
```

## 改动文件

| 文件 | 改动 |
|------|------|
| [HelloTriangle.cpp](src/Sample/1-HelloTriangle/HelloTriangle.cpp) | execute lambda: `backbuffer_rtv` → `GetCurrentBackBufferRTV()` |
| [Texture.cpp](src/Sample/2-Texture/Texture.cpp) | 同上 |
| [CubeMap.cpp](src/Sample/3-CubeMap/CubeMap.cpp) | 同上 |
| [Bindless.cpp](src/Sample/5-Bindless/Bindless.cpp) | 同上（两个 Pass 都要改） |
