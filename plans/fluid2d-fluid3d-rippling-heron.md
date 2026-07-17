# Fluid3D → FLIP 网格混合海湾：无限放水 + 水位自由上涨 + 海洋渲染

## 当前实施进度（续作从这里开始）

- ✅ P1-1：10 个 FLIP shader 已写完（emit_prep/emit/grid_clear/p2g/grid_vel/divergence/jacobi/project/g2p/cull，均在 resource/Shader/Sample/）
- ✅ P1-2：fluid3d_splat.comp 已改 alive 标志遍历（MAX=786432 + pos.w 检查）；fluid3d_display.frag 已改海湾尺寸（POOL_MIN/MAX=±3/±2、高 3、TILE 0.5）
- ⏳ P1-3：Fluid3D.cpp 重写**只写了前半**（头/常量/类声明/成员/CreateFluidBuffers 含预铺静海与 free-list 初始化），文件以 `// ==== SPLIT MARKER: second half appended below ====` 结尾，**当前不可编译**——续写时用 Edit 把该标记替换为后半部分：CreateFluidPipelines（14 shader→14 PSO→统一 delete）、CreateFluidBindings（15 个 SRB 按 shader 绑定表）、RecordSim（emit_prep/emit + 2 子步循环：grid_clear→p2g→grid_vel→divergence→jacobi×64 交替→project→g2p→cull，组数：粒子 3072、面 291、格 282）、RecordClear/Splat/BlurH/BlurV/Display（照旧 + splat 组数改 3072）、OnInit（retained 注册 + 6 pass）、OnUpdate（相机 eye(0,5.5,7) look(0,1.2,0)、WATER_REF_Y=1.0、aim clamp x±2.8 z±1.8、T clamp[0.3,1.0]、fp[3]=emit request）、OnShutdown、main
- ⬜ P1-4：删除 8 个 PBF shader（predict/fill_buf/grid_bin/neighbor/lambda/deltapos/velocity/finalize）
- ⬜ P1-5：构建+运行验证（静海平稳/无限放水/水位上涨）
- ⬜ P2：泡沫通道 + 海洋 display 重写


## Context

PBF 纯粒子路线存在机制性缺陷（不可压缩性靠迭代喂、深压缩恢复只能表面向内传播 → 水位压扁；粒子总数=水量 → 无法无限放水）。用户决定改用 **FLIP 网格混合**（UE Niagara Fluids Grid3D 同款机制）：压力投影在 3D MAC 网格上求解（无邻居表、不可压缩性强），粒子只携带速度（P2G/G2P）；**每格粒子数设上限、超出剔除、槽位入 GPU free-list 复用** → 粒子总量由域体积决定而非放水量，真"无限放水、水位自由上涨"。场景升级为 6×4×3m 深水海湾，渲染追加泡沫白水、波纹细节法线、海洋天空、深度雾+caustics。

用户手改的实验残留（`MAX_PARTICLES=655360`、`PREFILL_LAYERS=107`、deltapos 的 `H=100`/`DP_MAX=30*H` 等）随本次重写整体替换。

## 常量（C++ 与 shader 同步）

```
域: x[-3,3] z[-2,2] y[0,3]（内壁即域界）  CELL=0.1
GRID = 60(x) × 30(y) × 40(z) = 72,000 格
MAC 面数组: U 61×30×40=73,200  V 60×31×40=74,400  W 60×30×41=73,800（取 NF=74,400 统一分配）
MAX_PARTICLES = 786,432（满域 8/格=576K + 裕量; 3072 组×256）
CELL_TARGET = 8/格（静止 0.05 间距）  CELL_CAP = 16（超出剔除）
FIXED_SCALE = 65536（定点原子: |v|≤8 × 权重和≤16 × 65536 ≈ 8.4M << 2^31 安全）
SUBSTEPS = 2（dt/2≈8.3ms; CFL: 5m/s×8.3ms=4.2cm < CELL ✓）
JACOBI_ITERS = 64/子步（偶数，压力终点固定在 p_a）
FLIP_BLEND = 0.95（0.95 FLIP + 0.05 PIC，压噪声）
EMIT_PER_FRAME = 32  JET_SPEED = 5.0  GRAVITY = 9.8
PREFILL: 静水面 y=1.0（60×40×10 格 × 8 = 192K 粒子晶格，海湾开局即有海）
相机: eye(0, 5.5, 7.0) look(0, 1.2, 0) fovY 45°   喷嘴基准 (0, 3.4, 2.6)，瞄准面 WATER_REF_Y=1.0
IRES 保持 640×480
```

