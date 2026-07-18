# Fluid3D 泡沫白水（Phase 2 第一步）：foam 通道接通全渲染链

> **状态：已完成并经用户验收（水位稳定 + 无限放水 + 白沫手感调定：速度阈值 4.0 / 冲击阈值 1.5 / 充能 0.5·dt·8 / DECAY 1.8 / JET_SPEED 3.0）。回归构建 Editor/Fluid2D/Fluid3D 全部通过。**

## Context

水位问题已确认修复（用户验证通过）。当前问题：**高速水流没有白沫感**。原因明确：`fluid3d_g2p.comp` 已经在逐粒子维护 foam 因子（vel.w：高速充能、随时间指数衰减），但**泡沫从未进入渲染链**——splat 不泼溅它、blur 不传递它、display 不渲染它。本轮把 foam 通道从粒子接到屏幕。

## 改动

### 1. 新增 retained 纹理 `ink_foam_u`（R32U，STORAGE，640×480）

- `Fluid3D.cpp`：创建 + `AddRetainedResource("InkFoamU")` + ClearPass 增加 `ClearTexture(ink_foam_u, {0,...})` + ClearPass/SplatPass/BlurH 的 builder 声明补上 + shutdown 释放

### 2. `fluid3d_g2p.comp`：泡沫充能加"冲击"项

现有 `speed > 2.0` 充能保留，追加落水冲击充能（FLIP 速度突变是冲击/湍流的天然代理）：
`foam = min(foam + 0.5 * length(v_new - v_old) * dt * 8.0, 1.0)`（系数起调值）

### 3. `fluid3d_splat.comp`：泼溅泡沫

- 绑定新增 `vel`（读 vel.w 泡沫因子）与 `uimg_foam`（r32ui）
- 圆盘循环内：`imageAtomicAdd(uimg_foam, p, uint(foam * nd * FOAM_SCALE))`（FOAM_SCALE=1024 定点，与厚度同式）
- C++ srb_splat 增加两个绑定

### 4. `fluid3d_blur_h.comp` / `fluid3d_blur_v.comp`：b 通道传泡沫

- blur_h：绑定 `uimg_foam`，定点还原后按厚度同款高斯模糊，输出到 rgba16f 的 **b 通道**（现为 `vec4(depth, thick, 0, 0)` → `(depth, thick, foam, 0)`）
- blur_v：b 通道同样 9-tap 高斯透传
- C++ srb_blur_h 增加 uimg_foam 绑定

### 5. `fluid3d_display.frag`：白沫合成

- `foam = 1.0 - exp(-0.6 * ink.b)`（Beer 式映射）
- 水色之上叠加白沫：`col = mix(col, vec3(0.93, 0.95, 0.96), foam * mask)`
- 白沫区抑制镜面高光：`spec *= (1.0 - foam)`（泡沫是漫反射的）
- 白沫区菲涅尔反射同样衰减 `fresnel *= (1.0 - 0.7*foam)`（保留少量）

### 6. 顺带清理

- `fluid3d_prefill.comp` 删除已不用的 WATER_H/GRAVITY/DT_EST 常量
- todo 清单同步（水位项完结）

## 验证

1. `xmake build RendererSample-Fluid3D` → `xmake run`
2. LMB 喷水：射流落点出现白色泡沫团，随水流扩散、数秒内消散；静水面无泡沫
3. RMB 大冲量扰动：扰动中心泛白
4. 静水 60s：无泡沫残留、水位保持（回归确认上一轮修复未破坏）

## 关键文件

- `resource/Shader/Sample/fluid3d_splat.comp` / `fluid3d_blur_h.comp` / `fluid3d_blur_v.comp` / `fluid3d_display.frag` / `fluid3d_g2p.comp` / `fluid3d_prefill.comp`
- `src/Sample/8-Fluid3D/Fluid3D.cpp`（纹理/绑定/ClearPass/pass 声明）
