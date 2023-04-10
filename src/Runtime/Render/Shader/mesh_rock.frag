#version 460

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D basecolor_sampler;

layout(set = 0, binding = 1) uniform  CameraData{   
    vec3 viewPos; // w is for exponent
    vec3 viewDir;
} cameraData;

void main() {
    outColor = texture(basecolor_sampler, TexCoords);
}