## GPU Buffer 表（全部 Storage；★=int 原子）

| buffer | 布局 | 大小 | 写→读 |
|---|---|---|---|
| pos_buf | float4/粒子（w=alive 标志） | 12MB | prefill,emit,g2p → p2g,splat,cull |
| vel_buf | float4/粒子（w=foam 因子） | 12MB | emit,g2p → p2g,splat |
| rank_buf ★ | uint/粒子（P2G 抢票名次） | 3MB | p2g → cull |
| free_slots / free_cnt ★ | uint×MAX / uint×2{push,pop} | 3MB | cull(push),emit(pop)；init 时 CPU 预填 [prefill..MAX) |
| mom_u/v/w ★, wgt_u/v/w ★ | int×NF ×6 | 1.8MB | p2g → grid_vel |
| grid_u/v/w | float×NF ×3 | 0.9MB | grid_vel,project → g2p,divergence |
| old_u/v/w | float×NF ×3（投影前快照，FLIP Δ用） | 0.9MB | grid_vel → g2p |
| cell_count ★ / cell_type | uint×72K ×2 | 0.6MB | p2g/cell_type → jacobi,cull |
| div_buf, p_a, p_b | float×72K ×3 | 0.9MB | divergence/jacobi → project |
| fp_buf | float×64（布局沿用+扩展） | 256B | CPU → 各 pass |

合计 ~35MB。屏幕空间纹理沿用 + 新增 **ink_foam_u（R32U，STORAGE）**；f_a/f_b 的 b 通道传泡沫。

## Shader 清单

**删除**（PBF 全套）：predict / fill_buf / grid_bin / neighbor / lambda / deltapos / velocity / finalize（8 个）

**新增**（每子步顺序执行；绑定全 set=0，实例名即 SetResource 名）：
| shader | 功能要点 |
|---|---|
| fluid3d_grid_clear.comp | 一次 dispatch 清全部网格数组（NF 线程，各数组带边界检查置零） |
| fluid3d_p2g.comp | alive 粒子三线性散射 momentum/weight×FIXED_SCALE 到 6 个面数组（atomicAdd int）；cell_count atomicAdd 返回值存 rank_buf |
| fluid3d_grid_vel.comp | 面速度=mom/wgt（wgt=0 置零）；v 面加重力·dt；同 pass 快照 old_u/v/w |
| fluid3d_cell_type.comp | SOLID（域界外圈）/ FLUID（cell_count>0）/ AIR |
| fluid3d_divergence.comp | 流体格散度（面速度差分） |
| fluid3d_jacobi.comp ×2 SRB | 压力 ping-pong；solid 邻居 Neumann（用中心 p）、air Dirichlet（p=0） |
| fluid3d_project.comp | 面速度减 ∇p；solid 面法向速度清零 |
| fluid3d_g2p.comp | 三线性采样 new/old 网格速度 → FLIP Δ 混合 0.95/0.05；RMB 径向冲量；泡沫因子更新（|散度|大或高速近表面 +，否则指数衰减）；RK2 平流；clamp 域内；写 pos/vel |
| fluid3d_cull.comp | rank ≥ CELL_CAP → pos.w=0，slot 压入 free_slots（push 计数原子） |
| fluid3d_emit.comp（重写） | tid<emit_request：pop=atomicAdd(free_cnt.pop)，pop<push 才取 slot 生成粒子（弹道速度+锥形 jitter），否则跳过——全 GPU 端，CPU 不需要活粒子数 |

**修改**：splat（+foam：读 vel.w，atomicAdd 到 uimg_foam）、blur_h（3 uimage 入 → rgba16f r=depth g=thick b=foam）、blur_v（透传 b）、display（Phase 2 重写）。

每帧 dispatch 数：2×(1+1+1+1+1+64+1+1+1)+1 ≈ **145** + 泼溅链 4 —— 与 Fluid2D 模式同量级放大，可行；每 dispatch 后沿用双 ResourceBarrier。

