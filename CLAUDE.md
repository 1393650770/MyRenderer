# MyRenderer — Project Documentation

## Overview

MyRenderer (MXRender) is a real-time Vulkan-based rendering engine written in C++20. It features a declarative RenderGraph system with a visual node editor, GPU neural network inference, virtual texturing, and bindless rendering.

## Architecture

```
src/
  Runtime/              # Core engine library (single monolithic lib)
    Core/               # Base types, macros (ConstDefine.h), reflection
    RHI/                # Render Hardware Interface — platform-agnostic
      Vulkan/           # Vulkan backend (VK_* prefix)
    Render/Core/        # RenderGraph system
    Application/        # Window management (GLFW)
    Asset/              # Asset loading (mesh, texture, material)
    Platform/           # Platform abstractions
    GenCode/            # Generated code (flatbuffers, shader SPIR-V)
  Editor/               # Editor application
    Editor.cpp          # Entry point
    EditorRender/       # EditorRenderPipeline + EditorUI (ImGui)
    UI/
      BaseNode/Pin/Link/Panel/Item  # Generic node-graph editor base
      RenderGraphEditor/ # Visual RenderGraph node editor
        Services/       # Serializer, Builder, Validator, PassRegistry, EventBus
        Commands/       # Undo/Redo command system
        Templates/      # Pass template library
```

## Layer Dependency (Strict One-Way)

```
Editor.exe  ──depends──>  Runtime.lib  ──includes──>  ThirdParty/
                                                   (vulkan, glfw, imgui, etc.)
```

- **Runtime NEVER includes Editor headers** (src/Editor/ not in Runtime include path)
- **Render layer NEVER includes Vulkan headers** (all VK ops through RHI abstraction)
- **Editor headers NEVER include Vulkan headers** (EditorUI.cpp is the only exception, for ImGui backend)

## Build System

- **xmake** (v2.9.2+), generate VS solution: `gen_vs_sln.bat`
- Target: `xmake build Editor`
- Key dependencies: vulkansdk, glfw, glm, imgui (docking), nlohmann_json, flatbuffers, boost, assimp
- C++20, GBK encoding for .cpp/.h files

## Code Conventions

- Macros: `MYRENDERER_BEGIN_CLASS`, `MYRENDERER_END_CLASS`, `VIRTUAL`, `METHOD()`, `OVERRIDE`, `CONST`, `MYDEFAULT`
- Type aliases: `String`, `Vector`, `Map`, `Bool`, `UInt32`, `Int`, etc. (from ConstDefine.h)
- Namespaces: `MXRender::Render`, `MXRender::RHI`, `MXRender::UI`, `MXRender::RHI::Vulkan`
- `#pragma region METHOD` / `#pragma region MEMBER` for class organization
- AI-generated code: `// -- [AI]` (single line), `// -- [AI:BEGIN]` / `// -- [AI:END]` (multi-line)
- GBK encoding for all .cpp/.h files

## RenderGraph Architecture

### Runtime (src/Runtime/Render/Core/)

- **RenderGraph**: Pass/resource container, Compile() → Execute() pipeline
  - `Compile()`: Reference counting → Culling → Topo sort → Timeline → Barrier generation → Aliasing
  - `Execute()`: Realize → Prologue barriers → Pass execute → Epilogue barriers → Derealize
- **RenderGraphPass<T>**: Template pass with typed data, setup lambda (declares deps), execute lambda (records commands)
- **RenderGraphResource<Desc, Actual>**: Template resource with descriptor and actual RHI object
- **RenderGraphDefinition**: Pure-data IR for serialization (JSON)
- **RenderGraphCompileConfig**: Compile toggles + safe mode

### Editor (src/Editor/UI/RenderGraphEditor/)

- **RenderGraphPanel**: Main ImGui node-editor canvas (ax::NodeEditor)
- **GraphValidator**: Pre-compile validation (cycles, connectivity, naming)
- **RenderGraphBuilder**: Definition → Runtime graph construction (Editor-side)
- **RenderGraphSerializer**: JSON save/load (.rgraph.json, v2 schema)
- **CommandHistory**: Undo/Redo (max 256 depth, command merging)
- **EditorEventBus**: Observer pattern for panel communication
- **TemplateLibrary**: Built-in pass templates (GBuffer, Lighting, Shadow, PostProcess)

## Key Files Reference

| Purpose | Path |
|---------|------|
| Type aliases, macros | `src/Runtime/Core/ConstDefine.h` |
| RHI enums (states, formats) | `src/Runtime/RHI/RenderEnum.h` |
| RHI resource descriptors | `src/Runtime/RHI/RenderRource.h` |
| RenderGraph core | `src/Runtime/Render/Core/RenderGraph.h` |
| RenderGraphDefinition (IR) | `src/Runtime/Render/Core/RenderGraphDefinition.h` |
| RenderGraphResource + barriers | `src/Runtime/Render/Core/RenderGraphResource.h` |
| VK resource pool + bridge | `src/Runtime/RHI/Vulkan/VK_ResourcePool.cpp` |
| Editor main panel | `src/Editor/UI/RenderGraphEditor/RenderGraphPanel.cpp` |
| Graph validator | `src/Editor/UI/RenderGraphEditor/Services/GraphValidator.h` |
| Command system | `src/Editor/UI/RenderGraphEditor/Commands/CommandHistory.h` |
| Template library | `src/Editor/UI/RenderGraphEditor/Templates/TemplateLibrary.h` |

## Layering Verification Checklist

After each significant change, verify:
```bash
# No Vulkan includes in Render layer
grep -r '#include.*vulkan' src/Runtime/Render/ | wc -l  # must be 0
# No Editor includes in Runtime  
grep -r '#include.*Editor' src/Runtime/ | wc -l         # must be 0
# No VK_ includes in Render layer
grep -r '#include.*VK_' src/Runtime/Render/ | wc -l     # must be 0
```
