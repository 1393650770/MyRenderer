#version 460

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform  CameraData{   
    vec3 viewPos; // w is for exponent
    vec3 viewDir;
} cameraData;

void main() {
    outColor = vec4(fragColor, 1.0);
}