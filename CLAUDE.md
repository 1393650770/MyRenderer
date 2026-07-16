# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

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
  Sample/               # RendererSample-* demos (one dir per sample, see Samples section)
  Reflect/meta_parser/  # MetaParser codegen tool (libclang)
  _Generated/           # Generated reflection/serializer code (do not hand-edit)
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
- Build: `xmake build Editor` (or any `RendererSample-*` target)
- Run: `xmake run <target>` — required; launching the exe directly fails to find glfw3.dll (shared libs live in the xmake package cache and `xmake run` injects the PATH). Working directory must be the output dir (`build/windows/x64/<mode>/`) because shaders load via relative `Shader/...` paths — `xmake run` handles this.
- `Runtime` is a static lib in debug, shared lib in release/releasedbg
- Pre-build (`CompileResource` target) runs automatically: ① MetaParser (libclang + mustache) generates reflection/serializer code into `src/_Generated/` ② `glslangValidator` compiles `resource/Shader/**` to `.spv` ③ `flatc` generates flatbuffers C++
- Post-build `MoveResource` copies `.spv` (flattened!), textures, and dlls into the output dir
- Key dependencies: vulkansdk, glfw, glm, imgui (docking), nlohmann_json, flatbuffers, boost, assimp
- C++20, GBK encoding for .cpp/.h files

## Code Conventions

- Macros: `MYRENDERER_BEGIN_CLASS`, `MYRENDERER_END_CLASS`, `VIRTUAL`, `METHOD()`, `OVERRIDE`, `CONST`, `MYDEFAULT`
- Type aliases: `String`, `Vector`, `Map`, `Bool`, `UInt32`, `Int`, etc. (from ConstDefine.h)
- Namespaces: `MXRender::Render`, `MXRender::RHI`, `MXRender::UI`, `MXRender::RHI::Vulkan`
- `#pragma region METHOD` / `#pragma region MEMBER` for class organization
- GBK encoding for all .cpp/.h files (keep comments ASCII to avoid transcoding issues)
- **`CHECK(flag)` / `CHECK_WITH_LOG(flag, msg)` fire when the condition is TRUE** — inverted vs standard assert (e.g. `CHECK_WITH_LOG(vkCreate...(...) != VK_SUCCESS, "...")`)
- Detailed conventions (namespace layout, macro system, file templates) live in the `myrenderer-conventions` skill

## Samples (src/Sample/)

Each sample is a single self-contained cpp with `main()`: a class deriving `MXRender::RenderInterface` overriding `OnInit_Logic / OnShutdown_Logic / OnUpdate / OnRender`, driven by `Window::Run`. `2-Texture/Texture.cpp` is the minimal RenderGraph template; `6-NeuralNetwork/` is the compute-shader reference (`ShaderHelper.h` for compute PSO creation); `7-Fluid2D/` combines both (compute sim + fullscreen draw in one RDG pass); `8-Fluid3D/` is the multi-pass reference (PBF particle sim + storage-image splatting across 5 RDG passes, retained screen-space textures with manual layout transitions, glm camera + ray-picking).

Adding a sample:
1. New `src/Sample/X-Name/Name.cpp` (with `main`)
2. Copy the 5-line target block in `xmake.lua` (`CommonProjectSetting()` + `add_files` + `set_group("Sample")` + `after_build(MoveResource)`)
3. Shaders go in `resource/Shader/Sample/` — compiled and copied automatically, but the copy is **flattened** into `Shader/`, so prefix filenames uniquely (e.g. `fluid_*`); load at runtime as `"Shader/<name>.spv"`

There is no input system: poll GLFW directly (`glfwGetMouseButton` / `glfwGetCursorPos` on `window->GetWindow()`) in `OnUpdate`. `Window.h` pulls in windows.h via GLFW/Vulkan — its min/max macros break `std::min/max`.

## RHI Gotchas (verified in practice)

