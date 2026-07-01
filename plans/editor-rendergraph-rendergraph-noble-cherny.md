# 可视化 RenderGraph 管线编辑器 — 实现计划

## Context

项目已有基础的节点图编辑器 (`RenderGraphPanel`)，使用 `ax::NodeEditor` 提供创建节点/Pin/连线的交互能力。同时运行时 `RenderGraph` 实现了 Pass/Resource DAG 调度。**但两者完全脱节** — 编辑器操作的是纯 UI 抽象，与运行时无关；序列化是空桩。

本次计划聚焦**实际的管线编辑能力**：
- 在编辑器中**新建 Pass 节点**，配置它的输入输出 RT
- **新建 Resource 节点**（Texture/Buffer），编辑其属性（格式/尺寸等）
- **连线编辑** — 把 Pass 的输出连到 Resource，Resource 连到 Pass 的输入
- **完整的 Save/Load** — 把编辑好的管线保存到文件，再加载回来
- 编辑器中的图结构能**同步到运行时 RenderGraph** 进行编译执行

参考：**Frostbite Frame Graph**（显式 Resource 节点位于 Pass 之间）、**Blender Shader Editor**（JSON 序列化节点树）、**Unity VFX Graph**（节点式管线编辑 UX）。

---

## Phase 1: 管线编辑核心 — 专用节点 + 属性编辑

> **这是整个计划的核心**。让用户能在编辑器中实际搭建渲染管线。

### 1.1 专用节点类型

**新建 [RenderGraphPassNode.h/.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphPassNode.h)**
- 继承 `BaseNode`，代表一个渲染 Pass
- 持有 `RenderGraphPassBase* bound_pass`（可选弱引用，编辑模式下可为空）
- **Input Pins**（左侧）：每个代表该 Pass 读取的资源（Read），Pin 名称 = 资源名，蓝色圆点
- **Output Pins**（右侧）：每个代表该 Pass 写入/创建的资源（Write/Create），Pin 名称 = 资源名，黄色/绿色圆点
- 右键菜单：「Add Input Pin」(读取一个 RT)、「Add Output Pin」(输出一个 RT)、「Rename Pass」、「Delete Pass」
- Header 按 Pass 类型着色：Graphics=深蓝、Compute=深绿、Copy=深黄、UI=深紫、Custom=深灰

**新建 [RenderGraphResourceNode.h/.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphResourceNode.h)**
- 继承 `BaseNode`，代表一个渲染资源（RenderTarget Texture / Buffer）
- **Input Pins**（左侧）：谁写入这个资源（Write/Create 的来源），单个 Pin
- **Output Pins**（右侧）：谁读取这个资源（Read 的目标），可以有多个 Pin
- 资源类型图标：Texture 显示 🖼 图标色=青色，Buffer 显示 📊 图标色=橙色
- Transient 资源（图内创建，生命周期由 RDG 管理）用虚线边框
- External/Imported 资源（外部传入，如 BackBuffer）用实线边框 + 白色边框高亮
- 右键菜单：「Edit Resource Properties」、「Delete Resource」

**新建 [RenderGraphNodeColors.h](src/Editor/UI/RenderGraphEditor/RenderGraphNodeColors.h)**
- 集中管理所有节点/连接的颜色常量
- `GetPassColor(PassType)` / `GetResourceColor(ResourceType)` / `GetPinColor(PinAccess)`

### 1.2 连线规则

**新建 [RenderGraphConnectionValidator.h/.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphConnectionValidator.h)**
- 连线合法性检查（在 `BaseOperator()` 拖拽连线时调用）：
  - **Pass Output Pin → Resource Input Pin**：✓ 允许（Pass 写入资源）
  - **Resource Output Pin → Pass Input Pin**：✓ 允许（Pass 读取资源）
  - **Resource Input Pin → Resource Input Pin**：✗ 禁止
  - **Pass → Pass 直连**：✗ 禁止（必须经过 Resource 节点中转）
  - **同节点自连接**：✗ 禁止
  - **同名 Pin 重复连接**：✗ 禁止
