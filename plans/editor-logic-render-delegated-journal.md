# UE 风格三线程：Logic / Render / RHI

## 核心矛盾

三线程跑不起来的原因只有一个：**ImGui 嵌在 RenderGraph 的 UIPass 里**。

```
UIPass lambda (目前在 OnRender → graph.Execute 里):
  BeginUI()           ← 调 ImGui_ImplGlfw_NewFrame(), 必须和 glfwPollEvents 同线程
  panel->Draw()       ← widget 调用, 必须和 NewFrame 同线程
  EndUI()             ← ImGui::Render() + 录制 GPU 命令
```

解决方案：**ImGui 从 RenderGraph 拆出来**。

## 架构

```
Logic Thread (main)                  Render Thread                   RHI Thread
────────────────                     ─────────────                   ──────────
glfwPollEvents()
editor_ui.BeginFrame_Logic()          WaitFrameReady                 Wait replay
  ImGui_ImplGlfw_NewFrame             
  ImGui::NewFrame                     OnPreRender(ctx):              Replay CB
                                        Rebuild graph if needed      
OnUpdate(dt)                            Update backbuffer            End CB
  ProcessAll commands                  OnRender():                   Present
  Check pending build                    graph.Execute()             
                                        (scene passes only)          
ImGui widgets (panel->Draw)             
                                        Record ImGui draw data       
editor_ui.EndFrame_Logic()              (ImGui_ImplVulkan_RenderDrawData)
  ImGui::Render()                     
  → draw_data 存入 FrameContext        Swap → RHI                    
                                       wait RHI replay               
OnPrepareFrameContext(ctx)             Present                       
                                       OnPostRender                  
SignalFrameReady ─────────────────→    SignalDone ─────────────────→
WaitFrameComplete ←────────────────                                  
```

## 具体改动

### 1. EditorUI 拆为两阶段

```cpp
// Logic 线程：NewFrame + widgets + Render, 返回 ImDrawData
ImDrawData* EditorUI::DrawFrame_Logic() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // widget 渲染（原本在 UIPass lambda 里）
    if (show_editor) {
        ImGui::Begin("MXRender Editor", ...);
        for (auto& panel : panels) panel->Draw();
        ImGui::End();
    }
    ImGui::Render();
    return ImGui::GetDrawData();
}

// Render 线程：录制 ImGui GPU 命令
void EditorUI::DrawFrame_Render(ImDrawData* draw_data, VkCommandBuffer cmd) {
    if (!draw_data) return;
    ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);
}
```

### 2. 删除 UIPass

- `EditorUI::AddPass()` → 删除
- `OnInit_Render` → 不再调用 `editor_ui.AddPass(&graph)`
- `RebuildFromDefinition` → 不再调用 `editor_ui.AddPass(&graph)`

### 3. FrameContext 传 ImDrawData

```cpp
struct FrameContext {
    ...
    ImDrawData* draw_data = nullptr;  // Logic 线程生成，Render 线程消费后 delete
};
```

### 4. Window.cpp ThreeThread

```cpp
case ThreeThread: {
    // Logic 线程：ImGui CPU
    ImDrawData* dd = editor_ui.DrawFrame_Logic();
    
    Fill FrameContext: ctx.draw_data = dd;
    OnUpdate(dt);
    OnPrepareFrameContext(ctx);
    SignalReady();
    WaitFrameComplete();  // 同步点
}
```

### 5. RenderThreadMain

```cpp
WaitFrameReady();
OnPreRender(ctx);
Begin CB;
OnRender();  // graph.Execute — scene passes only
// ImGui GPU 录制
if (ctx.draw_data) {
    ImGui_ImplVulkan_RenderDrawData(ctx.draw_data, cmd_buffer);
    delete ctx.draw_data;
}
End CB;
Swap → wait RHI → Present → SignalDone;
```

## 文件修改

| 文件 | 变更 |
|------|------|
| `EditorUI.h/cpp` | 新增 `DrawFrame_Logic` / `DrawFrame_Render`, 删除 `AddPass` |
| `Render.cpp` | 删除两处 `editor_ui.AddPass()` |
| `RenderFrameData.h` | FrameContext 增加 `ImDrawData* draw_data` |
| `Window.cpp` | ThreeThread 调 `DrawFrame_Logic`, 恢复 FrameSynchronizer |
| `RenderFrameSync.cpp` | 恢复 StartRenderThread + RenderThreadMain |