- **Storage images (UAV textures) are supported**: create with `ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_STORAGE` (add `ENUM_TYPE_SHADERRESOURCE` to also sample it), bind GLSL `image2D/uimage2D` via `SetResource` (descriptor written with GENERAL layout). `imageAtomicMin/Add` need format `R32U` (`r32ui` in GLSL). Keep storage textures `mip_level = 1`. Transition with `TransitionTextureState(tex, UnorderedAccess)` before compute writes and `ShaderResource` before sampling (RDG does this automatically for transient resources declared with `Read/Write(res, state)`).
- **Compute PSOs**: use `RenderGraphiPipelineStateDesc` with only `shaders[Shader_Compute]` filled + `RHICreateRenderPipelineState` (the separate `RHICreateComputePipelineState`/`ComputePipelineState` path has no CommandList bind entry point).
- **PSO cache hashes shader pointers**: create ALL shaders first, then all PSOs, then delete the shaders — interleaving create/delete can recycle a heap address and silently return the wrong cached PSO.
- **SRB binding is by GLSL instance name** (spirv_reflect); unknown names throw. `CreateShaderResourceBinding(srb, true)` is static/bind-once semantics (later `SetResource` silently ignored) — use `false` and bind once at init for ping-pong setups. **Never call `SetResource` at runtime (per frame)**: descriptor sets are persistent and the previous frame's command buffer may still be executing (see thread mode below) — `vkUpdateDescriptorSets` on an in-flight set is undefined behavior (manifests as resources binding to the wrong frame, e.g. clears landing on the wrong texture). ALL bindings must be complete at init; this also means transient RDG textures cannot be bound through SRBs (their actual changes every frame) — use retained textures with manual `TransitionTextureState` instead until per-frame descriptor sets exist.
- **Shader param buffers must be `readonly buffer` (storage), not `uniform` blocks** — `ENUM_BUFFER_TYPE::Storage` buffers lack UNIFORM usage.
- **Barriers**: use `cmd->ResourceBarrier(src_state, dst_state)`; `MemoryBarrier(ENUM_SHADER_STAGE, ...)` misuses shader-stage bits as pipeline-stage bits (known bug). RDG auto-barriers only apply to **transient** resources at pass boundaries (per-pass state from `Read/Write(res, state)`) — retained resources, buffers, and intra-pass hazards need manual barriers. Same-state UAV→UAV across passes emits nothing (`TransitionTextureState` early-outs on equal state) — use the Fluid2D double `ResourceBarrier` after each Dispatch.
- **Frame sync / threading**: the actual thread mode is **ThreeThread** — `Platform.cpp` (Win) hardcodes `factory.threading_mode = ThreeThread` and `RenderRHI.h`'s factory default is the same; the `g_thread_mode = Single` in ConstGlobals.cpp is overwritten during RHIInit. Two frame command buffers alternate (`write_cb`/`rhi_cb`), each with its own fence, and `Begin()` only waits its OWN fence (= frame N-2), so **two frames can be in flight**: frame N's CPU work can overlap frame N-1's GPU execution. Per-frame `RHIMapBuffer` upload of param buffers inside an execute lambda is the established pattern (Fluid2D/Fluid3D) and tolerable, but is not strictly race-free; per-frame descriptor updates are NOT safe (see SRB bullet). The swapchain prefers MAILBOX whenever available (ignores the vsync flag), which makes frame overlap chronic.
- **Resource pool is LIFO** (`VK_ResourcePool`): multiple transient resources with identical descs swap physical identities every frame (returned then re-acquired in reverse order). Combined with the SRB constraint above this is why per-frame rebinding of transients corrupts frames.
- **32-bit float formats (R32F/RG32F/RGBA32F) do not guarantee linear filtering** in Vulkan (NV/AMD don't support it) — use RGBA16F for anything sampled with a linear sampler.
- Inside one RDG pass execute lambda, "N × Dispatch then SetRenderTarget + Draw" is valid: Dispatch auto-ends the render pass and both Dispatch and SetRenderTarget flush pending barriers first.
- Formats missing from `Translate_Texture_Format_To_Vulkan` throw at texture creation; `R32U/R32I` and 3D image type were added 2026-07, but many formats (R16 family int, ASTC, etc.) still fall through. `GetTextureFormatAttribs` has its own separate table (`RenderTexture.cpp`) — a format used with `TransitionTextureState` must exist in BOTH.

## RenderGraph Architecture

### Runtime (src/Runtime/Render/Core/)

- **RenderGraph**: Pass/resource container, Compile() → Execute() pipeline
  - `Compile()`: Reference counting → Culling → Topo sort → Timeline → Barrier generation → Aliasing
  - `Execute()`: Realize → Prologue barriers → Pass execute → Epilogue barriers → Derealize
  - Transient resources (`builder.Create` + `Read/Write(res, state)`): realized from the cross-frame pool (`VK_ResourcePool`), returned to it on Derealize; passes with no declared resources have ref_count 0 and MUST call `SetIsCullable(false)` or they are dropped from the timeline. **Caveat**: transient textures cannot be bound through SRBs (per-frame `SetResource` races with in-flight frames, see RHI Gotchas) — until per-frame descriptor sets exist, prefer retained textures + manual `TransitionTextureState` (`8-Fluid3D` pattern).
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
