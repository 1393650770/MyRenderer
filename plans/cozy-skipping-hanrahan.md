# RenderGraph 完整序列化/反序列化 — HelloTriangle 通用 pass 渲染

## Context

用户从 `RendererSample-HelloTriangle` 保存了 `hello_triangle.rgraph.json`，在 Editor 中加载后看不到三角形。

### 根因分析

序列化只捕获了**资源依赖图**（pass 读/写了哪些 resource），但没有捕获**渲染逻辑**（shader、pipeline、draw call）。

**HelloTriangle 的 OnShutdown**：
```cpp
pd.name = pass->GetName();
pd.pass_kind = Render::RDGPassKind::Graphics;
// ❌ 没有 pd.shader_path = pass->GetShaderPath();
// ❌ OnInit 也没调用 pass->SetShaderPath()
for (auto* r : pass->GetReadResources())  pd.read_resources.push_back(r->GetName());
for (auto* w : pass->GetWriteResources()) pd.write_resources.push_back(w->GetName());
```

**对比 Bindless 的 OnShutdown**（已有正确模式）：
```cpp
pd.shader_path = pass->GetShaderPath();  // ✅ 运行时设置 → 定义层 → JSON
```

**Builder 默认执行路径**（无注册 callback 时）：只做 Clear，不加载 shader、不创建 pipeline、不 Draw。

### 数据流缺口总结

```
Sample 运行态              JSON 存储              Editor Builder 重建
─────────────────         ──────────             ──────────────────
shader 路径 (.vert/.frag)  shader_path ✅ 已有    ❌ 未用于创建 pipeline
vertex count = 3          ❌ 未存储              ❌ 默认只 Clear
pipeline state            ❌ 未存储              ❌ 未创建
Draw call                 ❌ 不可序列化          ❌ 不执行
```

## 方案

### 1. HelloTriangle 添加 shader_path（匹配 Bindless 模式）

**文件**: `src/Sample/1-HelloTriangle/HelloTriangle.cpp`

- `OnInit`: 添加 `rdg_pass->SetShaderPath("Shader/triangle_test");`
- `OnShutdown`: 添加 `pd.shader_path = pass->GetShaderPath();`

### 2. RDGPassDef 添加 vertex_count

**文件**: `src/Runtime/Render/Core/RenderGraphDefinition.h`

```cpp
struct RDGPassDef {
    // ... existing fields ...
    String shader_path;
    UInt32 vertex_count = 3;   // ← 新增，默认 3（三角形）
};
```

### 3. 序列化 vertex_count

**文件**: `src/Runtime/Render/Core/RenderGraphSerializer.cpp`

- Save: `if (pd.vertex_count != 3) pj["vertex_count"] = pd.vertex_count;`
- Load: `pd.vertex_count = pj.value("vertex_count", (UInt32)3);`

### 4. Builder 通用 pass 执行 — shader 加载 + 绘制

**文件**: `src/Runtime/Render/Core/RenderGraphBuilder.cpp`

扩展 `MinimalPassData` 和 setup/execute lambda：

**Setup 阶段**（当无注册 callback 但有 `shader_path` 时）：
- 读取 `pd.shader_path` + ".vert.spv" / ".frag.spv"
- 创建 VS + PS shader
- 创建 RenderPipelineState（从 BackBuffer 获取 RT 格式）
- 创建 SRB
- 存储到 `MinimalPassData`

**Execute 阶段**（默认路径，当 pipeline 存在时）：
- SetRenderTarget（BackBuffer + DepthStencil，带 clear）
- SetGraphicsPipeline
- SetShaderResourceBinding
- Draw（vertex_count 取自定义）

修改后的 execute lambda 逻辑：
```
if (有注册 callback) → 调用 callback（现有行为）
else if (有 pipeline) → 绑定 pipeline + draw（新增）
else → clear render targets（现有默认行为）
```

### 5. RenderGraphPassBase 存储 vertex_count

**文件**: `src/Runtime/Render/Core/RenderGraphPass.h`

添加 `UInt32 vertex_count = 3` 成员和 getter/setter（供 Sample 在 OnInit 设置，OnShutdown 读取）。

### 6. HelloTriangle 完整序列化

**文件**: `src/Sample/1-HelloTriangle/HelloTriangle.cpp`

- `OnInit`: `rdg_pass->SetVertexCount(3);`
- `OnShutdown`: `pd.vertex_count = pass->GetVertexCount();`

## 文件清单

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | `RenderGraphPass.h` | 添加 `vertex_count` 成员 |
| 修改 | `RenderGraphDefinition.h` | `RDGPassDef` 添加 `vertex_count` |
| 修改 | `RenderGraphSerializer.cpp` | 序列化/反序列化 `vertex_count` |
| 修改 | `RenderGraphBuilder.cpp` | 通用 shader 加载 + pipeline 创建 + Draw |
| 修改 | `Sample/1-HelloTriangle/HelloTriangle.cpp` | 设置 shader_path + vertex_count，序列化它们 |

## 验证

```bash
# 1. 编译 + 运行 HelloTriangle sample → 生成新的 JSON
xmake build RendererSample-HelloTriangle
# 运行后检查 JSON 包含 shader_path 和 vertex_count

# 2. 编译 Editor
xmake build Editor

# 3. Editor 加载 hello_triangle.rgraph.json → 应看到三角形
```
