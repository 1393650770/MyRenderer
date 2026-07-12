# 三线程模式 Graph 切换不渲染 — 最终根因

## 根因

`FrameSynchronizer::AcquireWriteSlot` 只接受 `SlotState::Free` 的 slot：

```cpp
cv_logic.wait(lock, [this]() {
    return slot_states[write_index] == SlotState::Free || !render_running;
});
```

但 `Free` 状态只在 `WaitFrameComplete()` 中设置：
```cpp
slot_states[complete_index] = SlotState::Free;  // Done → Free
```

`WaitFrameComplete` 在 ThreeThread 模式中**从不被调用**（注释说 "ThreeThread mode no longer calls it"）。

**Slot 生命周期**：
```
Free → LogicWriting → Ready → RenderProcessing → Done → ❌ 停在这里
```

3帧后全部 3 个 slot 都是 Done → `AcquireWriteSlot` 永久阻塞 → logic 线程无法进入 `OnPrepareFrameContext` → `has_deferred_rebuild` 永远无法传递给 render 线程。

日志验证：前3帧有 `[Window] AcquireWriteSlot=...`，之后永远消失。

## 修复

在 `AcquireWriteSlot` 中加入 `SlotState::Done` 条件，将 Done 视为可复用的 Free slot：

```cpp
// RenderFrameSync.cpp
FrameContext* FrameSynchronizer::AcquireWriteSlot()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv_logic.wait(lock, [this]() {
        return slot_states[write_index] == SlotState::Free 
            || slot_states[write_index] == SlotState::Done  // ← 新增
            || !render_running.load(std::memory_order_acquire);
    });

    if (!render_running.load(std::memory_order_acquire))
        return nullptr;

    slot_states[write_index] = SlotState::LogicWriting;
    return &contexts[write_index];
}
```

Done → LogicWriting（跳过 Free），logic 线程直接从 render 线程已完成的位置继续写入。`complete_index` 和 `WaitFrameComplete` 在 ThreeThread 模式下不再需要。

### 文件

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | `RenderFrameSync.cpp` | `AcquireWriteSlot` 接受 Done 状态 |

### 验证

```bash
xmake build Editor
# [Window] AcquireWriteSlot 应持续打印 → OnPrepareFrameContext 持续调用
```
