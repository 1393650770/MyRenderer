# 双窗口渲染 — RenderGraph Editor + Render View

## Context

当前 Editor 是单窗口架构：

```
GLFW Window "MXRender" (1280×960)
├─ ClearPass → 清空 backbuffer
├─ TestPass → 渲染 3D 三角形到 backbuffer
└─ UIPass  → ImGui Dockspace (编辑器面板) 叠加绘制到同一 backbuffer
```

用户希望默认开启两个窗口：
- **渲染窗口**：显示 RenderGraph 渲染管线的输出（纯 3D 画面，无编辑器叠加）
- **编辑器窗口**：显示 RenderGraph 节点编辑器（节点图 + 属性面板 + 大纲面板）

## 关键发现

**`ImGuiConfigFlags_ViewportsEnable` 已经开启。** ImGui 的 GLFW+Vulkan 后端已经支持多视口：

- 当 ImGui 窗口位置超出主视口范围时，ImGui 自动创建新的 OS 窗口（GLFW window）
- 辅视口的 VkSwapChain/渲染由 `ImGui::UpdatePlatformWindows()` + `ImGui::RenderPlatformWindowsDefault()` 内部处理
- `VK_CommandBuffer::EndUI()` 已正确调用这两个函数（VK_CommandBuffer.cpp:355-360）
- 辅视口的资源管理完全由 ImGui backend 负责，与 RHI 不冲突

**这意味着不需要创建第二个 GLFW 窗口，不需要第二个 VK_Viewport/SwapChain。**

## 方案

核心思路：把编辑器 UI 窗口移到主视口外部 → ImGui 自动创建第二个 OS 窗口。

```
渲染窗口 (主 GLFW 窗口, 1280×960)       编辑器窗口 (ImGui 辅视口, 自动创建)
┌────────────────────────────┐       ┌──────────────────────────────┐
│                            │       │ MXRender Editor              │
│   ClearPass + TestPass     │       │ ┌──────────────────────────┐ │
│   3D 渲染输出              │       │ │ RenderGraph 节点编辑     │ │
│                            │       │ │ Properties 属性面板      │ │
│   (无 ImGui 叠加)          │       │ │ Outline 大纲面板         │ │
│                            │       │ └──────────────────────────┘ │
└────────────────────────────┘       └──────────────────────────────┘
```

**工作原理**：
- 主视口：没有任何 ImGui 窗口 → `ImGui::GetDrawData()` 为空 → `ImGui_ImplVulkan_RenderDrawData` 是 no-op → 3D 渲染结果直接显示在渲染窗口
- 辅视口：编辑器窗口位于主视口外部 → ImGui GLFW backend 自动创建新窗口 → `RenderPlatformWindowsDefault()` 在新窗口上渲染编辑器 UI → swapchain/present 全由 ImGui 管理

## 改动

### 1. `src/Editor/Editor.cpp` — 调整渲染窗口标题

- 标题从 `"MXRender"` 改为 `"MXRender - Render View"`
- ~30 字符的简单改动

### 2. `src/Editor/EditorRender/EditorUI.h` — 添加 `show_editor` 标志

```cpp
Bool show_editor = true; // 编辑器窗口是否可见
```

### 3. `src/Editor/EditorRender/EditorUI.cpp` — 核心改动：UIPass 重写

**删除** (行 125-151)：主视口的全屏 dockspace
```cpp
// 删除以下代码：
ImGuiViewport* viewport = ImGui::GetMainViewport();
ImGui::SetNextWindowPos(viewport->WorkPos);
ImGui::SetNextWindowSize(viewport->WorkSize);
ImGui::SetNextWindowViewport(viewport->ID);
// ... MainDockSpace + dockspace ...
```

**替换为**：编辑器窗口（定位在主视口外部）

```cpp
// === 渲染窗口（主视口）：不绘制任何 ImGui — 3D 渲染结果直接可见 ===

// === 编辑器窗口（辅视口）：位于渲染窗口右侧 ===
if (show_editor)
{
    ImGuiViewport* main_vp = ImGui::GetMainViewport();
    ImVec2 editor_pos = ImVec2(main_vp->Pos.x + main_vp->Size.x + 10, main_vp->Pos.y);
    ImVec2 editor_size = ImVec2(1280, 960);

    ImGui::SetNextWindowPos(editor_pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(editor_size, ImGuiCond_FirstUseEver);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags editor_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    Bool editor_open = true;
    ImGui::Begin("MXRender Editor", &editor_open, editor_flags);
    ImGui::PopStyleVar(2);

    // FPS
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %.2f (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);

    // 编辑器 Dockspace
    ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_None);

    // 绘制所有面板
    for (auto& panel : panels)
    {
        panel->Draw();
    }

    ImGui::End();

    if (!editor_open) show_editor = false;
}
```

### 4. `src/Editor/UI/BasePanel.cpp` — 移除 OnBegin 中的 FPS

- `BasePanel::OnBegin()` 中的 FPS 行移除（FPS 现在在编辑器窗口级别显示）

## 不改变的文件

- **Window.h/.cpp** — 不需要第二个 GLFW 窗口，现有单窗口架构不变
- **VK_Viewport/VK_SwapChain** — 辅视口由 ImGui 管理
- **Render.cpp (ClearPass/TestPass)** — 依然渲染到主窗口 swapchain
- **RenderGraphPanel/PropertiesPanel/OutlinePanel** — 面板代码完全不改
- **CommandQueue/CommandHistory** — 不需要改动
- **VK_CommandBuffer (BeginUI/EndUI)** — `RenderPlatformWindowsDefault()` 已存在

## 行为细节

| 场景 | 行为 |
|------|------|
| 首次启动 | 渲染窗口(左) + 编辑器窗口(右, ImGui 自动创建) |
| 关闭编辑器窗口 | `show_editor = false`，渲染窗口继续显示 3D 画面 |
| 拖拽面板到渲染窗口 | 用户可把面板拖回主视口（ViewportsEnable 支持） |
| 窗口 resize | 渲染窗口 resize 触发 VK_Viewport swapchain 重建；编辑器窗口 resize 由 ImGui 管理 |
| 布局持久化 | ImGui 的 `imgui.ini` 自动记住窗口/面板位置 |

## 验证

```bash
xmake build Editor  # 编译通过
```

运行时验证：
1. 看到两个窗口 — 左侧渲染窗口(3D 画面)、右侧编辑器窗口(RenderGraph 编辑器)
2. 编辑器窗口可正常编辑节点、连线、undo/redo
3. 关闭编辑器窗口不会退出程序
4. 可拖拽面板在窗口之间移动