- 拖拽时实时显示错误提示（红色文字标签）

### 1.3 属性编辑面板

**新建 [PropertiesPanel.h/.cpp](src/Editor/UI/RenderGraphEditor/PropertiesPanel.h)**
- 继承 `BasePanel`，注册为 `"PropertiesPanel"`
- 接收共享的选中节点指针

**选中 Pass 节点时显示**：
```
┌─ Pass Properties ─────────────┐
│ Name:    [GBufferPass_______] │  ← 可编辑
│ Type:    [Graphics       ▼]  │  ← 下拉选择
│ Cullable: [✓]                 │  ← 复选框
│                               │
│ ▼ Input Resources (Read):     │
│   📄 SceneDepth     [×]      │  ← 可删除
│   📄 GBufferNormals [×]      │
│   [+ Add Input Resource]      │  ← 添加按钮
│                               │
│ ▼ Output Resources (Write):   │
│   📄 GBufferAlbedo   [×]     │
│   📄 GBufferRoughness[×]     │
│   [+ Add Output Resource]     │
└───────────────────────────────┘
```

**选中 Resource 节点时显示**：
```
┌─ Resource Properties ─────────────┐
│ Name:  [GBufferAlbedo__________]  │
│ Type:  Texture ▼                   │
│                                   │
│ ── Texture Settings ──            │
│ Format: [RGBA8            ▼]     │  ← 从 RenderGraph.fbs 枚举
│ Width:  [1920]  Height: [1080]   │
│ Mip:    [1]     Samples: [1]     │
│ Usage:  ☑ ColorAttachment         │
│         ☑ ShaderResource          │
│         ☐ DepthAttachment         │
│                                   │
│ ── Resource Info ──               │
│ Created by: GBufferPass           │  ← 只读
│ Read by: LightingPass, SSAOPass  │  ← 只读
│ Is Transient: ✓ (managed by RDG) │  ← 只读
└───────────────────────────────────┘
```

### 1.4 修改 BasePin

**[BasePin.h](src/Editor/UI/BasePin.h)** — 添加：
- `enum class PinAccess { Read, Write, Create }` — Pin 的语义类型
- `PinAccess access_type` 成员
- `Draw()` 中按 access_type 显示不同颜色圆点

### 1.5 修改 RenderGraphPanel

**[RenderGraphPanel.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp)** 修改右键菜单：

**画布空白处右键**：
```
┌──────────────────────┐
│ ➕ Add Pass          │ → 子菜单选择 Pass 类型
│    Graphics Pass     │
│    Compute Pass      │
│    Copy Pass         │
│    Custom Pass       │
│ ─────────────────── │
│ ➕ Add Resource      │ → 子菜单选择资源类型
│    RenderTarget (2D) │
│    DepthStencil      │
│    Buffer (Uniform)  │
│    Buffer (Storage)  │
│    External Texture  │ → 导入外部资源
└──────────────────────┘
```

**Pass 节点右键**：Add Input Resource / Add Output Resource / Rename / Delete
**Resource 节点右键**：Edit Properties / Delete

### 验证
1. 启动 Editor → 在画布空白处右键 → "Add Graphics Pass" → 节点出现
2. 右键 "Add RenderTarget" → Resource 节点出现
3. 从 Pass 的 Output Pin 拖线到 Resource 的 Input Pin → 连线成功（黄色线表示 Write）
4. 从 Resource 的 Output Pin 拖线到另一个 Pass 的 Input Pin → 连线成功（蓝色线表示 Read）
5. 尝试 Pass→Pass 直连 → 被拒绝，显示红色提示
6. 点击 Resource 节点 → PropertiesPanel 显示格式/尺寸编辑器
7. 修改 RT 格式 → 节点刷新显示

---

## Phase 2: 数据结构 — Graph 定义的中间表示

> 编辑器和运行时之间的桥梁。编辑器产出 `RenderGraphDefinition`，运行时消费它。

### 2.1 新建 RenderGraphDefinition

**新建 [RenderGraphDefinition.h/.cpp](src/Runtime/Render/Core/RenderGraphDefinition.h)**

