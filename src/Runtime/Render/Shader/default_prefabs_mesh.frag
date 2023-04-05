#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 0) uniform sampler2D basecolor_sampler;

void main() {
    outFragColor = texture(basecolor_sampler, texCoord);
}