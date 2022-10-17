#version 450


layout(location = 0) in vec3 TexCoords;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform samplerCube specular_sampler;

void main() {
    outColor = vec4(TexCoords, 1.0);
}