这是纯数据结构，不包含任何 lambda/执行逻辑，专门用于序列化和编辑器交互：

```cpp
// 资源描述
struct ResourceDef {
    String name;
    enum Type { Texture, Buffer } type;
    // Texture
    TextureFomat texture_format;
    UInt32 width, height;
    UInt8 mip_level, samples;
    TextureType texture_type;
    TextureUsageType usage;
    // Buffer
    UInt64 buffer_size;
    UInt32 buffer_stride;
    UInt32 buffer_type; // Uniform/Storage/Vertex/Index
    // Metadata
    bool is_transient;       // 图内创建 vs 外部导入
    bool is_depth_stencil;
};

// Pass 描述
struct PassDef {
    String name;
    enum PassType { Graphics, Compute, Copy, Custom } type;
    bool is_cullable = true;
    Vector<String> read_resources;   // 资源名列表
    Vector<String> write_resources;  // 资源名列表
    Vector<String> create_resources; // 资源名列表
};

// 完整的图定义
struct RenderGraphDefinition {
    String graph_name;
    Vector<PassDef> passes;
    Vector<ResourceDef> resources;
    // 编辑器元数据（布局信息）
    struct NodeLayout { String name; float pos_x, pos_y; };
    Vector<NodeLayout> node_layouts;
};
```

### 2.2 编辑器 ↔ 定义 转换

在 `RenderGraphPanel` 中：
- **`BuildDefinition()`**：遍历 `nodes` 和 `links`，构建 `RenderGraphDefinition`
- **`LoadDefinition(def)`**：根据定义创建 Pass/Resource 节点，恢复连线

### 验证
- 手动构造一个 `RenderGraphDefinition`（2 Pass + 3 Resource），调用 `LoadDefinition` → 节点图正确显示
- 编辑后调用 `BuildDefinition` → 检查结构正确

---

## Phase 3: 序列化 — Save/Load 管线文件

> 这是你最关心的功能之一。让你编辑好管线后存盘，下次加载继续编辑。

### 3.1 JSON 序列化器

**新建 [RenderGraphSerializer.h/.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphSerializer.h)**

使用项目已有的 `nlohmann_json`，Save/Load `RenderGraphDefinition`：

```json
{
  "graph_name": "MyDeferredPipeline",
  "version": 1,
  "resources": [
    {
      "name": "GBufferAlbedo",
      "type": "Texture",
      "format": "RGBA8",
      "width": 1920,
      "height": 1080,
      "mip_level": 1,
      "samples": 1,
      "texture_type": "ENUM_TYPE_2D",
      "usage": ["COLOR_ATTACHMENT", "SHADERRESOURCE"],
      "is_transient": true
    },
    {
      "name": "SceneDepth",
      "type": "Texture",
      "format": "D32F",
      "width": 1920,
      "height": 1080,
      "is_depth_stencil": true,
      "is_transient": true
    },
    {
      "name": "BackBuffer",
      "type": "Texture",
      "format": "BGRA8",
      "is_transient": false
    }
  ],
  "passes": [
    {
      "name": "GBufferPass",
      "type": "Graphics",
      "is_cullable": true,
      "read_resources": [],
      "write_resources": ["GBufferAlbedo", "GBufferNormal", "SceneDepth"]
    },
    {
      "name": "LightingPass",
      "type": "Compute",
      "is_cullable": true,
      "read_resources": ["GBufferAlbedo", "GBufferNormal", "SceneDepth"],
      "write_resources": ["BackBuffer"]
    }
  ],
  "editor_state": {
    "nodes": [
      {"name": "GBufferPass", "x": 100, "y": 200},
      {"name": "LightingPass", "x": 500, "y": 200},
      {"name": "GBufferAlbedo", "x": 300, "y": 100}
    ],
    "zoom": 1.0,
    "offset": [0, 0]
  }
}
```

核心 API：
- `bool SaveGraph(RenderGraphDefinition& def, const String& filepath)` → 序列化为 JSON 写入文件
- `bool LoadGraph(RenderGraphDefinition& out_def, const String& filepath)` → 从 JSON 反序列化
- 使用枚举名而非数字（`"RGBA8"` 而非 `42`），便于手动编辑

