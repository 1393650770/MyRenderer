# Editor ImGui Vulkan 断言失败修复计划

## Context

Editor 运行时在 `ImGui_ImplVulkan_Init()` 处崩溃，报错：

```
Assertion failed: g_FunctionsLoaded && "Need to call ImGui_ImplVulkan_LoadFunctions()
if IMGUI_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!"
```

### 根因

`patch_imgui.ps1` 脚本会修改 xmake-repo 中 imgui 包的 `port/xmake.lua`，在 `vulkan=true` 配置块中注入 `add_defines("IMGUI_IMPL_VULKAN_NO_PROTOTYPES")`。这导致 **imgui.dll 在编译时定义了 `IMGUI_IMPL_VULKAN_NO_PROTOTYPES`**，其中的 `imgui_impl_vulkan.cpp` 使用函数指针表（而非直接调用）来访问 Vulkan 函数，并要求在 `ImGui_ImplVulkan_Init()` 之前调用 `ImGui_ImplVulkan_LoadFunctions()`。

但应用程序代码 [VK_Viewport.cpp](src/Runtime/RHI/Vulkan/VK_Viewport.cpp#L114) 从未调用 `ImGui_ImplVulkan_LoadFunctions()`，直接走到了 `ImGui_ImplVulkan_Init()`，触发断言。

### 两种修复方案

| 方案 | 描述 | 改动量 |
|------|------|--------|
| **A（推荐）** | 从 port 文件移除 `IMGUI_IMPL_VULKAN_NO_PROTOTYPES` 补丁，清除缓存重建 imgui.dll | 删一行 + 重建 |
| **B** | 在 `VK_Viewport.cpp` 中 `ImGui_ImplVulkan_Init()` 前加 `ImGui_ImplVulkan_LoadFunctions()` 调用 | 加几行代码 |

---

## 推荐方案：A — 移除 NO_PROTOTYPES 补丁

**理由**：项目其余部分不使用 `VK_NO_PROTOTYPES`，也没有 volk 这类显式加载器。项目直接链接 vulkan-1.lib 使用标准 Vulkan 原型。imgui.dll 也用标准原型是最自然、最一致的方案。

`ImGui_ImplVulkan_LoadFunctions()` 即使是没定义 NO_PROTOTYPES 时也安全可调用（返回 true 的 no-op），但保持 DLL 与应用程序一致是更干净的架构选择。

## 实施步骤

### Step 1: 恢复 imgui port 文件

编辑 `C:/Users/qq139/AppData/Local/.xmake/repositories/xmake-repo/packages/i/imgui/port/xmake.lua`，删除 `add_defines("IMGUI_IMPL_VULKAN_NO_PROTOTYPES")` 这一行。

### Step 2: 清除 imgui 包缓存

```bash
xmake f -c
# 或者直接删除包缓存目录
rm -rf C:/Users/qq139/AppData/Local/.xmake/packages/i/imgui/v1.89.9-docking/
```

### Step 3: 重建项目

```bash
xmake build Editor
```

### Step 4: 验证

运行 Editor，确认不再触发断言，ImGui 界面正常渲染。

---

## 备选方案：B — 添加 LoadFunctions 调用

如果方案 A 因某些原因不可行（如 imgui.dll 就是需要用 NO_PROTOTYPES），可改用方案 B。

### 修改文件

**`src/Runtime/RHI/Vulkan/VK_Viewport.cpp`**，在 `AttachUiLayer()` 方法中，`ImGui_ImplVulkan_Init()` 调用之前（约第 113 行），添加：

```cpp
// Load Vulkan function pointers for ImGui (required when imgui is built with IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* user_data) {
    return vkGetInstanceProcAddr(static_cast<VkInstance>(user_data), function_name);
}, static_cast<void*>(init_info.Instance));
```

### 验证

运行 Editor，确认不再触发断言。

---

## 涉及的关键文件

| 文件 | 角色 |
|------|------|
| [VK_Viewport.cpp](src/Runtime/RHI/Vulkan/VK_Viewport.cpp#L78-L135) | ImGui Vulkan 初始化位置，缺少 LoadFunctions 调用 |
| [patch_imgui.ps1](patch_imgui.ps1) | 向 imgui port 注入 NO_PROTOTYPES 的补丁脚本 |
| [xmake.lua](xmake.lua#L191-L199) | Editor 目标已定义 `IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING`，但未定义 `IMGUI_IMPL_VULKAN_NO_PROTOTYPES` |
| `C:/Users/qq139/AppData/Local/.xmake/repositories/xmake-repo/packages/i/imgui/port/xmake.lua` | 被 patch 修改的 imgui port 文件 |

## 验证

1. 方案 A：重建后运行 Editor，确认 ImGui 界面正常显示无断言
2. 方案 B：重新编译 Runtime 库后运行 Editor，确认 ImGui 界面正常显示无断言
