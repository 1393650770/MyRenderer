#version 460
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"
layout(set=0,binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) in vec3 inPosition;
layout (location = 1) in vec2 vNormal;
layout (location = 2) in vec3 vColor;
layout(location = 3) in vec2 inTexCoords;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outPos;


void main() {
    vec4 pos = mvp.proj * mvp.view *mvp.model* vec4(inPosition, 1.0f);
    gl_Position = pos;
    texCoord=inTexCoords;
    outColor=vColor;
    outNormal= OctNormalDecode(vNormal);
}