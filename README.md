# MyRenderer
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
## How to build 
##### ①You should download the xmake : https://github.com/xmake-io/xmake

##### ②Open cmd in this document folder , then input then you can build win project:
<pre>
xmake project -k vsxmake -m "debug;release"
</pre>

##### ③Build the Project in debug mode
<pre>
xmake config -m debug
xmake
</pre>

## Branch
① The release version is in the branch [**main**] 

② Old Shit code is in branch [**batch**] , but it can run

③ Each branch develops one feature and merges into the branch [**main_dev**] 

④ The updatest one branch now is the branch [**RHI-Recode**]

## Feature
###### Tip: maybe still in progress
 - [x] RHI
The architecture refers to UE and
 - [x] Vulkan 
The implementation refers to UE/ Nvrhi/ DiligentEngine etc
 - [x] RenderGraph
Refers to fg

## To do list
 - [ ] add shadow
 - [ ] add raytrace-gi
 - [ ] add Skeletal-Animation
 - [ ] add particle system





 ## Sample
######Tip: Mainly to verify the engine feature
 ### 1-HelloTriangle
