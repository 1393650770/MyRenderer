# 可视化 RenderGraph 管线编辑器 — 下一步计划

## 当前状态

Phase 1-4 已完成 ✅：
- 专用 Pass/Resource 节点 + 彩色 Pin
- 连线验证 (Pass↔Resource OK, Pass↔Pass 拒绝)
- 属性面板 (内嵌在 RenderGraphPanel 右侧)
- JSON Save/Load (硬编码路径)
- RenderGraphDefinition 数据结构
- RenderGraphBuilder stub

## 当前缺失（需要修复）

1. **Save/Load 不保存连线拓扑** — `BuildDefinition()` 只记了 pass 的 read/write resource 名字，没有记哪些 pin 连到哪些 pin。`LoadDefinition()` 创建节点后没有创建 `BaseLink`。
2. **Load 后连线丢失** — 加载 JSON 回来，节点出现但之间没有连线。
3. **运行时 Graph 不可见** — 启动时 `EditorRenderPipeline` 构建了 ClearPass/TestPass/UIPass，但编辑器不显示。
4. **PropertiesPanel 内嵌** — 当前用 `ImGui::BeginChild` 嵌在 RenderGraphPanel 右侧，不是独立面板。
5. **OutlinePanel 未创建** — 大纲树形视图还没做。
6. **Save/Load 路径硬编码** — 没有文件选择对话框。
7. **Link 颜色未按 PinAccess 区分** — 所有连线都是白色。

---

## Phase 5: 编辑器体验完善（核心）

> 修复连线拓扑保存、DockSpace布局、OutlinePanel、Link着色

### 5.1 修复连线拓扑保存

**问题**：`BuildDefinition()` 不保存 link 信息（哪个 pin 连到哪个 pin），`LoadDefinition()` 不恢复 link。

**方案**：在 `RenderGraphDefinition` 中增加 `RDGEdgeDef`，保存连线拓扑。

**修改 [RenderGraphDefinition.h](src/Runtime/Render/Core/RenderGraphDefinition.h)**

新增结构体：

```cpp
// Edge (link) connecting two pins
struct RDGEdgeDef
{
    String source_node_name;  // 源节点名
    String source_pin_name;   // 源 Pin 名
    String target_node_name;  // 目标节点名
    String target_pin_name;   // 目标 Pin 名
    Int edge_type = 0;        // 0=Read, 1=Write, 2=Create（PinAccess 值）
};

// 在 RenderGraphDefinition 中增加字段：
Vector<RDGEdgeDef> edges;
```

**修改 [RenderGraphPanel.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp)**

`BuildDefinition()` 中遍历 `links`：
```cpp
for (auto* link : links)
{
    BasePin* start_pin = GetItemByID(link->GetStartID())->AsPin();
    BasePin* end_pin   = GetItemByID(link->GetEndID())->AsPin();
    if (!start_pin || !end_pin) continue;

    Render::RDGEdgeDef edge;
    edge.source_node_name = start_pin->GetBelongNode()->GetName();
    edge.source_pin_name  = start_pin->GetName();
    edge.target_node_name = end_pin->GetBelongNode()->GetName();
    edge.target_pin_name  = end_pin->GetName();
    edge.edge_type = (Int)end_pin->GetPinAccess();
    def.edges.push_back(edge);
}
```

`LoadDefinition()` 中恢复连线：
```cpp
for (auto& ed : def.edges)
{
    BaseNode* src_node = name_to_node[ed.source_node_name];
    BaseNode* tgt_node = name_to_node[ed.target_node_name];
    if (!src_node || !tgt_node) continue;

    BasePin* src_pin = src_node->GetPinByName(ed.source_pin_name);
    BasePin* tgt_pin = tgt_node->GetPinByName(ed.target_pin_name);
    if (!src_pin || !tgt_pin) continue;

    BaseLink* link = new BaseLink("Link");
    link->Init(src_pin->GetSelfID(), tgt_pin->GetSelfID());
    links.push_back(link);
}
```

**修改 [BaseNode.h](src/Editor/UI/BaseNode.h)** — 新增 `GetPinByName()` 方法：
```cpp
BasePin* GetPinByName(CONST String& name)
{
    for (auto* p : input_pins) if (p->GetName() == name) return p;
    for (auto* p : output_pins) if (p->GetName() == name) return p;
    return nullptr;
}
```

### 5.2 修改序列化器保存/加载 edge

**修改 [RenderGraphSerializer.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphSerializer.cpp)**

Save:
```json
"edges": [
    {"source_node": "GBufferPass", "source_pin": "GBufferAlbedo",
     "target_node": "GBufferAlbedo", "target_pin": "Input", "edge_type": 1},
    {"source_node": "GBufferAlbedo", "source_pin": "Output",
     "target_node": "LightingPass", "target_pin": "GBufferAlbedo", "edge_type": 0}
]
```

Load: 解析 edges 数组，填充 `def.edges`。

### 5.3 Link 按 PinAccess 着色

**修改 [BaseLink.cpp](src/Editor/UI/BaseLink.cpp)**

