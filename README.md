# MyRenderer
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
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

## Branch
① The release version is in the branch [**main**] 

② Old Shit code is in branch [**batch**] , but it can run

③ Each branch develops one feature and merges into the branch [**main_dev**] 

④ The updatest one branch now is the branch [**RHI-Recode**]

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
 ## Sample
######Tip: Mainly to verify the engine feature