### 3.2 接入编辑器菜单

**[RenderGraphPanel.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp)** `GraphMenu()` 接入：

```
Menu Bar:
  File → New Graph     (清空画布)
  File → Open...       (打开文件选择对话框，LoadGraph)
  File → Save          (SaveGraph 到当前路径)
  File → Save As...    (SaveGraph 到新路径)
```

使用 ImGui 文件对话框或系统原生对话框选择文件路径。

### 3.3 实现 RenderGraph::Searilize/Desearilize

**[RenderGraph.cpp](src/Runtime/Render/Core/RenderGraph.cpp)**
- `Searilize(filename)`：将运行时 Graph 导出为 `RenderGraphDefinition` JSON（用于调试/导出运行时状态）
- `Desearilize(filename)`：从 JSON 构建空的 Runtime Graph 结构（不含 lambda，仅供查看/编辑）

### 验证
1. 编辑器中搭建一个管线（3 Pass + 5 Resource + 连线）
2. File → Save As `test_pipeline.rgraph.json`
3. 用文本编辑器打开 JSON，确认结构正确、可读
4. File → New（清空），File → Open 加载回来 → 节点位置/连线恢复
5. 修改 JSON 中某个 RT 的 format，重新 Load → PropertiesPanel 显示更新后的值

---

## Phase 4: 编辑器 → 运行时桥梁

> 编辑器中的图结构同步到 RenderGraph 运行时，进行 Compile/Execute。

### 4.1 运行时绑定

**新建 [RenderGraphBuilder.h/.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphBuilder.h)**

根据 `RenderGraphDefinition` 构建运行时 RenderGraph：

```cpp
class RenderGraphBuilder {
public:
    // 从定义构建运行时 RenderGraph
    // 注意：pass 的 execute lambda 为空（占位），资源描述完整
    static void BuildRuntimeGraph(
        const RenderGraphDefinition& def,
        Render::RenderGraph* out_graph
    );

    // 可选：对已存在的 graph 做增量 Diff 更新
    static void UpdateRuntimeGraph(
        const RenderGraphDefinition& def,
        Render::RenderGraph* existing_graph
    );
};
```

构建流程：
1. 遍历 `def.resources` → 对每个 resource 调用 `graph->AddRetainedResource<TextureDesc/BufferDesc, Texture/Buffer>(...)` 或标记为 transient
2. 遍历 `def.passes` → 对每个 pass 调用 `graph->AddRenderPass<EmptyPassData>(...)`，在 setup 中声明 read/write/create resource
3. 调用 `graph->Compile()` 构建依赖关系和执行时间线

### 4.2 同步方向

编辑器 → 运行时（编辑完成后手动触发）：
- 点击 "Compile" 按钮 → `BuildRuntimeGraph(def, &graph)` → `graph.Compile()`
- 在 PropertiesPanel 底部显示 Compile 结果（Pass 数量、Resource 数量、Culled Pass 数量）

运行时 → 编辑器（已有运行时图时自动同步）：
- 启动时如果 `EditorRenderPipeline` 已构建好 graph（如现有的 ClearPass/TestPass/UIPass）
- 自动调用 `SyncRuntimeToEditor(graph, nodes, links)` 将运行时 Pass/Resource 反推显示为编辑器节点

### 4.3 修改 EditorRenderPipeline

**[Render.cpp](src/Editor/EditorRender/Render.cpp)**

改为双模式：
- **编辑模式**：不预建任何 Pass，提供空白画布给用户编辑管线
- **运行模式**：加载编辑好的管线 JSON → 构建 Runtime Graph → Compile → 每帧 Execute

```cpp
void EditorRenderPipeline::BeginRender() {
    editor_ui.Init(window);
    
    if (load_from_file) {
        // 从文件加载管线定义
        RenderGraphDefinition def;
        RenderGraphSerializer::LoadGraph(def, pipeline_file);
        RenderGraphBuilder::BuildRuntimeGraph(def, &graph);
        graph.Compile();
    }
    // else: 空白画布，用户手动编辑
    
    editor_ui.AddPass(&graph);
    editor_ui.SetRenderGraph(&graph);
}
```

