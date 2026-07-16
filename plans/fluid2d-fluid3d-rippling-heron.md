# 8-Fluid3D：水枪 + 蓄水池 + 水墨屏幕空间流体（PBF）+ 配套引擎功能补全

## Context

项目已有 `7-Fluid2D`（Stable Fluids + 单 RDG pass + 纯 storage buffer + 手动屏障）。本次新增 3D 流体示例：**水枪射水 → 积水蓄水池 → 鼠标瞄准/点击交互 → 水墨画风格渲染**，并且**不绕开引擎缺口，而是补全引擎功能后用更正规的方式实现**：

- 补 **storage image（UAV 纹理）支持** → 泼溅/模糊用真正的纹理，display 用硬件双线性采样
- 修 **RDG transient barrier 生成 bug** → 示例拆成**多个 RDG pass + transient 纹理**，由 RenderGraph 自动生成 pass 间布局转换（首个真正使用 transient 路径的样例，呼应 rendergraph-editor 分支）
- 顺带修：3D 纹理类型翻译缺失、blend dstAlphaBlendFactor 笔误 + write_mask 硬编码

用户已确认：水枪射水、固定斜俯视角、屏幕空间水墨、多 pass + transient、三项引擎改动全做。

---

# Part A：引擎改动（先行，独立可验证）

### A1. storage image 支持（4 处，探索已确认描述符池/反射布局/UAV→GENERAL 映射均已存在）

