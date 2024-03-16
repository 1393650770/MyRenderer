#version 460

layout(location = 0) in vec2 out_uv;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D basecolor_sampler;

void main() {
    outColor = vec4(texture(basecolor_sampler, out_uv).rgb, 1.0);
}