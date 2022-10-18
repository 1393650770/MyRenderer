#version 450


layout(location = 0) in vec3 TexCoords;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform samplerCube cubemap_sampler;

void main() {
    outColor = texture(cubemap_sampler, TexCoords);
}