### 验证
1. 在编辑器中搭建简单管线：1 ClearPass → 1 RT → 1 DrawPass
2. 点击 "Build & Compile" 按钮
3. 查看 Compile 输出日志：确认 Pass 数量、资源数量正确
4. 确认剔除逻辑生效（没有 consumer 的 resource 被剔除）

---

## Phase 5: DockSpace 布局 + 大纲面板

> 完善编辑器多面板布局，让编辑体验更专业。

### 5.1 DockSpace

**[EditorUI.cpp](src/Editor/EditorRender/EditorUI.cpp)** — `Init()` 中构建布局：

```
┌──────────────────────────────────────┬───────────┐
│           Menu Bar                   │           │
├──────────────────────────────────────┤ Properties│
│                                      │  Panel    │
│        RenderGraphPanel              │           │
│         (节点图编辑区)                 ├───────────┤
│                                      │ Outline   │
│                                      │  Panel    │
│                                      │           │
├──────────────────────────────────────┴───────────┤
│               Status Bar / Compile Log           │
└──────────────────────────────────────────────────┘
```

使用 `ImGui::DockBuilder` API。

### 5.2 大纲面板

**新建 [OutlinePanel.h/.cpp](src/Editor/UI/RenderGraphEditor/OutlinePanel.h)**
- 继承 `BasePanel`，注册为 `"OutlinePanel"`
- 树形视图：
  ```
  📁 Passes (3)
    ├─ 🎨 GBufferPass      (Graphics)
    ├─ 🎨 LightingPass     (Compute)
    └─ 🎨 PostProcessPass  (Graphics)
  📁 Resources (5)
    ├─ 🖼 GBufferAlbedo     (RT, 1920×1080, RGBA8)
    ├─ 🖼 GBufferNormal     (RT, 1920×1080, RGBA16F)
    ├─ 🖼 SceneDepth        (DS, 1920×1080, D32F)
    ├─ 🖼 BackBuffer        (External, BGRA8)
    └─ 📊 LightCB           (Buffer, 256B)
  ```
- 点击选中 → 同步高亮图中节点 + 更新 PropertiesPanel

### 共享选中状态

`RenderGraphPanel` 维护 `BaseNode* selected_node`（公开访问），PropertiesPanel 和 OutlinePanel 在 `Draw()` 中读取。

### 验证
- Editor 启动 → 四区域布局
- 点击节点 → Properties 同步更新
- Outline 树点击 → 图中节点高亮

---

## Phase 6: 增强 — 节点模板库 + 快捷操作

> 提升编辑效率的增量功能。

### 6.1 预设节点模板

**[RenderGraphPanel.cpp](src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp)**

定义常用 Pass 模板，右键创建时自动填充 Pins：

| 模板名 | 自动创建的 Input Pins | 自动创建的 Output Pins |
|--------|----------------------|------------------------|
| GBufferPass | SceneDepth(Read) | GBufferAlbedo(Write), GBufferNormal(Write), GBufferRoughness(Write), SceneDepth(Write) |
| DepthPrePass | — | SceneDepth(Write) |
| LightingPass | GBufferAlbedo(Read), GBufferNormal(Read), SceneDepth(Read) | LightBuffer(Write) |
| ShadowPass | — | ShadowMap(Write) |
| PostProcessPass | BackBuffer(Read) | BackBuffer(Write) |
| Empty Pass | — | — |

右键空白处 → "Add from Template" → 选择模板 → 自动创建 Pass + 关联 Resource 节点

### 6.2 键盘快捷键

| 快捷键 | 操作 |
|--------|------|
| `Delete` | 删除选中的节点/连线 |
| `Ctrl+S` | 保存 |
| `Ctrl+O` | 打开 |
| `Ctrl+N` | 新建图 |
| `Ctrl+A` | 全选节点 |
| `A` (在画布上) | 快速添加 Pass |
| `R` (在画布上) | 快速添加 Resource |
| `Ctrl+Z/Y` | Undo/Redo（可选，工作量较大） |