当前 `BaseLink::Draw()` 只调 `ed::Link(self_id, start_id, end_id)` 没有颜色参数。

新增 `access_type` 成员到 `BaseLink`：
```cpp
// BaseLink.h 增加
PinAccess link_access = PinAccess::Read; // 默认 Read
void SetLinkAccess(PinAccess a) { link_access = a; }

// BaseLink.cpp Draw() 中
ImColor color = RenderGraphColors::GetLinkColor(link_access);
ed::Link(self_id, start_id, end_id, color);
```

**修改 [RenderGraphPanel.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp)** `BaseOperator()`

创建 Link 时根据 PinAccess 设置 link 颜色：
```cpp
// 确定连接语义
PinAccess link_access;
// 如果 start_pin 是 Pass 的 Output → Write/Create
// 如果 start_pin 是 Resource 的 Output → Read
BaseNode* start_node = start_pin->GetBelongNode();
if (dynamic_cast<RenderGraphPassNode*>(start_node))
    link_access = start_pin->GetPinAccess(); // Pass output = Write or Create
else
    link_access = PinAccess::Read; // Resource output = Read

link->SetLinkAccess(link_access);
```

### 5.4 DockSpace 多面板布局

**修改 [EditorUI.cpp](src/Editor/EditorRender/EditorUI.cpp)**

将 `Init()` 中的 `AddPanelUI(RenderGraphPanel::GetTypeName())` 改为 DockSpace 布局：

```
┌──────────────────────────────────┬───────────┐
│           Menu Bar               │ Properties│
├──────────────────────────────────┤  Panel    │
│                                  ├───────────┤
│      RenderGraphPanel            │ Outline   │
│       (节点图主编辑区)             │  Panel    │
│                                  │           │
├──────────────────────────────────┴───────────┤
│              Compile Log Output              │
└──────────────────────────────────────────────┘
```

具体实现：
```cpp
void EditorUI::Init(Window* in_window)
{
    // ... existing ImGui/Vulkan init ...

    // Build dockspace
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    // Or use explicit DockBuilder API for fixed layout

    // Register all panels
    AddPanelUI(RenderGraphPanel::GetTypeName());
    AddPanelUI(PropertiesPanel::GetTypeName());
    AddPanelUI(OutlinePanel::GetTypeName());
}
```

**同时**，从 `RenderGraphPanel::Draw()` 中移除内嵌的 PropertiesPanel（`ImGui::BeginChild("PropertiesRegion"...)`），让它作为独立 DockSpace 面板工作。

**修改 [RenderGraphPanel.h](src/Editor/UI/RenderGraphEditor/RenderGraphPanel.h)**

- 移除 `PropertiesPanel* properties_panel` 成员
- 选中状态通过全局/静态方式共享（`GetSelectedNode()/SetSelectedNode()` 保持 public）

### 5.5 创建 OutlinePanel

**新建 [OutlinePanel.h/.cpp](src/Editor/UI/RenderGraphEditor/OutlinePanel.h)**

- 继承 `BasePanel`，注册为 `"OutlinePanel"`
- 需要获取 `RenderGraphPanel` 的 nodes/links 引用（可通过 `EditorUI` 中转或全局访问）
- 树形视图：
  ```
  📁 Passes (3)
    ├─ 🎨 GBufferPass
    ├─ 🎨 LightingPass
    └─ 🎨 PostProcessPass
  📁 Resources (5)
    ├─ 🖼 GBufferAlbedo
    ├─ 🖼 SceneDepth
    └─ ...
  ```
- 点击选中 → 同步选中状态（通过 `RenderGraphPanel::SetSelectedNode`）

共享节点的方案：在 `EditorUI` 中持有 `RenderGraphPanel*` 引用，OutlinePanel 通过 `EditorUI` 拿到节点列表。

### 5.6 运行时 Graph → 编辑器同步

**修改 [EditorUI.h](src/Editor/EditorRender/EditorUI.h)**

添加：
```cpp
Render::RenderGraph* GetRenderGraph() { return graph_ptr; }
void SetRenderGraph(Render::RenderGraph* g) { graph_ptr = g; }
```

**修改 [Render.cpp](src/Editor/EditorRender/Render.cpp)**

在 `BeginRender()` 构建完 graph 后：
```cpp
editor_ui.SetRenderGraph(&graph);
```

