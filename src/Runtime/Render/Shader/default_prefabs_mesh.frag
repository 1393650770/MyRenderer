#version 460

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inPos;
layout (location = 0) out vec4 outFragColor;

layout(push_constant) uniform  constants{   
   float z;
};  

layout(set = 1, binding = 0) uniform sampler2D basecolor_sampler;

layout(set = 0, binding = 1) uniform  CameraData{   
    vec4 viewPos; // w is for exponent
    vec4 viewDir;
} cameraData;

void main() {
    outFragColor = vec4(texture(basecolor_sampler, texCoord).xyz,z);
}