## C++ 改动（Fluid3D.cpp）

- 常量区整体替换；删 PBF buffer/PSO/SRB 成员，加上表新成员（SRB 总数 ~18，全 init 绑定，**不变约束**：运行期零 SetResource）
- ClearPass 增加 `ClearTexture(ink_foam_u, {0,...})`
- RecordSim 重写为上述子步循环；emit dispatch 每帧一次（组数 1）
- prefill：CPU map 写 pos_buf 晶格（alive=1）+ free_slots 预填 + free_cnt{push=MAX-prefill, pop=0}；vel 清零
- fp 布局沿用（[0]dt/2 [1]=emit_request 改语义为发射请求数 [4-6]喷嘴 [8-10]射速 [12-15]交互 [16-31]VP [32-35]aim/time [36-51]invVP [52-54]eye），发射不再需要 alive/emit_start
- 相机/喷嘴/瞄准平面按新场景常量；OnUpdate 弹道解算不变（JET_SPEED=5）

## Phase 1（先跑通 FLIP 核心，渲染沿用）

1. 上述 sim 重写 + splat/blur 暂不加 foam（display 只改盒子尺寸/相机常量：POOL_MIN/MAX→(±3,0,±2)、壁顶 3.0）
2. **验证里程碑**：开局 1m 静水面平稳（网格投影下应几乎无抖动——对比 PBF 的沸腾）；LMB 持续放水任意久，水位持续上涨到域顶也不崩（剔除生效，粒子数有界）；RMB 扰动波纹扩散衰减

## Phase 2（海洋渲染）

1. splat/blur/display 接入 foam 通道；display 重写：
   - 海洋天空：rd.y 渐变 + 太阳 + 2 octave FBM 云
   - 海湾背景：沙底（噪声斑驳暖色）+ 岩壁灰
   - 波纹细节法线：2 octave 滚动噪声扰动重建法线（幅度随泡沫/陡度衰减）
   - 深度雾：折射色按表面点→背景命中点距离指数衰减混入深水色
   - caustics：池底程序化干涉纹 × 水深调制 × time
   - 泡沫：b 通道密度 → 白色叠加（高光抑制）
2. 验证：射流落点与波峰出现白沫并随平静消散；水下沙底带焦散光斑；远处水面有波光

## 风险与对策

| 风险 | 对策 |
|---|---|
| 定点原子溢出 | 上界 8.4M<2^31 已算清；FIXED_SCALE 常量集中定义 |
| wgt=0 面除零 | 置零 + G2P 采样时按权重归一 |
| FLIP 噪声/抖动 | 0.05 PIC 混合，可调到 0.1；泡沫阈值避开噪声区 |
| free-list 竞争 | push/pop 单调计数 + pop<push 校验（miss 则本帧少发几粒，无害） |
| 剔除选谁死 | P2G atomic 抢票名次天然确定（同帧稳定），无 CPU 参与 |
| Jacobi 64 迭代不够深水 | 视觉标准足够；不够加到 96（每 dispatch 72K 线程，开销小） |
| 首帧网格全空 | grid_clear 在每子步开头，cell_type 全 AIR → jacobi/project 自然 no-op |
| 命令缓冲 ~150 dispatch | Fluid2D 31 个同构放大，无新机制；实测验证帧率 |

## 验证

1. `xmake build RendererSample-Fluid3D` → `xmake run`（Phase 1 后即可跑）
2. 静水验证 → 无限放水验证（放 2 分钟以上，水位到顶、帧率稳、无 validation 报错）→ Phase 2 外观验证（泡沫/波光/焦散/雾）
3. 回归 Editor + Fluid2D 构建

## 关键文件

- `src/Sample/8-Fluid3D/Fluid3D.cpp` — 主体重写
- `resource/Shader/Sample/fluid3d_{grid_clear,p2g,grid_vel,cell_type,divergence,jacobi,project,g2p,cull}.comp` — 新增
- `resource/Shader/Sample/fluid3d_emit.comp` — 重写（free-list）
- `resource/Shader/Sample/fluid3d_{splat,blur_h,blur_v}.comp`、`fluid3d_display.frag` — 泡沫通道 + 海洋合成
- 删除 8 个 PBF shader 文件
