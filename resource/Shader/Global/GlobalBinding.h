#ifndef GLOBAL_BINDING_H_
#define GLOBAL_BINDING_H_

layout(set=0,binding = 0) uniform ObjInfo {
    mat4 model;
} g_perObj;
layout(set=0,binding = 1) uniform MatInfo {
    float temp;
} g_perMat;
layout(set=0,binding = 2) uniform ViewInfo {
    mat4 view;
    mat4 proj;
} g_perView;


#endif
