# 编辑器图 → 运行时渲染：Builder 完善计划

## 当前状态

`RenderGraphBuilder::BuildRuntimeGraph` 已有：
- ✅ 从 `RDGResourceDef` 创建 RHI Texture/Buffer 资源
- ✅ 注册 retained/external 资源
- ✅ 构建 Pass 拓扑（Read/Write 声明）
- ❌ execute lambda 是空的 — 无实际渲染逻辑
- ❌ BackBuffer 没有映射到 swapchain 图像

## 需要补充

### 1. 默认 Execute Callback

对常用 Pass 类型提供默认渲染逻辑：

**ClearPass**：读取其 write_resources 中标记为 RenderTarget/DepthStencil 的资源，设置为 RT+DS 并 Clear。

**DrawPass**（Graphics）：加载默认 shader（fullscreen.vert + 简单 .frag），设置 RT+DS，Draw 全屏三角形。

**UIPass**：复用现有 EditorUI 的 ImGui 渲染逻辑。

**其他 Pass**：execute 为空（占位），拓扑正确即可。

### 2. BackBuffer 映射

在 Builder 中增加 `external_bindings` 参数：
```cpp
struct ExternalBinding {
    String resource_name;
    void* actual; // Texture* or Buffer* from swapchain/viewport
};
```
调用方传入 `{"BackBuffer", backbuffer_rtv}`, `{"DepthStencil", backbuffer_dsv}`。

### 3. Editor 集成

在 `Render.cpp` 中增加 **Run Mode**：
1. 加载 `render_graph.rgraph.json`
2. `BuildRuntimeGraph(def, &graph, external_bindings)`
3. `graph.Compile()`
4. 每帧 `graph.Execute()`

支持热切换：编辑模式 ↔ 运行模式。

### 4. 编辑器加载流程

```
手动编辑图 → Save JSON → File→Open 加载 → Build → Compile → Execute → 看到渲染结果
```

## 修改文件

| 文件 | 改动 |
|------|------|
| `Services/RenderGraphBuilder.h` | 增加 `ExternalBinding` 结构 + `BuildRuntimeGraph` 新重载 |
| `Services/RenderGraphBuilder.cpp` | 实现默认 execute callbacks + BackBuffer 绑定 |
| `EditorRender/Render.cpp` | 增加 Run Mode 支持 |
| `Panels/RenderGraphPanel.cpp` | GraphMenu 增加 "Build & Run" 按钮 |
