#version 450

layout(set=0,binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoords;

layout(location = 0) out vec2 TexCoords;



void main() {
    vec4 pos = mvp.proj * mvp.view *mvp.model* vec4(inPosition, 1.0f);
    gl_Position = pos;
    TexCoords=inTexCoords;
}