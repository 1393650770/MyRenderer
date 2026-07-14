# MyRenderer
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)

MyRenderer (MXRender) is a real-time rendering engine written in **C++20** on top of **Vulkan**, built with **xmake**. It features a declarative **RenderGraph** with a visual node editor, **bindless** rendering, **GPU neural network** training/inference via compute shaders, and a code-generation based **reflection** system.

## Overview

```
Editor.exe ──depends──> Runtime.lib ──includes──> ThirdParty
                                                  (vulkan, glfw, imgui, ...)
```

Strict one-way layering:
- **Runtime never includes Editor headers**
- **Render layer never includes Vulkan headers** — all GPU work goes through the RHI abstraction
- **Editor headers never include Vulkan headers** (the ImGui backend .cpp is the only exception)

## How to build

### Prerequisites
- **[xmake](https://github.com/xmake-io/xmake)** (tested with v2.9.2+)
- **Visual Studio 2022/2026** with C++20 support

### Quick Start (Generate VS Solution)

Double-click `gen_vs_sln.bat` or run in cmd:
```batch
gen_vs_sln.bat
```
This script will:
1. Automatically fix gli + imgui compatibility issues
2. Clean stale caches
3. Generate `.sln` for VS (debug / release / releasedbg, x64)

> **Note for users in China:** If GitHub downloads fail, set a proxy first:
> ```batch
> set HTTP_PROXY=http://127.0.0.1:10809
> gen_vs_sln.bat
> ```

### Manual Build

##### ① Install xmake
https://github.com/xmake-io/xmake/releases

##### ② Generate VS project
```batch
xmake project -k vsxmake -m "debug;release;releasedbg" -a x64 -y
```

##### ③ Build from command line
```batch
xmake config -m debug
xmake
```

### Build notes

- `Runtime` is built as a **static lib in debug** and a **shared lib in release/releasedbg** (symbols auto-exported).
- A pre-build step (`CompileResource`) runs automatically before every build:
  1. **Reflection codegen** — `MetaParser` (libclang) scans annotated headers and generates rttr registration + JSON serializer code into `src/_Generated/` via mustache templates
  2. **Shader compilation** — `glslangValidator` compiles everything under `resource/Shader/**` to SPIR-V
  3. **FlatBuffers codegen** — `flatc` generates C++ from the `*.fbs` schemas
- After each build, shaders/textures/dlls are copied next to the executables in `build/windows/x64/<mode>/`.

## Branch
① The release version is in the branch [**main**] 

② Old Shit code is in branch [**batch**] , but it can run

③ Each branch develops one feature and merges into the branch [**main_dev**] 

④ The most active development branch now is [**rendergraph-editor**] (RenderGraph system + visual node editor + samples)

## Feature
###### Tip: maybe still in progress
 - [x] RHI
The architecture refers to UE
 - [x] Vulkan 
The implementation refers to UE/ Nvrhi/ DiligentEngine etc
 - [x] RenderGraph
Refers to fg
 - [x] Reflection
The architecture is metaparser(refers to Piccolo) and rttr 
 - [x] RenderGraph compile pipeline
Reference counting → culling → topological sort (cycle detection) → timeline → split barrier generation → transient resource memory aliasing; plus async compute scheduling, blackboard, GraphViz dump and `.rgraph.json` (schema v2) serialization (`src/Runtime/Render/Core/RenderGraph.cpp`)
 - [x] Visual RenderGraph node editor
ax::NodeEditor based canvas with undo/redo command system, pass templates, connection/graph validators, auto-save with crash recovery and collapsible sub-graph nodes (`src/Editor/UI/RenderGraphEditor/`)
 - [x] Bindless rendering
Descriptor-indexing global descriptor set (set 2): 4096 texture2D + 256 cube + 64 sampler slots with slot alloc/free/update (`src/Runtime/RHI/Vulkan/VK_BindlessManager.cpp`)
 - [x] GPU Neural Network (NNE)
Full training on GPU via 31 compute shaders: Linear / Conv2D / MultiHeadAttention / BatchNorm / LayerNorm / Dropout / Residual blocks, ReLU/GELU/SiLU/... activations, SoftmaxCrossEntropy loss, SGD/Adam/AdamW optimizers, LR schedulers, model save/load (`src/Sample/6-NeuralNetwork/`, `resource/Shader/nn_*.comp`)
 - [x] Multi-threaded rendering
Switchable `Single` / `RHIThread` / `ThreeThread` (Logic + Render + RHI) modes with a triple-buffered `FrameSynchronizer` (`src/Runtime/Render/Core/RenderFrameSync.h`)
 - [x] GPU memory sub-allocation
UE-style: `VK_DeviceMemoryManager` (pages) → `VK_MemoryResourceFragmentAllocator` (sub-alloc) → `VK_ResourcePool` (transient reuse by desc hash) (`src/Runtime/RHI/Vulkan/VK_Memory.cpp`)
 - [x] Dynamic Rendering
Vulkan 1.3 `vkCmdBeginRendering` path with automatic fallback to legacy render passes
 - [x] Pipeline cache persistence
`PipelineCache.bin` loaded at startup / saved at shutdown; PSOs are content-hash cached
 - [x] Shader reflection based binding
SPIRV-Reflect drives descriptor set layouts and by-name resource binding (`srb->SetResource("name", ...)`)
 - [x] Staging buffer manager
Pooled staging buffers with fence-based deferred reclamation for texture/buffer uploads
 - [ ] Virtual Texture
Quadtree page layout prototype (`src/Runtime/Render/Core/VirtualTexture/`), still WIP
 - [ ] GPU-Driven rendering
Culling/depth-reduce shaders exist (`resource/Shader/gpu_culling.comp`), C++ side currently parked in `backup/`, WIP

## Editor

The `Editor` target is an ImGui (docking + multi-viewport) application centered on a **visual RenderGraph editor**:

- **RenderGraphPanel** — node-editor canvas: add/delete pass & resource nodes, link pins, right-click menus driven by the built-in `PassRegistry` (GBuffer, DepthPre, Shadow, lighting, SSAO, Bloom, TAA, ToneMapping, ...), sync a runtime graph back into the editor
- **PropertiesPanel / OutlinePanel** — selection-synced inspection via an editor-wide `EventBus`
- **Command system** — undo/redo history with transactions and command merging
- **TemplateLibrary** — built-in GBuffer / Lighting / Shadow / PostProcess pass templates, plus user `.rgtemplate.json`
- **GraphValidator + ConnectionValidator** — pre-compile checks (cycles, connectivity, naming, pin type rules)
- **AutoSaveService** — periodic atomic auto-save with crash recovery
- **Serialization** — graphs load/save as human-readable `.rgraph.json` (schema v2); every graphics sample exports one that the editor can open

## Sample
###### Tip: Mainly to verify the engine feature

| Target | What it demonstrates |
|---|---|
| `RendererSample-HelloTriangle` | Minimal RenderGraph pass drawing a triangle; exports `hello_triangle.rgraph.json` |
| `RendererSample-Texture` | Async texture asset loading (DDS) + sampled fullscreen quad; exports `texture.rgraph.json` |
| `RendererSample-CubeMap` | DDS cubemap skybox rendering; exports `cubemap.rgraph.json` |
| `RendererSample-Reflection` | Console demo of the MetaParser + rttr reflection (property get/set, method invoke) |
| `RendererSample-Bindless` | Bindless PBR: textures allocated into the global descriptor heap, sampled by index in shader |
| `RendererSample-NeuralNetwork` | GPU-trained MLP classifying a 2D spiral dataset (forward/backward/update all in compute) |
| `RendererSample-NNE_MNIST_CNN` | MNIST-style CNN: Conv2D + BatchNorm + Dropout + AdamW, model save/load (synthetic data) |
| `RendererSample-NNE_Transformer` | Tiny Transformer: MultiHeadAttention + LayerNorm + Residual + GELU + cosine LR schedule |
| `RendererSample-Fluid2D` | Interactive 2D stable-fluids sim (mouse splat force/dye, Jacobi pressure solve) with blurred metaball ink-wash stylized rendering, all on storage buffers |
| `RendererSample-VirtualTexture` | Virtual texture quadtree page-layout prototype |

Run any sample from its output directory (shaders are loaded via relative `Shader/...` paths), e.g.:
```batch
xmake build RendererSample-Fluid2D
xmake run RendererSample-Fluid2D
```

## Project Structure

```
src/
  Runtime/              # Engine library
    Core/               # Base types & macros (ConstDefine.h), reflection runtime
    RHI/                # Render Hardware Interface (platform-agnostic)
      Vulkan/           # Vulkan backend (VK_* classes)
    Render/Core/        # RenderGraph system (+ serializer, validator, pass registry)
    Application/        # Window / main loop (GLFW)
    Asset/              # Mesh / texture / material assets
    Platform/           # Platform abstractions
    GenCode/            # Generated code (flatbuffers schemas, embedded SPIR-V)
  Editor/               # Editor application
    EditorRender/       # EditorRenderPipeline + EditorUI (ImGui)
    UI/                 # Node/Pin/Link/Panel framework + RenderGraphEditor
  Sample/               # RendererSample-* sources
  Reflect/meta_parser/  # MetaParser (libclang based codegen tool)
  _Generated/           # Reflection & serializer generated code
resource/               # Shaders (GLSL + SPIR-V), textures (incl. Sponza), editor assets
template/               # Mustache templates for reflection codegen
```

## Third-Party

Fetched via xmake packages: `vulkansdk`, `glfw 3.4`, `glm`, `imgui 1.89.9-docking`, `assimp`, `tinyobjloader`, `gli`, `lz4`, `nlohmann_json`, `rttr`, `boost 1.84`, `flatbuffers 1.12`, `glslang`.

Vendored in-tree: `TaskScheduler` (multithreaded task scheduler), `SPIRV-Reflect`, `stb_image`, `VulkanMemoryAllocator`, `imgui-node-editor` (ax::NodeEditor), `libclang` (for MetaParser).
