# RenderGraphResourceImplementation 序列化接口层

## 设计

每种 `Desc` 类型在 `RenderGraphResourceImplementation` 中注册三个接口：

| 接口 | 方向 | 签名 |
|------|------|------|
| `RealizeResource<Desc, Actual>` | Desc → RHI | `unique_ptr<Actual>(const Desc&)` |
| `ResourceDescSerializer<Desc>::Serialize` | Desc → JSON | `void(json&, const Desc&)` |
| `ResourceDescSerializer<Desc>::Deserialize` | JSON → Desc | `Desc(const json&)` |

全部 7 种：

```cpp
template<typename Desc> struct ResourceDescSerializer; // 未特化 = static_assert

// 7 个特化：
template<> struct ResourceDescSerializer<RHI::TextureDesc> { ... };
template<> struct ResourceDescSerializer<RHI::BufferDesc> { ... };
template<> struct ResourceDescSerializer<RHI::ShaderDesc> { ... };
template<> struct ResourceDescSerializer<RHI::RenderPassDesc> { ... };
template<> struct ResourceDescSerializer<RHI::FrameBufferDesc> { ... };
template<> struct ResourceDescSerializer<RHI::RenderGraphiPipelineStateDesc> { ... };
template<> struct ResourceDescSerializer<RHI::ComputePipelineStateDesc> { ... };
```

`ResourceDescSerializer<Desc>::Serialize` 和 `::Deserialize` 的实现放在 `RenderGraphResourceImplementation.cpp` 中（与 7 个 `RealizeResource` 放在一起）。

## RenderGraphSerializer 改为通用调用

`SaveGraph` 改用 `std::visit`：
```cpp
std::visit([&](auto& d) {
    using T = std::decay_t<decltype(d)>;
    ResourceDescSerializer<T>::Serialize(rj, d);
}, rd.desc);
```

`LoadGraph` 根据 kind 分发：
```cpp
if (kind == RDGResourceKind::Buffer)
    rd.desc = ResourceDescSerializer<RHI::BufferDesc>::Deserialize(rj);
else if (rj.contains("spirv_data"))
    rd.desc = ResourceDescSerializer<RHI::ShaderDesc>::Deserialize(rj);
else
    rd.desc = ResourceDescSerializer<RHI::TextureDesc>::Deserialize(rj);
```

## 改动文件

| 文件 | 改动 |
|------|------|
| [RenderGraphResourceImplementation.h](src/Runtime/Render/Core/RenderGraphResourceImplementation.h) | `ResourceDescSerializer<Desc>` 模板 + 7 个特化声明 |
| [RenderGraphResourceImplementation.cpp](src/Runtime/Render/Core/RenderGraphResourceImplementation.cpp) | 7 个 `Serialize` + 7 个 `Deserialize` 实现 |
| [RenderGraphSerializer.cpp](src/Runtime/Render/Core/RenderGraphSerializer.cpp) | 删除手写 if-else，改为调用 `ResourceDescSerializer` |

## 验证

```bash
xmake build Editor        # Serializer 编译通过
xmake build RendererSample-HelloTriangle  # Sample 编译通过
```
