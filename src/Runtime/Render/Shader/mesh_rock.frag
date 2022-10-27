#version 450

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D basecolor_sampler;

void main() {
    outColor = texture(basecolor_sampler, TexCoords);
}