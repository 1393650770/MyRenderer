#version 450

layout(location = 0) in vec3 TexCoords;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D cubemap_sampler;

void main() {
    outColor = vec4(1.0,0.0,0.0, 1.0);
}