#version 460

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inPos;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform  constants{   
   float z;
};

layout(set = 1, binding = 0) uniform sampler2D basecolor_sampler;

layout(set = 0, binding = 1) uniform  CameraData{   
    vec3 viewPos; // w is for exponent
    vec3 viewDir;
} cameraData;


float screenPxRange() {
    vec2 unitRange = vec2(z)/vec2(textureSize(basecolor_sampler, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(texCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

void main() {

    float sdf = texture(basecolor_sampler, texCoord).r;
    float smoothWidth = fwidth(sdf);
    float alpha = smoothstep(0.5f - smoothWidth, 0.5f + smoothWidth, sdf);
    vec3 rgb = vec3(alpha);
    if(alpha <= 0.01f)
    {
        discard;
    }
    outColor =vec4(alpha, alpha, alpha, alpha);
}