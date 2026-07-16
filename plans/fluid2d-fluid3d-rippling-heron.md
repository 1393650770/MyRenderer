# Fluid3D：修复水位不上升（密度饱和）+ 真实水渲染（替换水墨）

## Context

ClearPass 修复后画面干净。用户反馈两个问题：
1. **持续放水水位不上升**。此前 CPU 回读已给出证据：32768 粒子挤在 y[0.025,0.124]（12cm 层）= **7 倍静止密度**（正常应 ~73cm 深）。根因：① 压缩区每粒子真实邻居 100+，`NBR_MAX=48` 截断 + `CELL_CAP=32` 溢出丢弃 → 密度系统性低估 → PBF 压力响应不足 → 无限压缩；② `DP_MAX=0.1h`/迭代解压太慢；③ 粒子满员后环形复用把池底老粒子传送回喷嘴，总量恒定。
2. **水墨风格不像水**，希望改成真实水。屏幕空间流体链路（深度 atomicMin/厚度 atomicAdd/双边模糊）完全复用，只重写合成。

## Part A：模拟修复（水位上升）

### A1. 容量扩充（C++ 常量 + shader 顶部常量同步）

| 常量 | 旧 | 新 | 影响文件 |
|---|---|---|---|
| `NBR_MAX` | 48 | **64** | Fluid3D.cpp（nbr_buf 尺寸 32768×64）、fluid3d_neighbor/lambda/deltapos/velocity.comp |
| `CELL_CAP` | 32 | **48** | Fluid3D.cpp（cell_particles 尺寸 13200×48）、fluid3d_grid_bin/neighbor.comp |
| `SOLVER_ITERS` | 3 | **5**（保持奇数，终点仍 xstar_b） | Fluid3D.cpp |
| `DP_MAX` | 0.1·H | **0.3·H** | fluid3d_deltapos.comp（速度尖峰由 velocity 的 MAX_SPEED=5 + DAMPING 兜底，用户已调） |

### A2. 满员停止发射（替代环形复用）

`RecordSim` 记账改为：`emit_count = (firing && alive_count < MAX_PARTICLES) ? min(EMIT_PER_FRAME, MAX_PARTICLES - alive_count) : 0`；`emit_head` 逻辑不变（不再回绕覆盖活粒子）。这样注入的水永久累积，32K 满时水面应达 ~0.7m（近壁顶）。

## Part B：真实水渲染（重写 `fluid3d_display.frag`）

### B1. fp_buf 扩充（Fluid3D.cpp，64 float 内）

| idx | 内容 |
|---|---|
| [36-51] | `inv_view_proj`（列主序 memcpy，供逐像素射线重建） |
| [52-54] | 相机世界位置 eye (0,3.0,3.2) |

### B2. 合成算法（标准屏幕空间流体着色）

1. **逐像素射线**：uv→NDC→`inv_view_proj` unproject 近/远两点 → `ro=eye, rd`
2. **程序化背景 `bg(ro,rd)`**：泳池内壁（底 y=0 + 四壁到 y=0.8，footprint x±1.4/z±1.0）铺 **0.4m 浅青色瓷砖 + 深色砖缝**（checker+grid，射线-平面解析求交）；池外为暖灰台面（y=0.8 平面）；天空 = 按 rd.y 的蓝白渐变 + 太阳光斑（sun_dir≈normalize(-0.4,1.0,0.3)）
3. **水面重建**：平滑视深 d → 表面世界坐标 `Pw = ro + rd·(d/dot(rd,cam_fwd))`；世界法线 = 邻元 unproject 位置差叉积（比现在的屏幕空间梯度法线更正确）
4. **着色**：
   - Fresnel（Schlick，F0=0.02）：`F = 0.02+0.98(1-max(dot(n,-rd),0))^5`
   - 折射：`rr = refract(rd,n,0.75)` → `bg(Pw,rr)` × **Beer-Lambert 吸收** `exp(-σ·t)`，σ≈(1.0,0.4,0.25)（深水蓝绿、浅水透明）
   - 反射：`bg(Pw, reflect(rd,n))`（天空为主）
   - 高光：Blinn-Phong `pow(max(dot(n,h),0),200)`
   - `col = mix(refr,refl,F) + spec`；水体边缘用现有 mask smoothstep 与背景软混合
5. 水枪指示器保留（小十字/圆环，改中性深灰）；删除纸纹/晕影/墨色/池体勾线（池体由 bg 的 3D 瓷砖呈现）
6. `fluid3d_splat.comp`：`DRAW_R` 0.045→**0.055**（表面更连贯，利于折射）；blur 不变

### 保持不变

深度/厚度泼溅与模糊链、5+1 个 RDG pass 结构、retained 纹理 + init 绑定（描述符约束）、fp 上传方式、输入交互。

## 验证

1. `xmake build RendererSample-Fluid3D` → `xmake run`
2. **水位测试**：按住 LMB 持续放水 ~35 秒到满 32K：水面应从池底持续上升至 ~0.7m（接近壁顶），停喷后水面平静、水位保持（不再 12cm 封顶、不再有粒子被传送回喷嘴）
3. **外观**：未放水时可见 3D 泳池瓷砖 + 天空反射的空池；放水后水体透明带蓝绿色调，能看到被波浪扭曲折射的池底瓷砖，视角掠射处偏天空反射（菲涅尔），有太阳高光；RMB 扰动产生可见波纹折射变化
4. 控制台无 validation 报错；回归构建 Editor + Fluid2D

## 关键文件

- `src/Sample/8-Fluid3D/Fluid3D.cpp` — 常量、满员停喷、fp 扩充
- `resource/Shader/Sample/fluid3d_display.frag` — 全量重写（真实水合成）
- `resource/Shader/Sample/fluid3d_neighbor.comp` / `fluid3d_grid_bin.comp` / `fluid3d_lambda.comp` / `fluid3d_deltapos.comp` / `fluid3d_velocity.comp` — NBR_MAX/CELL_CAP/DP_MAX 常量
- `resource/Shader/Sample/fluid3d_splat.comp` — DRAW_R
