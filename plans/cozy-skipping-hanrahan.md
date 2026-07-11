# RenderGraph 层修复 + UBO 序列化

## Part 1: RenderGraphResource 保留构造函数修复

### 根因

`AddRetainedResource(name, desc, nullptr)` → 保留构造函数中 `if (!in_actual)` 触发 `RealizeResource` → 创建 GPU 对象存入 `unique_ptr` variant → `Release()` 析构时 `FreeAllocation` 崩溃（pool 返回的资源 allocation 与主分配器不兼容）。

### 修复

移除保留构造函数中的自动创建逻辑：

```cpp
// RenderGraphResource.h — 修改前
explicit RenderGraphResource(CONST String& name, CONST description_type& in_description, actual_type* in_actual = nullptr)
    : RenderGraphResourceBase(name, nullptr), description(in_description), actual(in_actual)
{
    if (!in_actual)                              // ← 删除这个 block
        actual = RealizeResource<...>(...);       // ← 
}

// 修改后
explicit RenderGraphResource(CONST String& name, CONST description_type& in_description, actual_type* in_actual = nullptr)
    : RenderGraphResourceBase(name, nullptr), description(in_description), actual(in_actual)
{
    // nullptr → variant 存 Texture* = nullptr → GetAsTexture() 返回 nullptr → 安全占位
}
```

`AddRetainedResource(name, desc, nullptr)` 现在创建的是**仅描述符的占位 resource**：
- `actual` variant 存 `Texture* = nullptr`
- 析构时只丢弃 nullptr，不释放 GPU 资源
- `GetAsTexture()` 返回 nullptr，渲染时自然跳过

### 影响范围

检查所有 `AddRetainedResource` 调用点——都传入了非 null 的 actual 指针（或 ShaderDesc 有 spirv_data），不受影响。

## Part 2: UBO 序列化/反序列化

### Context

Bindless sample 中 UBO 每帧写入动态数据（MVP、Camera、Material）。当前 JSON 只序列化了 `BufferDesc`（size、stride、type），没有 buffer 内容。

### 方案

**序列化**（Sample OnShutdown）：
1. Map buffer → 读取当前内容 → 存储为 base64/字节数组
2. 存入 `RDGResourceDef.buffer_data`

**反序列化**（Builder）：
1. 读取 `BufferDesc` → `RHICreateBuffer(desc)` → 创建真实 GPU buffer
2. 读取 `buffer_data` → `UpdateBufferData()` → 上传初始数据
3. `AddRetainedResource(name, desc, buf)` → buf 非 null → raw ptr variant → 安全

### 数据流

```
Sample 运行态                  JSON                         Editor Builder
─────────────────             ──────────                   ──────────────────
BufferDesc (size/type)  →     buffer_size/type       →    RHICreateBuffer(desc)
Buffer 当前内容          →     buffer_data (bytes)    →    UpdateBufferData()
RHICreateBuffer(desc)         (base64 编码)               AddRetainedResource(name,desc,buf)
                                                          → raw ptr → 安全析构
```

### 文件清单

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | `RenderGraphResource.h` | 移除保留构造函数的 `RealizeResource` 自动调用 |
| 修改 | `RenderGraphDefinition.h` | `RDGResourceDef` 添加 `buffer_data` 字段 |
| 修改 | `RenderGraphSerializer.cpp` | 序列化/反序列化 `buffer_data` |
| 修改 | `RenderGraphBuilder.cpp` | Buffer 有 data 时创建 GPU buffer 并上传 |
| 修改 | `Sample/5-Bindless/Bindless.cpp` | OnShutdown 读取 UBO 内容存储到 buffer_data |

## 验证

```bash
xmake build Editor
xmake build RendererSample-Bindless
# 1. 运行 Bindless sample → 生成含 buffer_data 的 JSON
# 2. Editor 加载 bindless.rgraph.json → 不应崩溃
# 3. Editor 切换 graph → 不应崩溃
```