**新建方法** `SyncRuntimeToEditor` 在 `RenderGraphPanel` 中：
```cpp
void RenderGraphPanel::SyncRuntimeToEditor(Render::RenderGraph* graph)
{
    // 遍历 graph->passes
    for (auto& pass : graph->passes)
    {
        auto* node = new RenderGraphPassNode(pass->GetName(), PassNodeType::Custom);
        for (auto* res : pass->read_resources)
            node->AddInputPin(res->GetName(), PinAccess::Read);
        for (auto* res : pass->write_resources)
            node->AddOutputPin(res->GetName(), PinAccess::Write);
        for (auto* res : pass->create_resources)
            node->AddOutputPin(res->GetName(), PinAccess::Create);
        node->BindPass(pass.get());
        nodes.push_back(node);
    }

    // 遍历 graph->resources
    for (auto& res : graph->resources)
    {
        // 确定资源类型
        ResourceNodeType rtype = res->IsTextureResource() ? ResourceNodeType::Texture
            : res->IsBufferResource() ? ResourceNodeType::Buffer
            : ResourceNodeType::Texture;

        auto* node = new RenderGraphResourceNode(res->GetName(), rtype);
        node->SetIsTransient(res->GetIsTransient());
        node->AddInputPin("Input", PinAccess::Write);
        node->AddOutputPin("Output", PinAccess::Read);
        node->BindResource(res.get());
        nodes.push_back(node);
    }

    // 自动布局
    AutoLayout();
}
```

---

## Phase 6: 增强功能

### 6.1 文件选择对话框

使用 ImGui 内置文件对话框或 TinyFileDialog 替代硬编码路径。

方案一：使用 [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)（需添加依赖）
方案二：编写简单的 ImGui 文件浏览器（利用已有 `backup/UI/Editor_UI.cpp` 中被注释掉的 AssetBrowser 逻辑）
方案三：使用 Windows 原生 `GetOpenFileNameA/GetSaveFileNameA`（简单但不可移植）

**推荐方案二**：用 ImGui 列表 + `std::filesystem` 写一个轻量文件选择弹窗。

### 6.2 节点模板

**修改 [RenderGraphPanel.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp)**

右键空白处增加 "Add from Template" 子菜单：

| 模板 | Input Pins | Output Pins | 自动创建 Resource 节点 |
|------|-----------|-------------|---------------------|
| DepthPrePass | — | SceneDepth(Write) | SceneDepth(DS) |
| GBufferPass | SceneDepth(Read) | GBufferAlbedo(Write), GBufferNormal(Write), GBufferRoughness(Write), SceneDepth(Write) | GBufferAlbedo, GBufferNormal, GBufferRoughness, SceneDepth |
| ShadowPass | — | ShadowMap(Write) | ShadowMap |
| LightingPass | GBufferAlbedo(Read), GBufferNormal(Read), SceneDepth(Read) | LightBuffer(Write) | LightBuffer |

### 6.3 编译/执行集成

**修改 [RenderGraphPanel.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp)**

在 GraphMenu 中增加 "Build" 菜单项：
```cpp
if (ImGui::MenuItem("Build & Compile"))
{
    auto def = BuildDefinition();
    RenderGraph* runtime_graph = /* get from EditorUI */;
    RenderGraphBuilder::BuildRuntimeGraph(def, runtime_graph);
    runtime_graph->Compile();
    // 显示编译结果
}
```

---

## 实现顺序

```
5.1 修复连线拓扑保存      ← 最核心的缺失功能
  │
5.2 序列化器更新          ← 依赖 5.1
  │
5.3 Link 着色             ← 独立，可并行
  │
5.4 DockSpace 布局        ← 改变EditorUI结构
  │
5.5 OutlinePanel          ← 依赖 5.4
  │
5.6 运行时→编辑器同步      ← 依赖已有节点类型
  │
6.1 文件对话框            ← 独立增强
  │
6.2 节点模板              ← 独立增强
  │
6.3 编译/执行集成          ← 依赖 5.6 + Builder
```

---

## 文件清单（本次）

### 新建文件

| 文件 | Phase | 说明 |
|------|-------|------|
| `src/Editor/UI/RenderGraphEditor/OutlinePanel.h/.cpp` | 5.5 | 大纲面板 |

### 修改文件

| 文件 | Phase | 改动 |
|------|-------|------|
| `src/Runtime/Render/Core/RenderGraphDefinition.h` | 5.1 | 增加 `RDGEdgeDef` + `edges` 字段 |
| `src/Editor/UI/RenderGraphEditor/RenderGraphPanel.h` | 5.3,5.4,5.6 | 移除内嵌 PropertiesPanel, 增加 SyncRuntimeToEditor, Link access |
| `src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp` | 5.1,5.3,5.6 | BuildDefinition 保存 edges, LoadDefinition 恢复 edges, Link 着色, 运行时同步 |
| `src/Editor/UI/RenderGraphEditor/RenderGraphSerializer.cpp` | 5.2 | 序列化/反序列化 edges |
| `src/Editor/UI/BaseNode.h` | 5.1 | 增加 `GetPinByName()` |
| `src/Editor/UI/BaseLink.h/.cpp` | 5.3 | 增加 `link_access` + `SetLinkAccess` + 着色 Draw |
| `src/Editor/EditorRender/EditorUI.h` | 5.4,5.6 | 增加 DockSpace init, GetRenderGraph/SetRenderGraph |
| `src/Editor/EditorRender/EditorUI.cpp` | 5.4,5.6 | DockSpace 布局, 注册所有面板 |
| `src/Editor/EditorRender/Render.cpp` | 5.6 | 调用 SetRenderGraph |
