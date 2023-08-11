# MyRenderer
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
## How to build 
①You should download the xmake : https://github.com/xmake-io/xmake

②Open cmd in this document folder , then input then you can build win project:
<pre>
xmake project -k vsxmake -m "debug;release"
</pre>

③Build the Project in debug mode
<pre>
xmake config -m debug
xmake
</pre>

## To do list
 
 - [ ] wrap opengl 
 - [ ] wrap light model - pbr 
 - [ ] wrap camera 
 - [ ] wrap light system 
 - [ ] add shadow
 - [ ] add gi
 - [ ] add Skeletal-Animation
 - [ ] add optix to do real time ray tracing
 - [ ] add particle system