# MyRenderer 详细实施计划

## Context

本计划将评审报告中识别的问题转化为具体的代码改动方案。覆盖三个核心 Phase：Barrier 正确性修复、编辑器撤销/重做、GPU 资源延迟销毁。每个 Phase 列明涉及的文件、每个函数的改动、以及验证方法。

---

## Phase 1：修复 Barrier 同步系统 🔴

### 问题链路

```
Pass Setup:  Read/Write 不记录 required_state
     ↓
Compile:     access_sequence 永远为空 → prologue_barriers 永远为空
     ↓
Execute:     ResourceBarrier() 只创建全局管线屏障 → 无 ImageLayout 转换
```

### 1.1 让 Pass 能声明资源访问状态

**文件：** [RenderGraph.h](src/Runtime/Render/Core/RenderGraph.h)

在 `RenderGraphPassBuilder` 中增加带 `ENUM_RESOURCE_STATE` 的 Read/Write 重载：

```cpp
// 新增重载 —— 在 RenderGraph.h 模板区域添加
template<typename resource_type>
resource_type* Read(resource_type* resource, ENUM_RESOURCE_STATE required_state)
{
    resource->read_passes.push_back(pass);
    pass->read_resources.push_back(resource);
    resource->access_sequence.push_back({pass, required_state, false, 0});
    return resource;
}

template<typename resource_type>
resource_type* Write(resource_type* resource, ENUM_RESOURCE_STATE required_state)
{
    resource->write_passes.push_back(pass);
    pass->write_resources.push_back(resource);
    resource->access_sequence.push_back({pass, required_state, true, 0});
    return resource;
}
```

**兼容性：** 原有 `Read(resource)` / `Write(resource)` 保持不变，不破坏现有调用。逐步将调用点迁移到带状态版本。

### 1.2 修复 Execute 中的 Barrier 应用

**文件：** [RenderGraph.cpp](src/Runtime/Render/Core/RenderGraph.cpp) `Execute()` 函数

将第 124-136 行的 barrier 应用改为走 `TransitionTextureState`：

```cpp
// 替换原来的 ResourceBarrier 调用
for (auto& bd : step.prologue_barriers)
{
    if (!bd.resource) continue;
    if (auto* tex = bd.resource->GetAsTexture())
        cmd_list->TransitionTextureState(tex, bd.dst_state);
    // Buffer barriers 仍走管线屏障（buffer 不需要 layout transition）
    else if (bd.resource->GetAsBuffer())
        cmd_list->ResourceBarrier(bd.src_state, bd.dst_state);
}
```