| # | 文件 | 改动 |
|---|---|---|
| 1 | [RenderEnum.h:126-137](src/Runtime/RHI/RenderEnum.h#L126) | `ENUM_TEXTURE_USAGE_TYPE` 加 `ENUM_TYPE_STORAGE = 1 << 7` |
| 2 | [VK_Utils.cpp:475-543](src/Runtime/RHI/Vulkan/VK_Utils.cpp#L475) `Translate_Texture_usage_type_To_VulkanUsageFlags` | 加 `EnumHasAnyFlags(STORAGE)` 分支 → `VK_IMAGE_USAGE_STORAGE_BIT` |
| 3 | [VK_Shader.cpp:235-239](src/Runtime/RHI/Vulkan/VK_Shader.cpp#L235) `SetResource` | `VK_DESCRIPTOR_TYPE_STORAGE_IMAGE` 从拒绝改为写描述符：`imageLayout=VK_IMAGE_LAYOUT_GENERAL`、`imageView=GetImageView()`、`sampler=VK_NULL_HANDLE`（模式照抄 224-234 行 combined sampler case） |
| 4 | [VK_Utils.cpp:564-837](src/Runtime/RHI/Vulkan/VK_Utils.cpp#L564) `Translate_Texture_Format_To_Vulkan` | 加 `R32U→VK_FORMAT_R32_UINT`、`R32I→VK_FORMAT_R32_SINT`（`imageAtomicMin/Add` 必须 r32ui） |

已存在无需改：描述符池已含 STORAGE_IMAGE（[VK_DescriptorSets.cpp:79](src/Runtime/RHI/Vulkan/VK_DescriptorSets.cpp#L79)）、set layout 反射直接 cast 枚举值（VK_Shader.cpp:93）、`UnorderedAccess→GENERAL` 布局映射（VK_Utils.cpp:1215）、空纹理创建（Bindless.cpp:131 先例，`GenerateImageCreateInfo` 恒加 SAMPLED|TRANSFER）、每纹理自动线性 sampler。
**约束**：storage 纹理保持 `mip_level=1`（视图覆盖全 mip，storage image 视图要求单 mip）。

### A2. RDG transient barrier 生成修复（多 pass 方案的前置条件）

[RenderGraph.cpp:358-393](src/Runtime/Render/Core/RenderGraph.cpp#L358)：read/write 两个循环里 barrier 的 `dst_state` 取的是 `res->access_sequence.back().required_state` —— 资源被多个 pass 以不同状态访问时（如 Splat 写 UAV → BlurH 读 SRV），会在**第一个 pass 就转到最后声明的状态**。修复：改为取**与当前 `step.pass` 匹配的 access entry** 的 `required_state`（该循环本来就在做 `acc.pass == step.pass` 匹配打 stamp，顺势取 state 即可）。只影响 transient 资源（现无任何样例使用；编辑器回归见验证节）。

### A3. 顺带修复

| 文件 | 改动 |
|---|---|
| [VK_Utils.cpp:546-562](src/Runtime/RHI/Vulkan/VK_Utils.cpp#L546) `Translate_Texture_type_To_Vulkan` | 加 `ENUM_TYPE_3D → VK_IMAGE_TYPE_3D` |
| [VK_PipelineState.cpp:142-159](src/Runtime/RHI/Vulkan/VK_PipelineState.cpp#L142) | ① 152 行 `dstAlphaBlendFactor` 误用 `src_alpha` → 改 `dst_alpha` ② 146 行 colorWriteMask 硬编码 RGBA → 按 `write_mask` 字段翻译（默认值须仍映射为 RGBA，不改变现有 PSO 行为） |

### A4. 文档同步

实现完成后更新 `CLAUDE.md` 的 RHI Gotchas：storage image 已支持（usage 加 STORAGE 位、SetResource 接受 STORAGE_IMAGE、r32ui 可用）、R32U/R32I/3D 纹理翻译已补、RDG transient barrier 按 pass 匹配已修。

---

# Part B：Fluid3D 示例

## 总体架构

**PBF（Macklin & Müller）粒子模拟**（32K 粒子、均匀网格定容量 cell list、无前缀和）→ **compute 泼溅**（`imageAtomicMin` 深度 / `imageAtomicAdd` 厚度到 640×480 r32ui 纹理）→ **双边模糊**（compute，输出 rg32f）→ **全屏水墨合成**（sampler2D 硬件双线性）。

**5 个 RDG pass**（AddRenderPass 顺序即执行顺序；steps 按插入序构建）：

| pass | 声明 | execute 内容 |
|---|---|---|
| SimPass | 无声明，`SetIsCullable(false)`（副作用全在外部 buffer 上） | 上传 fp → emit/predict/fill_buf/grid_bin/neighbor/[lambda,deltapos]×3/velocity/finalize，每次 Dispatch 后 Fluid2D 双屏障 |
| SplatPass | `Create+Write(depth_u, UAV)`、`Create+Write(thick_u, UAV)` | 每帧 `SetResource` 实体化纹理 → fill_img×2 清屏 → `ResourceBarrier(UAV,UAV)` → splat |
| BlurHPass | `Read(depth_u, UAV)`、`Read(thick_u, UAV)`（imageLoad）、`Create+Write(f_a, UAV)` | 开头手动 `ResourceBarrier(UAV,UAV)`（同态跨 pass 写→读，RDG 不发屏障）→ dispatch |
| BlurVPass | `Read(f_a, SRV)`、`Create+Write(f_b, UAV)` | dispatch（UAV→SRV 转换由 RDG prologue 自动做） |
| DisplayPass | `Read(f_b, SRV)`、`Write(backbuffer)`(+dsv) | SetRenderTarget + 全屏三角形 Draw |

**Transient 纹理**（`builder.Create<RenderGraphResource<TextureDesc,Texture>, TextureDesc>`，RDG 自动 Realize/池化/Derealize/pass 间布局转换）：

| 纹理 | 格式 | 大小 | usage | 生命期 |
|---|---|---|---|---|
| depth_u / thick_u | R32U | 640×480 | STORAGE（自动附 SAMPLED） | Splat→BlurH |
| f_a（R=深度,G=厚度） | RG32F | 640×480 | STORAGE | BlurH→BlurV |
| f_b | RG32F | 640×480 | SHADERRESOURCE\|STORAGE | BlurV→Display |

`mip_level=1`、`resource_state=Undefined`。**引用 transient 纹理的 SRB 必须每帧在 execute lambda 里 `SetResource`**（池化实体指针可能变化；`GetActual()` 仅在实体化区间有效；FlushDescriptorWrites 由 Dispatch/Draw 自动触发；单帧 in-flight 安全）。buffer 绑定仍 init 时一次。
风险提示：RG32F 线性过滤在桌面 N/A/Intel 全支持，但非规范强制；若遇不支持改 RGBA16F。

## 文件清单

| 文件 | 作用 |
|---|---|
| `src/Sample/8-Fluid3D/Fluid3D.cpp` | 唯一 C++（含 main），骨架镜像 Fluid2D.cpp |
| `fluid3d_emit.comp` | 环形发射（喷嘴 + 锥形 jitter） |
| `fluid3d_predict.comp` | 重力 + RMB 径向冲量 + x*=pos+v·dt |
| `fluid3d_fill_buf.comp` | uint buffer 填充（清 cell_count，参数 buffer {value,count}） |
| `fluid3d_grid_bin.comp` | 入格 atomicAdd（CELL_CAP=24） |
| `fluid3d_neighbor.comp` | 27 格 → 定长邻居表（NBR_MAX=48，按 x*_a 建一次） |
| `fluid3d_lambda.comp` / `fluid3d_deltapos.comp` | PBF λ / Δp+s_corr+盒碰撞（xstar_a/b ping-pong） |
| `fluid3d_velocity.comp` | v=(x*_b−pos)/dt + XSPH（邻速由位置重算，只写 vel） |
| `fluid3d_finalize.comp` | pos=x*_b + NaN 重置池心 |
| `fluid3d_fill_img.comp` | 清 r32ui 纹理（imageStore，2 个 SRB：depth=FAR_BITS / thick=0） |
| `fluid3d_splat.comp` | 投影粒子→圆盘循环 `imageAtomicMin(uimg_depth)` / `imageAtomicAdd(uimg_thick)` |
| `fluid3d_blur_h.comp` | `imageLoad` 两张 uimage2D → 位型/定点还原 → 水平 9-tap（深度双边）→ `imageStore` rg32f f_a |
| `fluid3d_blur_v.comp` | `texelFetch(sampler2D f_a)` → 垂直 9-tap → `imageStore` rg32f f_b |
| `fluid3d_display.frag` | `texture(sampler2D f_b)` 硬件双线性 → 水墨合成 + 池体勾线 + 水枪指示 |
| 复用 `resource/Shader/fullscreen.vert` | 全屏三角形 |
| `xmake.lua` | Fluid2D block（~314-318 行）后复制 5 行：`RendererSample-Fluid3D` |

Shader 全部 `fluid3d_` 前缀（build 拷贝扁平化），加载路径 `"Shader/fluid3d_xxx.comp.spv"`。GBK 编码、Tab、注释 ASCII-only。**每个声明的 binding 必须在代码中实际使用**（SPIR-V 反射剔除未用绑定 → SetResource 抛异常）。绑定全 set=0，`SetResource` 名 = GLSL 实例名/变量名（image/sampler 用变量名，如 `uimg_depth`）。

## GPU Buffer（模拟部分不变，全 `ENUM_BUFFER_TYPE::Storage`，复用 Fluid2D `CreateStorageBuffer`/`UploadFloats`，新增 `UploadUInts`）

| buffer | 布局 | 大小 |
|---|---|---|
| pos_buf / vel_buf / xstar_a / xstar_b | float×4/粒子 | 各 512KB |
| lambda_buf | float/粒子 | 128KB |
| nbr_buf / nbr_count | uint×48 / uint | 6MB / 128KB |
| cell_count / cell_particles | uint / uint×24 每格 | 51.6KB / 1.21MB |
| fp_buf | float×64 | 256B（每帧上传） |
| fill_grid（{0,13200}）/ fill_far（{FAR_BITS}）/ fill_zero（{0}） | uint×2 | 各 8B（init 上传一次） |

**fp_buf**：`[0]`dt(clamp[1/240,1/60]) `[1]`alive `[2]`emit_start `[3]`emit_count `[4-6]`nozzle_pos `[7]`fire_flag `[8-10]`emit_vel `[11]`frame_seed `[12-14]`interact_pos `[15]`interact_flag `[16-31]`view_proj(列主序 memcpy) `[32-34]`aim_target `[35]`time_sec `[36-63]`保留。

## 常量（C++ 与 shader 顶部同步硬编码，Fluid2D 惯例）

```
MAX_PARTICLES=32768  LOCAL=256(1D)/16x16(2D)  EMIT_PER_FRAME=64  JET_SPEED=3.5
H=0.1  SPACING=0.05  MASS=1.0  REST_DENSITY=8000(起始,可调)  EPS_CFM=100
SOLVER_ITERS=3(必须奇数,链路a→b→a→b终点xstar_b)  S_CORR: K=0.1 N=4 DQ=0.3H
XSPH_C=0.02  GRAVITY=9.8
DOMAIN_MIN=(-1.5,-0.05,-1.1)  CELL=0.1  GRID=30x20x22=13200格  CELL_CAP=24  NBR_MAX=48
POOL内壁: x[-1.4,1.4] z[-1.0,1.0] 底y=0 壁顶y=0.8 域顶y=1.9（池即世界,粒子恒clamp池内,无逃逸）
COLL_R=0.025  DRAW_R=0.045  IRES=640x480  FAR_BITS=0x7F800000u  THICK_SCALE=1024
SPLAT_MAX_PX_R=16  NOZZLE_BASE=(0,1.7,0.9) x随瞄准clamp±1.0  WATER_REF_Y=0.3
相机: eye(0,3.0,3.2) look(0,0.25,0) up(0,1,0) fovY45° aspect4/3 near0.1 far100 (P11≈2.414)
核: Poly6=315/(64πh⁹)(h²-r²)³  Spiky梯度=-45/(πh⁶)(h-r)²r̂
```

## Pass 内算法要点（G=max(1,⌈alive/256⌉)；SimPass 内每 Dispatch 后照抄 Fluid2D:338-348 双屏障）

1. **emit**（emit_count>0 才录，1 组）：slot=(emit_start+tid)%MAX；pos=喷嘴+垂直圆盘 jitter+沿 v0 推进防聚团；vel=emit_vel+~6° 锥 jitter（hash(frame_seed^tid)）
2. **predict** ×G：v+=重力·dt；interact_flag 时 `v += normalize(dir)·25·exp(-len²/0.35²)·dt`（dir.y×0.5）；xstar_a=pos+v·dt
3. **fill_buf** ×52（清 cell_count）→ **grid_bin** ×G → **neighbor** ×G
4. **[lambda→deltapos]×3**：lambda SRB 迭代 0,2 读 a / 1 读 b；deltapos SRB [0]=a→b、[1]=b→a；deltapos 内盒碰撞 clamp。**ping-pong 仅此两组 SRB**，其余全静态
5. **velocity** ×G（XSPH 邻速=(x*_b[j]−pos[j])/dt 重算，只写 vel）→ **finalize** ×G
6. **SplatPass**：fill_img ×2（40×30 组）→ ResourceBarrier(UAV,UAV) → splat ×G：clip=VP·pos，w<0.1 弃，px=((ndc.x·0.5+0.5)·W,(0.5−0.5·ndc.y)·H)，pr=clamp(DRAW_R·0.5·H·P11/w,1,16)，圆盘内 `imageAtomicMin(uimg_depth, floatBitsToUint(w−R·sqrt(1−d²/pr²)))`、`imageAtomicAdd(uimg_thick, uint(弦长·THICK_SCALE))`（正 float 位型单调，atomicMin 即 float min）
7. **BlurH/V**（40×30 组，16×16）：深度 9-tap 双边（tap=FAR 或 |Δd|>0.15 退回中心值），厚度高斯（沿用 Fluid2D G0..G4 权重）
8. **Display**：SetRenderTarget(backbuffer+DSV, clear) → Draw{3,1}

## C++ 结构（镜像 Fluid2D.cpp）

- glm include 放 Window.h 之前；不用 std::min/max（windows.h 宏），用三目
- `struct Fluid3DData : RenderGraphPassDataBase`（空壳，5 个 pass 共用）
- CPU 状态：`alive_count, emit_head, frame_index; firing, interacting; nozzle_pos/emit_vel/interact_pos/aim_target(glm::vec3); cur_dt, time_sec; view_proj/inv_view_proj(glm::mat4)`；transient 资源指针成员（`RenderGraphResource<TextureDesc,Texture>* depth_u_res` 等，供 execute lambda `GetActual()`）

**OnInit_Logic**：① 注册 BackBuffer/DSV retained ② 建全部 buffer + 上传 3 个 fill 参数 ③ glm 建 view_proj/inv_view_proj ④ **Load 全部 14 个 shader → 建全部 13 个 PSO → 统一 delete shader**（PSO 缓存按指针哈希）⑤ 建 SRB：buffer 绑定即时 SetResource+Flush，纹理绑定留到每帧 ⑥ 按序 AddRenderPass 5 个 pass（transient 在 SplatPass/BlurH/BlurV 的 setup lambda 里 `builder.Create` + 带状态 `Read/Write`，资源指针存成员）⑦ SimPass `SetIsCullable(false)`；`graph.Compile()`

**OnUpdate(dt)**：光标→NDC（ny=1−2·cy/win_h）→ inv_view_proj unproject 成射线 → 交 y=WATER_REF_Y 平面（射线朝上沿用上次）→ aim_target=clamp(池内缩 0.1) → nozzle_pos.x=clamp(aim.x·0.5,±1) → 弹道解 `emit_vel=Δ/T+0.5·G·T·up`（T=clamp(|Δ|/JET_SPEED,0.25,0.7)）；firing=LMB，interacting=RMB，interact_pos=aim_target

**SimPass execute 开头 CPU 记账**：emit_count=firing?64:0；emit_start=emit_head；emit_head=(head+cnt)%MAX；alive=min(alive+cnt,MAX)（满员环形覆盖最老粒子）→ 填 64 float 上传 fp_buf

**OnShutdown_Logic**：graph.Release() → delete SRB → delete buffer。**main()** 与 Fluid2D:494-501 一致。

## 水墨 display.frag 合成

坐标约定全工程统一（splat/display/CPU unproject 三处）：`px=((ndc.x·0.5+0.5)·W,(0.5−0.5·ndc.y)·H)`；fullscreen.vert UV(0,0)=左上=纹理行 0，`texture(f_b, inUV)` 直接对齐。

1. `vec2 dt_s = texture(f_b, uv).rg`（硬件双线性）→ d=深度, t=厚度
2. `mask = (d<1e6) ? smoothstep(0.02,0.10,t) : 0`（软边=晕染）
3. 浓淡：`shade = 1−exp(−0.8t)`（Beer-Lambert）
4. 图像空间法线：邻 UV 采样 dzdx/dzdy（FAR 邻元用中心值），`n=normalize(vec3(−dzdx·8,−dzdy·8,1))`
5. 勾勒：`edge=clamp(|∇d|·SCALE−0.5,0,1)`；`rim=pow(1−n.z,3)`；`stroke=max(edge,rim·0.6)·mask`
6. 纸底 (0.96,0.945,0.915)+角落晕影+hash 纸纹
7. `ink=mix(淡墨(0.45,0.48,0.52),浓墨(0.06,0.08,0.11),shade)`；`col=mix(col,ink,mask·(0.35+0.65·shade))`；叠 `stroke·0.85`
8. 池体勾线：fp 的 view_proj 投影 8 角点，12 边求 2D 点-线段距离 → smoothstep 线宽 + sin 笔触抖动 + hash 飞白，远侧边减淡
9. 水枪指示：nozzle 投影墨点+朝 aim 短笔画；aim_target 淡墨圆环（未开火半透明）

## 风险与对策

| 风险 | 对策 |
|---|---|
| **y 翻转不一致**（最高概率 bug：上下颠倒/弹道不中） | 三处统一约定；先"投影池角点画十字"自检 |
| transient 路径首次被样例使用 | A2 修复后先用 DebugDumpBarriers() 打印验证屏障序列；validation layer 盯布局错误；池化纹理带上帧遗留 state，TransitionTextureState 按纹理自身 state 转换（机制已确认成立） |
| 同态跨 pass UAV 写→读无 RDG 屏障 | BlurH lambda 开头手动 `ResourceBarrier(UAV,UAV)`（全局 memory barrier 覆盖 GENERAL 布局 image 访问） |
| PBF 爆炸/NaN | finalize 扫 NaN 重置池心；调参入口 REST_DENSITY/EPS_CFM/S_CORR_K |
| 格/邻居表溢出 | 丢弃+读端 min(cnt,CAP)，PBF 自恢复；异常提 CAP |
| RG32F 线性过滤非规范强制 | 桌面 GPU 实际全支持；不行换 RGBA16F |
| A2 修复影响编辑器 | transient barrier 逻辑本来就错，修复更正确；回归跑 Editor + Fluid2D |
| glslang 全量编译 | 新 shader 语法错误挂全部目标构建，先本地全量 build |

## 验证

**Part A 回归**（引擎改动后、写样例前）：
1. `xmake build Editor` + `xmake build RendererSample-Fluid2D` 编译通过
2. `xmake run RendererSample-Fluid2D` / `xmake run RendererSample-Texture` 表现不变（blend 修复默认行为不变、barrier 修复不影响 retained-only 图）
3. 层级检查 grep 三连（CLAUDE.md 命令）

**Part B**：
1. `xmake build RendererSample-Fluid3D` → `xmake run RendererSample-Fluid3D`
2. 目视：a) 启动=纸白+池体墨线（验证投影）b) LMB=弧形墨色水流射向准星并随鼠标移动 c) 5-10 秒积水、厚区墨浓、边缘勾线 d) RMB 点水面扩散扰动 e) 松开渐平 f) 射满 32K 不崩帧率稳
3. 控制台无 validation 报错、无 SRB 名字异常
4. 调试开关：display 直接输出 t/d 灰度跳过 blur 验证 splat；SOLVER_ITERS 降 1 验证 ping-pong 链路

## 参考文件

- `src/Sample/7-Fluid2D/Fluid2D.cpp` — C++ 骨架/屏障/SRB/main 模板
- `resource/Shader/Sample/fluid_display.frag` — 风格化合成参考
- `src/Sample/5-Bindless/Bindless.cpp` — glm 相机 + 空纹理创建先例
- `src/Runtime/Render/Core/RenderGraph.cpp:318-401` — barrier 生成（A2 修复点）
- `src/Runtime/RHI/Vulkan/VK_Shader.cpp:182-252` — SetResource 描述符分发（A1 修复点）