### 验证
- 右键 → "DepthPrePass Template" → 自动生成 Pass + SceneDepth Resource 节点（已连线）

---

## 实现顺序

```
Phase 1 (管线编辑核心)       ← 最重要，开始写
  │  专用节点 + 属性面板 + 连线规则
  │
Phase 2 (数据中间表示)        ← 编辑器 ↔ 运行时的桥梁
  │  RenderGraphDefinition
  │
Phase 3 (序列化 Save/Load)   ← 你关注的核心功能
  │  JSON 序列化 + 菜单接入
  │
Phase 4 (编辑器→运行时)      ← 让编辑的管线可编译执行
  │  RenderGraphBuilder
  │
Phase 5 (DockSpace + Outline) ← 编辑器体验完善
  │
Phase 6 (模板 + 快捷键)       ← 提效增强
```

Phase 1-3 是最小可用版本（MVP），完成后你就可以在编辑器中搭建管线并保存/加载了。

---

## 文件清单

### 新建文件 (10 个)

| 文件 | Phase | 说明 |
|------|-------|------|
| `src/Editor/UI/RenderGraphEditor/RenderGraphNodeColors.h` | 1 | 颜色常量 |
| `src/Editor/UI/RenderGraphEditor/RenderGraphPassNode.h/.cpp` | 1 | Pass 节点 |
| `src/Editor/UI/RenderGraphEditor/RenderGraphResourceNode.h/.cpp` | 1 | 资源节点 |
| `src/Editor/UI/RenderGraphEditor/RenderGraphConnectionValidator.h/.cpp` | 1 | 连线规则 |
| `src/Editor/UI/RenderGraphEditor/PropertiesPanel.h/.cpp` | 1 | 属性面板 |
| `src/Runtime/Render/Core/RenderGraphDefinition.h/.cpp` | 2 | 图定义数据结构 |
| `src/Editor/UI/RenderGraphEditor/RenderGraphSerializer.h/.cpp` | 3 | JSON 序列化 |
| `src/Editor/UI/RenderGraphEditor/RenderGraphBuilder.h/.cpp` | 4 | 编辑器→运行时 |
| `src/Editor/UI/RenderGraphEditor/OutlinePanel.h/.cpp` | 5 | 大纲面板 |

### 修改文件 (5 个)

| 文件 | Phase | 改动 |
|------|-------|------|
| `src/Editor/UI/BasePin.h` | 1 | 添加 `PinAccess` 枚举和成员 |
| `src/Editor/UI/BasePin.cpp` | 1 | Draw 中按 access 显示不同颜色 |
| `src/Editor/UI/RenderGraphEditor/RenderGraphPanel.h` | 1,3,5 | 添加 `BuildDefinition/LoadDefinition`、`selected_node`、接入序列化菜单 |
| `src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp` | 1,3,5 | 右键菜单重做、GraphMenu 接入 Save/Load、编辑模式入口 |
| `src/Editor/EditorRender/EditorUI.cpp` | 3,5 | 接入新面板注册、DockSpace 布局 |
| `src/Editor/EditorRender/Render.cpp` | 4 | 双模式（编辑/运行） |
| `src/Runtime/Render/Core/RenderGraph.cpp` | 3 | 实现 Searilize/Desearilize |

---

## 关键技术决策

1. **显式 Resource 节点**（Frostbite 风格）而非内联在 Pass 上（Unreal 风格）— 数据流更清晰，编辑更直观
2. **JSON 序列化**用已有的 `nlohmann_json`，枚举值用字符串名（如 `"RGBA8"`），文件可手动编辑
3. **RenderGraphDefinition** 纯数据结构作为编辑器 ↔ 运行时的中间表示，解耦 UI 和 Runtime
4. **Pass 的 execute lambda 不可序列化**，Save/Load 保存的是管线拓扑 + 资源配置，执行逻辑仍需 C++ 编写