**说明：**
- `TransitionTextureState` 已正确实现 VkImageMemoryBarrier（[VK_CommandBuffer.cpp:284-327](src/Runtime/RHI/Vulkan/VK_CommandBuffer.cpp#L284-L327)），包括 aspect mask 推导、layout 转换、stage/access mask 设置
- Buffer 不需要 image layout transition，可以保留现有的 pipeline barrier 路径
- `GetAsTexture()` / `GetAsBuffer()` 已存在（[RenderGraphResource.h:76-82](src/Runtime/Render/Core/RenderGraphResource.h#L76-L82)），通过 virtual dispatch 返回 RHI 对象

### 1.3 填充 access_sequence（降级方案）

如果暂不修改所有 Pass 的 Read/Write 调用点，可以在 `Compile()` 中根据 pass 读写关系自动推断状态：

**文件：** [RenderGraph.cpp](src/Runtime/Render/Core/RenderGraph.cpp) `Compile()` 函数

在 barrier generation 循环（287 行之前）添加推导逻辑：

```cpp
// 自动推导 access_sequence（当 pass 未显式声明状态时）
for (auto& step : steps) {
    for (auto* r : step.pass->create_resources) {
        auto* res = const_cast<RenderGraphResourceBase*>(r);
        if (res->access_sequence.empty()) {
            // 创建后默认状态
            ENUM_RESOURCE_STATE init_state = ENUM_RESOURCE_STATE::Undefined;
            if (res->IsTextureResource()) {
                // 纹理创建后通常是 RenderTarget 或 ShaderResource
                bool first_use_is_write = false;
                for (auto* w : res->write_passes)
                    if (w == step.pass) { first_use_is_write = true; break; }
                init_state = first_use_is_write ? ENUM_RESOURCE_STATE::RenderTarget
                                                : ENUM_RESOURCE_STATE::ShaderResource;
            }
            res->access_sequence.push_back({step.pass, init_state, false, 0});
            res->tracked_state = ENUM_RESOURCE_STATE::Undefined;
        }
    }
    for (auto* r : step.pass->read_resources) {
        auto* res = const_cast<RenderGraphResourceBase*>(r);
        if (res->access_sequence.empty() || res->access_sequence.back().pass != step.pass)
            res->access_sequence.push_back({step.pass, ENUM_RESOURCE_STATE::ShaderResource, false, 0});
    }
    for (auto* r : step.pass->write_resources) {
        auto* res = const_cast<RenderGraphResourceBase*>(r);
        if (res->access_sequence.empty() || res->access_sequence.back().pass != step.pass)
            res->access_sequence.push_back({step.pass, ENUM_RESOURCE_STATE::RenderTarget, true, 0});
    }
}
```

**推荐策略：** 先实施 1.1 + 1.2，然后在关键 Pass（GBuffer、Lighting、PostProcess）中使用带状态版本的 Read/Write。1.3 作为降级兜底，保证旧代码也能产生正确的 barrier。

### 1.4 验证方法

```bash
# 构建
xmake build Editor

# 启用 Vulkan Validation Layer 运行
# 修复前：大量 VkImageMemoryBarrier 缺失警告
# 修复后：无同步验证错误

# 验证 barrier 输出
# 在 DebugDumpBarriers() 中查看 prologue_barriers 数量 > 0
```

---

## Phase 2：编辑器撤销/重做 🟠

### 2.1 新增文件

在 `src/Editor/UI/RenderGraphEditor/Commands/` 下创建：

| 文件 | 内容 |
|------|------|
| `CreateNodeCmd.h/.cpp` | 创建节点的撤销/重做 |
| `DeleteNodeCmd.h/.cpp` | 删除节点的撤销/重做（含关联连线恢复） |
| `CreateLinkCmd.h/.cpp` | 创建连线的撤销/重做 |
| `DeleteLinkCmd.h/.cpp` | 删除连线的撤销/重做 |
| `RenameCmd.h/.cpp` | 重命名节点/Pin（可 Merge 连续重命名） |
| `MoveNodeCmd.h/.cpp` | 移动节点（可 Merge 连续拖拽） |

### 2.2 核心设计模式

以 `DeleteNodeCmd` 为例，遵循项目 raw pointer 所有权模式：

```cpp
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(DeleteNodeCmd, public MXRender::UI::Command)
#pragma region METHOD
public:
    // 构造时接管 panel 中对 node+links 的所有权
    DeleteNodeCmd(RenderGraphPanel* panel, UInt64 node_id);

    VIRTUAL void Execute() OVERRIDE FINAL;  // 从 panel 移除
    VIRTUAL void Undo() OVERRIDE FINAL;     // 恢复到 panel
#pragma endregion

#pragma region MEMBER
private:
    RenderGraphPanel* panel;
    BaseNode* node_raw = nullptr;                    // panel 借出的指针
    Vector<BaseLink*> deleted_links_raw;             // 关联连线
    std::unique_ptr<BaseNode> owned_node;            // Command 持有所有权
    Vector<std::unique_ptr<BaseLink>> owned_links;   // Command 持有所有权
    BaseNode* old_selected = nullptr;                // 恢复选择
#pragma endregion
MYRENDERER_END_CLASS
```

**关键设计决策：**
- **Execute() 时将 raw pointer 移入 `unique_ptr`**（接管所有权），从 `nodes`/`links` 中移除
- **Undo() 时将 raw pointer 还给 `nodes`/`links`**（释放所有权），保持 `unique_ptr` 不释放（后续 Redo 需要）
- `DeleteNodeCmd` 在 Execute 时同时捕获该节点所有 Pin 上关联的连线（与现有 `DeleteNode()` 行为一致）

### 2.3 接入 RenderGraphPanel

**文件：** [RenderGraphPanel.h](src/Editor/UI/RenderGraphEditor/Panels/RenderGraphPanel.h)

```cpp
// 在 MEMBER 区域添加
CommandHistory command_history;  // 现已有 CommandHistory，直接使用
```

**文件：** [RenderGraphPanel.cpp](src/Editor/UI/RenderGraphEditor/Panels/RenderGraphPanel.cpp)

在 `CreateOperator()` 的创建节点/连线处：
```cpp
// 替换直接 new + push_back
auto cmd = std::make_unique<CreateNodeCmd>(this, node_type, name, pos);
command_history.Execute(std::move(cmd));
```

在 `BaseOperator()` 的创建连线处：
```cpp
auto cmd = std::make_unique<CreateLinkCmd>(this, start_pin, end_pin, access);
command_history.Execute(std::move(cmd));
```

在 `DeleteItem()` / `DeleteNode()` / `DeleteLink()` 处：
```cpp
auto cmd = std::make_unique<DeleteNodeCmd>(this, node_id);
command_history.Execute(std::move(cmd));
```

在 `GraphMenu()` 或快捷键处理处添加 Ctrl+Z / Ctrl+Y：
```cpp
if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z))
    command_history.Undo();
if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y))
    command_history.Redo();
```

### 2.4 Merge 支持

`RenameCmd` 和 `MoveNodeCmd` 实现 `CanMerge` + `Merge`：

```cpp
// RenameCmd::CanMerge
Bool CanMerge(CONST Command& other) CONST override {
    auto* r = dynamic_cast<CONST RenameCmd*>(&other);
    return r && r->node_id == this->node_id;
}
void Merge(std::unique_ptr<Command> other) override {
    // 保留 old_name，更新 new_name
    new_name = static_cast<RenameCmd*>(other.get())->new_name;
}

// MoveNodeCmd::CanMerge — 连续拖拽合并为一次移动
Bool CanMerge(CONST Command& other) CONST override {
    auto* r = dynamic_cast<CONST MoveNodeCmd*>(&other);
    return r && r->node_id == this->node_id;
}
```

### 2.5 验证方法

```bash
# 构建
xmake build Editor

# 手动测试流程：
# 1. 创建 Pass 节点 → Ctrl+Z → 节点消失
# 2. Ctrl+Y → 节点恢复
# 3. 创建连线 → Ctrl+Z → 连线消失
# 4. 删除节点 → Ctrl+Z → 节点及连线恢复
# 5. 重命名 → Ctrl+Z → 名称恢复
# 6. 连续拖拽多次 → 一次 Ctrl+Z 回到初始位置
# 7. 执行多步操作 → 连续 Ctrl+Z 正确回溯
```

---

## Phase 3：GPU 资源延迟销毁 🟠

### 3.1 前提分析

当前 `VK_Queue::Submit()` 在 `vkQueueSubmit` 后立即 `vkWaitForFences(UINT64_MAX)`，CPU 完全同步 GPU。因此当前阶段资源不会在 GPU 使用中被释放。但这是最大的性能瓶颈。

**延迟销毁的实现有两种路径：**
- **路径 A（保守）**：先实现延迟销毁框架（帧环队列），此时立即可用于异步提交模式
- **路径 B（激进）**：先让提交异步化，再做延迟销毁

**推荐路径 A**，因为框架改动安全且可独立验证。

### 3.2 增加非阻塞 Fence 检查

**文件：** [VK_Fence.h](src/Runtime/RHI/Vulkan/VK_Fence.h) / [VK_Fence.cpp](src/Runtime/RHI/Vulkan/VK_Fence.cpp)

```cpp
// 在 VK_Fence 中增加
Bool METHOD(CheckSignaled)()
{
    if (state == ENUM_Fence_State::Signaled) return true;
    VkResult result = vkGetFenceStatus(device, fence);
    if (result == VK_SUCCESS) { state = ENUM_Fence_State::Signaled; return true; }
    return false;
}
```

**文件：** [VK_RenderRHI.cpp](src/Runtime/RHI/Vulkan/VK_RenderRHI.cpp) `RenderEnd()` 末尾增加：
```cpp
// 每帧结束时处理延迟销毁
Render::ProcessDeferredDestruction();
```

### 3.3 实现帧环延迟销毁队列

**文件：** [RenderGraphResource.cpp](src/Runtime/Render/Core/RenderGraphResource.cpp)

```cpp
// 替换现有空桩实现
static constexpr UInt32 DEFERRED_FRAME_DEPTH = 3;  // 三帧缓冲

struct DeferredItem {
    std::unique_ptr<MXRender::RHI::RenderResource> resource;
    UInt64 frame_number;
};

static Vector<DeferredItem> g_deferred_destruction_queue;
static UInt64 g_deferred_frame_counter = 0;

void PushDeferredDestruction(std::unique_ptr<MXRender::RHI::RenderResource>&& resource)
{
    g_deferred_destruction_queue.push_back({std::move(resource), g_deferred_frame_counter});
}

void ProcessDeferredDestruction()
{
    g_deferred_frame_counter++;
    // 销毁超过 DEFERRED_FRAME_DEPTH 帧的资源
    UInt64 safe_boundary = (g_deferred_frame_counter > DEFERRED_FRAME_DEPTH) 
        ? (g_deferred_frame_counter - DEFERRED_FRAME_DEPTH) : 0;
    
    // 反向遍历以便 swap_remove
    for (Int i = (Int)g_deferred_destruction_queue.size() - 1; i >= 0; --i) {
        if (g_deferred_destruction_queue[i].frame_number <= safe_boundary) {
            g_deferred_destruction_queue[i] = std::move(g_deferred_destruction_queue.back());
            g_deferred_destruction_queue.pop_back();
        }
    }
}
```

### 3.4 修改 Derealize 走延迟销毁

**文件：** [RenderGraphResource.h](src/Runtime/Render/Core/RenderGraphResource.h)

```cpp
// RenderGraphResource::Derealize 中
void Derealize() override {
    if (GetIsTransient()) {
        // 替换: std::get<unique_ptr<actual_type>>(actual).reset();
        auto& ptr = std::get<std::unique_ptr<actual_type>>(actual);
        if (ptr) {
            PushDeferredDestruction(std::move(ptr));  // 移到队列，不立即销毁
            ptr = nullptr;
        }
    }
}
```

**注意：** `PushDeferredDestruction` 的参数类型是 `unique_ptr<RenderResource>`，而 `actual_type` 是 `Texture*` 或 `Buffer*`。需要确保 `Texture`/`Buffer` 继承自 `RenderResource`（检查继承链：`Texture → RenderResource → IObject`）。

### 3.5 验证方法

```bash
xmake build Editor
# 运行 Editor，观察：
# 1. 无崩溃（资源在正确的时机释放）
# 2. 启用 Validation Layer，无 use-after-free 警告
# 3. 用 RenderDoc 抓帧观察资源生命周期
```

---

## 改动文件汇总

| Phase | 文件 | 改动类型 |
|-------|------|----------|
| 1.1 | [RenderGraph.h](src/Runtime/Render/Core/RenderGraph.h) | 增加 Read/Write 重载 |
| 1.2 | [RenderGraph.cpp](src/Runtime/Render/Core/RenderGraph.cpp) | 修改 Execute barrier 应用 |
| 1.3 | [RenderGraph.cpp](src/Runtime/Render/Core/RenderGraph.cpp) | 增加 access_sequence 自动推导 |
| 2.1 | `Commands/CreateNodeCmd.h/.cpp` | **新建** |
| 2.1 | `Commands/DeleteNodeCmd.h/.cpp` | **新建** |
| 2.1 | `Commands/CreateLinkCmd.h/.cpp` | **新建** |
| 2.1 | `Commands/DeleteLinkCmd.h/.cpp` | **新建** |
| 2.1 | `Commands/RenameCmd.h/.cpp` | **新建** |
| 2.1 | `Commands/MoveNodeCmd.h/.cpp` | **新建** |
| 2.3 | [RenderGraphPanel.h](src/Editor/UI/RenderGraphEditor/Panels/RenderGraphPanel.h) | 增加 CommandHistory 成员 |
| 2.3 | [RenderGraphPanel.cpp](src/Editor/UI/RenderGraphEditor/Panels/RenderGraphPanel.cpp) | 接入 Command 系统 + 快捷键 |
| 3.2 | [VK_Fence.h](src/Runtime/RHI/Vulkan/VK_Fence.h) | 增加 CheckSignaled |
| 3.2 | [VK_Fence.cpp](src/Runtime/RHI/Vulkan/VK_Fence.cpp) | 实现 vkGetFenceStatus |
| 3.2 | [VK_RenderRHI.cpp](src/Runtime/RHI/Vulkan/VK_RenderRHI.cpp) | RenderEnd 调用 ProcessDeferredDestruction |
| 3.3 | [RenderGraphResource.cpp](src/Runtime/Render/Core/RenderGraphResource.cpp) | 实现帧环队列 |
| 3.4 | [RenderGraphResource.h](src/Runtime/Render/Core/RenderGraphResource.h) | Derealize 走延迟销毁 |

---

## 实施顺序

```
Phase 1.3 → Phase 1.2 → Phase 1.1 （Barrier 由兜底→完善）
    ↓
Phase 2.1 → Phase 2.3 （先创建 Command 类，再接入 Panel）
    ↓
Phase 3.2 → Phase 3.3 → Phase 3.4 （Fence → 队列 → Derealize）
```

每个 Phase 独立可验证，不依赖后续 Phase。
